"""Verify the simplified workflows use the declared PowerShell shell."""
from pathlib import Path
import unittest

ROOT = Path(__file__).resolve().parents[2]


class WorkstationShellTests(unittest.TestCase):
    def test_new_workflows_use_pwsh(self):
        for name in ("verify.yml", "deliver.yml"):
            text = (ROOT / ".github/workflows" / name).read_text(encoding="utf-8")
            self.assertIn("shell: pwsh", text, name)


if __name__ == "__main__":
    unittest.main()
