"""Exercise the real PowerShell guards in disposable Git repositories."""

import json
import os
from pathlib import Path
import shutil
import stat
import subprocess
import tempfile
import unittest
import uuid


SOURCE = Path(__file__).resolve().parents[1]
BASELINE = "accepted/p4-rig06-20261007"


@unittest.skipUnless(shutil.which("pwsh") and shutil.which("git"), "requires pwsh and git")
class VehicleFeelGuards(unittest.TestCase):
    def setUp(self):
        # Default mkdir permissions also work with restricted Windows test tokens.
        self.root = Path(os.environ.get("PINKCAB_TEST_TMP", tempfile.gettempdir())) / (
            "pinkcab-guard-" + uuid.uuid4().hex
        )
        self.root.mkdir(parents=True)
        self.addCleanup(self.cleanup)
        for name in ("vehicle-feel-guard.ps1", "platform-status.ps1"):
            self.write("scripts/" + name, (SOURCE / name).read_text(encoding="utf-8-sig"))
        self.write(".github/workflows/verify.yml", "name: verify\n")
        self.write(".github/workflows/deliver.yml", "name: deliver\n")
        self.write("PinkCab.uproject", "{}\n")
        self.git("init", "-q")
        self.git("config", "user.name", "Guard Test")
        self.git("config", "user.email", "guard-test@example.invalid")
        self.git("config", "commit.gpgsign", "false")
        self.commit()
        self.git("tag", BASELINE)
        self.git("update-ref", "refs/remotes/origin/main", "HEAD")

    def write(self, name, text="fixture\n"):
        path = self.root / name
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(text, encoding="utf-8")

    def cleanup(self):
        def remove_readonly(function, path, error):
            target = Path(path).resolve()
            if not target.is_relative_to(self.root.resolve()):
                raise error
            attributes = getattr(target.stat(), "st_file_attributes", 0)
            if not attributes & stat.FILE_ATTRIBUTE_READONLY:
                raise error
            target.chmod(stat.S_IREAD | stat.S_IWRITE)
            function(path)
        shutil.rmtree(self.root, onexc=remove_readonly)

    def git(self, *args):
        return subprocess.run(
            ["git", *args], cwd=self.root, check=True, capture_output=True, text=True
        ).stdout.strip()

    def commit(self):
        self.git("add", ".")
        self.git("commit", "-qm", "fixture")

    def run_guard(self, script="vehicle-feel-guard.ps1", *args):
        env = dict(os.environ, USERPROFILE=str(self.root), PINKCAB_UE_ROOT=str(self.root / "engine"))
        result = subprocess.run(
            ["pwsh", "-NoProfile", "-File", str(self.root / "scripts" / script), *args],
            cwd=self.root, env=env, capture_output=True, text=True, timeout=30,
        )
        return result.returncode, result.stdout + result.stderr

    def test_allows_vehicle_calibration_and_documentation(self):
        self.write("Source/PinkCabVehicle/Private/Vehicle/Calibration.cpp")
        self.write("docs/measurements.md")
        code, output = self.run_guard()
        self.assertEqual(code, 0, output)
        self.assertIn("SCOPE=PASS", output)

    def test_rejects_untracked_files_in_every_frozen_layout(self):
        paths = (
            "Source/PinkCabWorld/new.cpp", "Source/PinkCabTaxi/new.cpp",
            "Source/PinkCabTraffic/new.cpp", "Source/PinkCabEconomy/new.cpp",
            "Source/PinkCabPersistence/new.cpp",
            "Source/PinkCab/Private/World/new.cpp", "Source/PinkCab/Public/World/new.h",
            "Source/PinkCab/Private/Service/new.cpp", "Source/PinkCab/Public/Service/new.h",
            "Source/PinkCab/Private/Persistence/new.cpp",
            "Source/PinkCab/Public/Persistence/new.h",
            "Content/World/new.uasset", "Content/Dev/Maps/new.umap",
            "Content/Game/Services/new.uasset",
        )
        for path in paths:
            with self.subTest(path=path):
                self.write(path)
                code, output = self.run_guard()
                self.assertNotEqual(code, 0, output)
                self.assertIn("FORBIDDEN_VF90_CHANGE=" + path, output)
                (self.root / path).unlink()

    def test_rejects_committed_staged_and_unstaged_world_changes(self):
        path = "Source/PinkCab/Private/World/Road.cpp"
        self.write(path)
        self.commit()
        for state in ("committed", "unstaged", "staged"):
            with self.subTest(state=state):
                if state != "committed":
                    self.write(path, state)
                if state == "staged":
                    self.git("add", path)
                code, output = self.run_guard()
                self.assertNotEqual(code, 0, output)
                self.assertIn(path, output)

    def test_rename_out_of_frozen_directory_still_rejected(self):
        self.write("Source/PinkCabWorld/Old.cpp")
        self.commit()
        self.git("tag", "-f", BASELINE)
        (self.root / "docs").mkdir(exist_ok=True)
        self.git("mv", "Source/PinkCabWorld/Old.cpp", "docs/Old.cpp")
        self.commit()
        code, output = self.run_guard()
        self.assertNotEqual(code, 0, output)
        self.assertIn("Source/PinkCabWorld/Old.cpp", output)

    def test_scope_missing_baseline_fails_closed(self):
        self.git("tag", "-d", BASELINE)
        code, output = self.run_guard()
        self.assertNotEqual(code, 0, output)
        self.assertIn("BASELINE_MISSING", output)

    def test_platform_missing_tag_reports_structured_failure(self):
        self.git("tag", "-d", BASELINE)
        code, output = self.run_guard("platform-status.ps1", "-Strict")
        self.assertEqual(code, 2, output)
        report, _ = json.JSONDecoder().raw_decode(output.lstrip())
        self.assertFalse(report["Checks"]["AcceptedTagExists"])
        self.assertIn("PINKCAB_PLATFORM_STATUS=FAIL", output)


if __name__ == "__main__":
    unittest.main()
