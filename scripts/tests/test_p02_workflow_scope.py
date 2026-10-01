import pathlib
import re
import unittest


ROOT = pathlib.Path(__file__).resolve().parents[2]
WORKFLOW = ROOT / ".github" / "workflows" / "cd648-p02-phy009.yml"


class P02WorkflowScopeTests(unittest.TestCase):
    def setUp(self):
        self.text = WORKFLOW.read_text(encoding="utf-8")

    def test_docs_only_changes_do_not_enter_runtime_physics_scope(self):
        match = re.search(
            r"\$physics=@\(\$changed \| Where-Object \{(?P<body>.*?)\n\s*\}\)",
            self.text,
            flags=re.S,
        )
        self.assertIsNotNone(match, "latest-scope classifier block not found")
        body = match.group("body")
        self.assertNotIn(
            "$_.StartsWith('docs/vehicle_physics/')",
            body,
            "docs-only admin changes must not trigger the P02 runtime suite",
        )

    def test_scope_regression_file_is_a_pull_request_trigger(self):
        self.assertIn(
            "- 'scripts/tests/test_p02_workflow_scope.py'",
            self.text,
            "editing the scope regression test must trigger this workflow",
        )

    def test_runtime_scope_guard_admits_canonical_p03_steering_owner(self):
        required = (
            "Source/PinkCabVehicle/Public/Vehicle/PinkCabSteeringController.h",
            "Source/PinkCabVehicle/Private/Vehicle/PinkCabSteeringController.cpp",
        )
        allowed_match = re.search(
            r"\$allowed=@\((?P<body>.*?)\n\s*\)",
            self.text,
            flags=re.S,
        )
        self.assertIsNotNone(allowed_match, "scope-guard allowlist block not found")
        allowed = allowed_match.group("body")
        for path in required:
            with self.subTest(path=path):
                self.assertIn(
                    path,
                    allowed,
                    "frozen P02 regression gate must admit the explicit canonical P03 steering owner without admitting arbitrary files",
                )

    def test_runtime_scope_guard_allows_canonical_p02_admin_documents(self):
        required = (
            "docs/vehicle_physics/P02_DRIVELINE_ARCHITECTURE_2026-09-28.md",
            "docs/vehicle_physics/PINK_CAB_VEHICLE_PHYSICS_CALIBRATION_PROGRAM_2026-09-26.md",
            "docs/vehicle_physics/TASKS.csv",
            "docs/vehicle_physics/TESTS.csv",
            "docs/vehicle_physics/GATE1_ROAD_HANDLING_INTEGRATION_2026-09-28.md",
        )
        allowed_match = re.search(
            r"\$allowed=@\((?P<body>.*?)\n\s*\)",
            self.text,
            flags=re.S,
        )
        self.assertIsNotNone(allowed_match, "scope-guard allowlist block not found")
        allowed = allowed_match.group("body")
        for path in required:
            with self.subTest(path=path):
                self.assertIn(path, allowed)


if __name__ == "__main__":
    unittest.main()
