from __future__ import annotations

import tempfile
import unittest
from pathlib import Path

from scripts.code_health import analyze_repository, build_baseline, compare_baseline, validate_ownership_manifest, validate_policy


def write(root: Path, relative: str, text: str) -> None:
    path = root / relative
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(text, encoding="utf-8")


def policy(**thresholds: int) -> dict:
    limits = {
        "cpp_loc": 350,
        "header_loc": 250,
        "function_loc": 80,
        "complexity": 15,
    }
    limits.update(thresholds)
    return {
        "source_roots": ["Source/PinkCab"],
        "domains": ["Core", "Interaction", "Vehicle", "Taxi", "Economy", "World", "Traffic", "Persistence", "Service", "Cockpit"],
        "thresholds": limits,
        "raw_input": {
            "allowed_prefixes": ["Source/PinkCab/Public/Interaction", "Source/PinkCab/Private/Interaction"],
            "patterns": ["IsInputKeyDown(", "GetInputAnalogKeyState(", "GetRawKeyValue(", "InputKey("],
        },
        "chaos_writes": {
            "allowed_files": ["Source/PinkCab/Private/Vehicle/PinkCabChaosVehicleDynamicsProvider.cpp"],
            "patterns": ["SetSteeringInput(", "SetThrottleInput(", "SetBrakeInput(", "SetHandbrakeInput(", "SetDriveTorque(", "SetBrakeTorque(", "SetTorqueCombineMethod("],
        },
        "forbidden_dependencies": [
            {"from": ["Vehicle", "Taxi", "Service", "Economy", "World", "Traffic", "Cockpit"], "to": ["Persistence"]}
        ],
        "allowlist": [],
    }


class CodeHealthAnalyzerTests(unittest.TestCase):
    def run_analysis(self, files: dict[str, str], config: dict | None = None) -> dict:
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            for relative, text in files.items():
                write(root, relative, text)
            return analyze_repository(root, config or policy())

    def rules(self, report: dict) -> list[str]:
        return [item["rule"] for item in report["violations"]]

    def test_detects_file_loc_limit(self) -> None:
        report = self.run_analysis(
            {"Source/PinkCab/Private/Vehicle/Large.cpp": "\n".join(["int x=0;" for _ in range(6)])},
            policy(cpp_loc=5),
        )
        self.assertIn("file_loc", self.rules(report))

    def test_detects_function_length(self) -> None:
        source = "void FThing::Run()\n{\nint a=0;\nint b=0;\nint c=0;\n}\n"
        report = self.run_analysis(
            {"Source/PinkCab/Private/Vehicle/Thing.cpp": source},
            policy(function_loc=4),
        )
        self.assertIn("function_loc", self.rules(report))

    def test_detects_complexity(self) -> None:
        source = "bool FThing::Run(bool A, bool B)\n{\nif (A && B) return true;\nif (A) return false;\nreturn B;\n}\n"
        report = self.run_analysis(
            {"Source/PinkCab/Private/Vehicle/Thing.cpp": source},
            policy(complexity=2),
        )
        self.assertIn("complexity", self.rules(report))

    def test_control_flow_keywords_are_not_reported_as_functions(self) -> None:
        source = "int FThing::Run(int X)\n{\n    switch (X)\n    {\n    case 1: return 1;\n    default: break;\n    }\n    while (X > 0)\n    {\n        --X;\n    }\n    return X;\n}\n"
        report = self.run_analysis({"Source/PinkCab/Private/Vehicle/Thing.cpp": source})
        names = [item["name"] for item in report["files"]["Source/PinkCab/Private/Vehicle/Thing.cpp"]["functions"]]
        self.assertEqual(["FThing::Run"], names)

    def test_detects_nontrivial_business_logic_in_public_header(self) -> None:
        source = "struct FThing { bool Run(bool A) { if (A) return true; return false; } };\n"
        report = self.run_analysis({"Source/PinkCab/Public/Vehicle/Thing.h": source})
        self.assertIn("public_header_logic", self.rules(report))

    def test_detects_raw_input_outside_interaction(self) -> None:
        source = "bool Down(APlayerController& PC) { return PC.IsInputKeyDown(EKeys::E); }\n"
        report = self.run_analysis({"Source/PinkCab/Private/Vehicle/BadInput.cpp": source})
        self.assertIn("raw_input_ownership", self.rules(report))

    def test_allows_raw_input_inside_interaction(self) -> None:
        source = "bool Down(APlayerController& PC) { return PC.IsInputKeyDown(EKeys::E); }\n"
        report = self.run_analysis({"Source/PinkCab/Private/Interaction/Input.cpp": source})
        self.assertNotIn("raw_input_ownership", self.rules(report))

    def test_detects_every_direct_chaos_write_outside_provider(self) -> None:
        apis = [
            "SetSteeringInput(",
            "SetThrottleInput(",
            "SetBrakeInput(",
            "SetHandbrakeInput(",
            "SetDriveTorque(",
            "SetBrakeTorque(",
            "SetTorqueCombineMethod(",
        ]
        for api in apis:
            with self.subTest(api=api):
                source = f"void Bad() {{ Movement.{api}0); }}\n"
                report = self.run_analysis({"Source/PinkCab/Private/Vehicle/BadChaos.cpp": source})
                self.assertIn("chaos_write_ownership", self.rules(report))

    def test_allows_direct_chaos_writes_only_in_canonical_provider(self) -> None:
        source = "".join([
            "void Good() { Movement.SetSteeringInput(0); }\n",
            "void Good2() { Movement.SetThrottleInput(0); }\n",
            "void Good3() { Movement.SetBrakeInput(0); }\n",
            "void Good4() { Movement.SetHandbrakeInput(false); }\n",
            "void Good5() { Movement.SetDriveTorque(0, 0); }\n",
            "void Good6() { Movement.SetBrakeTorque(0, 0); }\n",
            "void Good7() { Movement.SetTorqueCombineMethod(0, 0); }\n",
        ])
        report = self.run_analysis({
            "Source/PinkCab/Private/Vehicle/PinkCabChaosVehicleDynamicsProvider.cpp": source
        })
        self.assertNotIn("chaos_write_ownership", self.rules(report))

    def test_chaos_write_names_in_comments_do_not_count_as_writes(self) -> None:
        source = "// Never call Movement.SetThrottleInput(1.0f) here.\nvoid Good() {}\n"
        report = self.run_analysis({"Source/PinkCab/Private/Vehicle/CommentOnly.cpp": source})
        self.assertNotIn("chaos_write_ownership", self.rules(report))

    def test_detects_forbidden_persistence_dependency(self) -> None:
        source = '#include "Persistence/PinkCabGameSnapshot.h"\n'
        report = self.run_analysis({"Source/PinkCab/Public/Taxi/Bad.h": source})
        self.assertIn("forbidden_dependency", self.rules(report))

    def test_detects_domain_cycle(self) -> None:
        report = self.run_analysis({
            "Source/PinkCab/Public/Vehicle/A.h": '#include "Taxi/B.h"\n',
            "Source/PinkCab/Public/Taxi/B.h": '#include "Persistence/C.h"\n',
            "Source/PinkCab/Public/Persistence/C.h": '#include "Vehicle/A.h"\n',
        })
        self.assertIn(["Persistence", "Taxi", "Vehicle"], report["dependency_sccs"])
        self.assertIn("dependency_cycle", self.rules(report))

    def test_baseline_allows_existing_debt_but_rejects_new_identity(self) -> None:
        first = self.run_analysis({
            "Source/PinkCab/Private/Vehicle/Old.cpp": "bool Down(APlayerController& PC) { return PC.IsInputKeyDown(EKeys::E); }\n",
        })
        baseline = build_baseline(first)
        self.assertEqual([], compare_baseline(first, baseline))
        second = self.run_analysis({
            "Source/PinkCab/Private/Vehicle/Old.cpp": "bool Down(APlayerController& PC) { return PC.IsInputKeyDown(EKeys::E); }\n",
            "Source/PinkCab/Private/Taxi/New.cpp": "bool Down(APlayerController& PC) { return PC.IsInputKeyDown(EKeys::W); }\n",
        })
        regressions = compare_baseline(second, baseline)
        self.assertTrue(any(item["rule"] == "baseline_regression" for item in regressions))

    def test_ownership_manifest_rejects_missing_paths(self) -> None:
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            write(root, "Source/PinkCab/Public/Vehicle/Good.h", "#pragma once\n")
            manifest = {"version": 1, "concerns": [{
                "id": "steering",
                "owner_domain": "Vehicle",
                "public_contract": "Source/PinkCab/Public/Vehicle/Good.h",
                "implementation_root": "Source/PinkCab/Private/Vehicle/Missing.cpp",
                "test_prefix": "PinkCab.Vehicle.Steering",
            }]}
            violations = validate_ownership_manifest(root, manifest)
            self.assertTrue(any(item["rule"] == "ownership_manifest" and "implementation_root" in item["detail"] for item in violations))

    def test_ownership_manifest_rejects_duplicate_concern_ids(self) -> None:
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            write(root, "Source/PinkCab/Public/Vehicle/Good.h", "#pragma once\n")
            entry = {
                "id": "steering", "owner_domain": "Vehicle",
                "public_contract": "Source/PinkCab/Public/Vehicle/Good.h",
                "implementation_root": "Source/PinkCab/Public/Vehicle/Good.h",
                "test_prefix": "PinkCab.Vehicle.Steering",
            }
            violations = validate_ownership_manifest(root, {"version": 1, "concerns": [entry, dict(entry)]})
            self.assertTrue(any("duplicate concern id" in item["detail"] for item in violations))

    def test_ownership_doc_sync_requires_every_concern_id(self) -> None:
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            write(root, "Source/PinkCab/Public/Vehicle/Good.h", "#pragma once\n")
            manifest = {"version": 1, "concerns": [{
                "id": "steering", "owner_domain": "Vehicle",
                "public_contract": "Source/PinkCab/Public/Vehicle/Good.h",
                "implementation_root": "Source/PinkCab/Public/Vehicle/Good.h",
                "test_prefix": "PinkCab.Vehicle.Steering",
            }]}
            violations = validate_ownership_manifest(root, manifest, onboarding_text="# Start here\n")
            self.assertTrue(any(item["rule"] == "ownership_doc_sync" for item in violations))

    def test_rejects_broad_or_unexplained_allowlist(self) -> None:
        bad = policy()
        bad["allowlist"] = [{"path": "Source/PinkCab/**", "rule": "file_loc", "reason": "legacy", "expires_when": "never"}]
        with self.assertRaises(ValueError):
            validate_policy(bad)


if __name__ == "__main__":
    unittest.main()
