"""Exercise the Windows shell used by the simplified workstation CI."""
import hashlib
import os
from pathlib import Path
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[2]

@unittest.skipUnless(os.name == "nt", "Windows workstation shell contract")
class WorkstationShellTests(unittest.TestCase):
    def test_new_workflows_use_powershell_and_hash_cmdlet_is_available(self):
        for name in ("verify.yml", "deliver.yml"):
            text = (ROOT / ".github/workflows" / name).read_text(encoding="utf-8")
            self.assertIn("shell: pwsh", text, name)

        shell = Path(os.environ["SystemRoot"]) / "System32/WindowsPowerShell/v1.0/powershell.exe"
        with tempfile.TemporaryDirectory() as temp:
            fixture = Path(temp) / "hash-fixture.txt"
            payload = b"PINKCAB exact-source evidence"
            fixture.write_bytes(payload)
            command = "(Get-FileHash -LiteralPath $env:PINKCAB_HASH_FIXTURE -Algorithm SHA256).Hash.ToLowerInvariant()"
            env = dict(os.environ)
            env["PINKCAB_HASH_FIXTURE"] = str(fixture)
            result = subprocess.run(
                [str(shell), "-NoProfile", "-NonInteractive", "-Command", command],
                env=env, capture_output=True, text=True, timeout=30)
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertEqual(result.stdout.strip(), hashlib.sha256(payload).hexdigest())

if __name__ == "__main__":
    unittest.main()
