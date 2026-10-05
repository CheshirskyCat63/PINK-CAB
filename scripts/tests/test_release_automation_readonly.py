import pathlib
import re
import unittest


ROOT = pathlib.Path(__file__).resolve().parents[2]
TEST_ROOT = ROOT / "Source" / "PinkCabTests"
WORKFLOWS = ROOT / ".github" / "workflows"
AUTOMATION_RUNNER = ROOT / "scripts" / "ci" / "run-unreal-automation.ps1"
RELEASE_WORKFLOW = WORKFLOWS / "pinkcab-g1-github-control-plane.yml"

WRITE_MARKERS = (
    "UPackage::SavePackage",
    "UEditorLoadingAndSavingUtils::SaveMap",
    "MarkPackageDirty(",
)
GENERATOR_RE = re.compile(
    r'IMPLEMENT_SIMPLE_AUTOMATION_TEST\(\s*'
    r'(?P<class>\w+),\s*'
    r'"(?P<name>PinkCab\.Editor\.Generate[^"]+)"',
    re.DOTALL,
)


class ReleaseAutomationReadOnlyTests(unittest.TestCase):
    def test_write_capable_test_files_use_explicit_authoring_guard(self):
        write_capable = []
        for path in sorted(TEST_ROOT.rglob("*.cpp")):
            text = path.read_text(encoding="utf-8")
            if not any(marker in text for marker in WRITE_MARKERS):
                continue
            write_capable.append(path)
            self.assertIn(
                '#include "PinkCabAutomationWriteGuard.h"',
                text,
                path.as_posix(),
            )
            generators = list(GENERATOR_RE.finditer(text))
            self.assertGreater(len(generators), 0, path.as_posix())
            for generator in generators:
                class_name = generator.group("class")
                run = re.search(
                    rf"bool\s+{re.escape(class_name)}::RunTest\s*"
                    r"\([^)]*\)\s*\{",
                    text,
                    re.DOTALL,
                )
                self.assertIsNotNone(run, f"{path}: {class_name}")
                prefix = text[run.end() : run.end() + 500]
                self.assertIn(
                    "PinkCabAutomationWriteGuard::"
                    "SkipUnlessAssetAuthoringAllowed",
                    prefix,
                    f"{path}: {class_name}",
                )
        self.assertGreaterEqual(len(write_capable), 7)

    def test_guard_requires_explicit_command_line_capability(self):
        guard = (
            ROOT
            / "Source"
            / "PinkCabTests"
            / "Private"
            / "PinkCabAutomationWriteGuard.h"
        ).read_text(encoding="utf-8")
        self.assertIn("PinkCabAllowAssetAuthoring", guard)
        self.assertIn("FParse::Param", guard)
        self.assertIn("PINKCAB_ASSET_AUTHORING_SKIPPED", guard)

    def test_explicit_generator_invocations_opt_in_to_authoring(self):
        occurrences = 0
        for path in sorted(WORKFLOWS.glob("*.yml")):
            lines = path.read_text(encoding="utf-8").splitlines()
            for index, line in enumerate(lines):
                if "PinkCab.Editor.Generate" not in line:
                    continue
                occurrences += 1
                start = max(0, index - 6)
                end = min(len(lines), index + 7)
                context = "\n".join(lines[start:end])
                self.assertTrue(
                    "AllowAssetAuthoring" in context
                    or "PinkCabAllowAssetAuthoring" in context,
                    f"authoring invocation lacks capability: {path}:{index + 1}",
                )
        self.assertGreaterEqual(occurrences, 8)

    def test_release_full_suite_has_no_authoring_capability_and_checks_clean_tree(self):
        text = RELEASE_WORKFLOW.read_text(encoding="utf-8")
        match = re.search(
            r"- name: Release full automation regression\n"
            r"(?P<body>.*?)(?=\n\s*- name: Release signed cook and package)",
            text,
            re.DOTALL,
        )
        self.assertIsNotNone(match)
        body = match.group("body")
        self.assertIn("Automation RunTests PinkCab", body)
        self.assertNotIn("PinkCabAllowAssetAuthoring", body)
        self.assertIn(
            "PINKCAB_RELEASE_AUTOMATION_MUTATED_TRACKED_SOURCE",
            body,
        )
        self.assertIn("PINKCAB_RELEASE_AUTOMATION_READ_ONLY=PASS", body)

    def test_shared_automation_runner_exposes_narrow_authoring_switch(self):
        text = AUTOMATION_RUNNER.read_text(encoding="utf-8")
        self.assertIn("[switch]$AllowAssetAuthoring", text)
        self.assertIn("'-PinkCabAllowAssetAuthoring'", text)

    def test_shared_automation_runner_flushes_evidence_before_test_exit(self):
        text = AUTOMATION_RUNNER.read_text(encoding="utf-8").lower()
        self.assertIn("'-forcelogflush'", text)
        self.assertLess(
            text.index("'-forcelogflush'"),
            text.index("'-testexit=automation test queue empty'"),
        )


if __name__ == "__main__":
    unittest.main()
