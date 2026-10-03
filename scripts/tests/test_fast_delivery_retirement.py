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


class CanonicalHumanDeliveryContractTests(unittest.TestCase):
    def setUp(self):
        import yaml
        document = yaml.safe_load((ROOT / '.github/workflows/cd869-deliver.yml').read_text())
        self.document = document
        self.steps = document['jobs']['deliver']['steps']

    def script(self, name):
        matches = [step.get('run', '') for step in self.steps if step.get('name') == name]
        self.assertEqual(len(matches), 1, name)
        return matches[0]

    def test_one_owner_named_shortcut_is_published_after_visible_runtime_gate(self):
        text = self.script('Verify visible candidate and replace only PINCKCAB entry')
        self.assertIn("$desktop 'PINCKCAB.lnk'", text)
        self.assertNotIn('PINKCAB Latest.lnk', text)
        self.assertNotIn('PINKCAB L1 ENDLESS HUMAN.lnk', text)
        self.assertLess(text.index('CD869_HUMAN_VISIBLE_RUNTIME_CHECK_FAILED'),
                        text.index('[IO.File]::Replace'))
        self.assertIn('CD869_FINAL_LINK_READBACK_FAILED', text)
        self.assertIn('humanAccepted=$false', text)

    def test_existing_delivery_is_never_overwritten_and_copy_is_fully_compared(self):
        text = self.script('Install and verify immutable candidate without changing desktop')
        self.assertIn('_ATTEMPT{2}', text)
        self.assertIn('CD869_IMMUTABLE_DELIVERY_ALREADY_EXISTS', text)
        self.assertNotIn('Remove-Item', text)
        self.assertIn('$a.Count -ne $b.Count', text)
        self.assertIn('$a[$path].sha256 -ne $b[$path].sha256', text)
        self.assertIn('$a[$path].bytes -ne $b[$path].bytes', text)
        self.assertNotIn('CreateShortcut', text)

    def test_delivery_does_not_rewrite_or_push_its_tested_source(self):
        text = self.script('Regenerate canonical endless runtime map from exact-head source')
        self.assertNotIn('git commit', text)
        self.assertNotIn('git push', text)
        self.assertIn('CD869_REGENERATED_MAP_DIFFERS_FROM_TESTED_SOURCE', text)

    def test_delivery_cannot_be_cancelled_by_a_second_dispatch(self):
        self.assertIs(self.document['concurrency']['cancel-in-progress'], False)


    @unittest.skipUnless(POWERSHELL, 'PowerShell executes the real publication primitive')
    def test_real_atomic_publication_call_accepts_null_backup_on_windows_powershell(self):
        import re
        text = self.script('Verify visible candidate and replace only PINCKCAB entry')
        calls = re.findall(r'^\s*(\[IO.File\]::Replace\([^\r\n]+)\s*$', text, re.MULTILINE)
        self.assertEqual(len(calls), 1)
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            source, target = root / 'new.lnk', root / 'PINCKCAB.lnk'
            source.write_bytes(b'candidate')
            target.write_bytes(b'accepted')
            script = root / 'publication.ps1'
            script.write_text("$ErrorActionPreference='Stop'\n$temp=$env:TEST_SOURCE\n$linkPath=$env:TEST_TARGET\n" + calls[0], encoding='utf-8')
            result = subprocess.run([POWERSHELL, '-NoProfile', '-NonInteractive', '-File', str(script)],
                                    env={**os.environ, 'TEST_SOURCE': str(source), 'TEST_TARGET': str(target)},
                                    capture_output=True, text=True, timeout=30)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertEqual(target.read_bytes(), b'candidate')
            self.assertFalse(source.exists())

if __name__ == '__main__':
    unittest.main()
