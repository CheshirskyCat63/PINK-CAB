"""Exercise the actual Windows shell used by the workstation CI jobs."""
import hashlib
import os
from pathlib import Path
import re
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[2]


@unittest.skipUnless(os.name == 'nt', 'Windows workstation shell contract')
class WorkstationShellTests(unittest.TestCase):
    def test_hash_cmdlet_resolves_in_declared_workflow_environment(self):
        names = ['cd648-p02-phy009.yml', 'cd869-deliver.yml', 'cd869-inline.yml',
                 'pinkcab-g1-github-control-plane.yml', 'pinkcab-vehicle-physics-tdd.yml']
        shells = {}
        for name in names:
            text = (ROOT / '.github/workflows' / name).read_text(encoding='utf-8')
            match = re.search(r"^  PSModulePath: '([^']+)'$", text, re.MULTILINE)
            self.assertIsNotNone(match, name)
            shells.setdefault(match[1], []).append(name)
        shell = Path(os.environ['SystemRoot']) / 'System32/WindowsPowerShell/v1.0/powershell.exe'
        with tempfile.TemporaryDirectory() as temp:
            fixture = Path(temp) / 'hash-fixture.txt'
            payload = b'PINKCAB exact-source evidence'
            fixture.write_bytes(payload)
            for module_path, workflows in shells.items():
                with self.subTest(workflows=workflows):
                    env = {key: value for key, value in os.environ.items() if key.lower() != 'psmodulepath'}
                    env.update(PSModulePath=module_path, PINKCAB_HASH_FIXTURE=str(fixture))
                    command = "$ErrorActionPreference='Stop'; (Get-FileHash -LiteralPath $env:PINKCAB_HASH_FIXTURE -Algorithm SHA256).Hash.ToLowerInvariant()"
                    result = subprocess.run([str(shell), '-NoProfile', '-NonInteractive', '-Command', command],
                                            env=env, capture_output=True, text=True, timeout=30)
                    self.assertEqual(result.returncode, 0, result.stderr)
                    self.assertEqual(result.stdout.strip(), hashlib.sha256(payload).hexdigest())


if __name__ == '__main__':
    unittest.main()
