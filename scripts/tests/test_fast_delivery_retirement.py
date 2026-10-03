"""Prove obsolete callers cannot mutate or promote their retained packages."""
import json
import os
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest


ROOT = Path(__file__).resolve().parents[2]
POWERSHELL = shutil.which('powershell') or shutil.which('pwsh')


@unittest.skipUnless(POWERSHELL, 'PowerShell is required to exercise the retired entry')
class FastDeliveryRetirementTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        # Deliberately no Git checkout, project or Unreal installation.
        self.script = self.root / 'scripts' / 'fast-delivery.ps1'
        self.script.parent.mkdir()
        shutil.copyfile(ROOT / 'scripts/fast-delivery.ps1', self.script)
        self.package = self.root / 'retained package'
        for name in ('retained package', 'retained package_next', 'retained package_old'):
            directory = self.root / name
            directory.mkdir()
            (directory / 'BUILD_SHA.txt').write_text('previously accepted source')
            (directory / 'payload.exe').write_bytes(b'retained package bytes')
        self.shortcut = self.root / 'Latest.lnk'
        self.shortcut.write_bytes(b'previously accepted shortcut')

    def snapshot(self):
        return {path.relative_to(self.root).as_posix():
                path.read_bytes() if path.is_file() else None
                for path in self.root.rglob('*')}

    def invoke(self, *arguments):
        env = {**os.environ,
               'PINKCAB_UE_ROOT': str(self.root / 'missing engine'),
               'PINKCAB_ITERATION_BUILD': str(self.package),
               'PINKCAB_LATEST_SHORTCUT': str(self.shortcut),
               'PINKCAB_BUILD_TEMP': str(self.root / 'must not be created')}
        before = self.snapshot()
        result = subprocess.run(
            [POWERSHELL, '-NoProfile', '-NonInteractive', '-ExecutionPolicy', 'Bypass',
             '-File', str(self.script), *arguments], cwd=self.root, env=env,
            capture_output=True, text=True, timeout=30)
        self.assertEqual(self.snapshot(), before, result.stdout + result.stderr)
        return result

    def test_default_and_legacy_switches_refuse_without_touching_retained_builds(self):
        for arguments in ((), ('-SkipTests',), ('-ForceRecook',),
                          ('-TestFilter', 'Custom.Filter'),
                          ('-SkipTests', '-ForceRecook', '-EngineRoot', str(self.root),
                           '-IterationBuild', str(self.package),
                           '-ShortcutPath', str(self.shortcut))):
            with self.subTest(arguments=arguments):
                result = self.invoke(*arguments)
                self.assertEqual(result.returncode, 2, result.stdout + result.stderr)
                self.assertIn('PINKCAB_LEGACY_FAST_DELIVERY_RETIRED', result.stderr)
                self.assertNotIn('DELIVERED', result.stdout)

    def test_plan_has_no_engine_git_or_filesystem_prerequisite_and_cannot_dispatch(self):
        result = self.invoke('-PlanOnly', '-SkipTests', '-ForceRecook')
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        plan = json.loads(result.stdout)
        self.assertEqual(plan['status'], 'RETIRED')
        for key in ('automatic_dispatch', 'package_changes', 'baseline_changes', 'shortcut_changes'):
            self.assertIs(plan[key], False, key)
        self.assertEqual(plan['target_map'], '/Game/Dev/Maps/L_PinkCab_L1_EndlessStraight')
        self.assertIn('HUMAN_PENDING', plan['delivery_result'])


if __name__ == '__main__':
    unittest.main()
