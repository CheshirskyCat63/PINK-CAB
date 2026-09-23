from __future__ import annotations

import tempfile
import unittest
from pathlib import Path

from scripts.repo_hygiene import REQUIRED_COOK_LINES, scan_repository


def write(root: Path, relative: str, text: str) -> None:
    path = root / relative
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(text, encoding="utf-8")


def clean_config() -> str:
    return "\n".join(sorted(REQUIRED_COOK_LINES)) + "\n"


class RepoHygieneTests(unittest.TestCase):
    def test_rejects_personal_path_in_active_script(self) -> None:
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            write(root, "scripts/tool.py", 'path = r"C:\\Users\\Developer\\Downloads\\asset.glb"\n')
            write(root, "Config/DefaultGame.ini", clean_config())
            rules = [item["rule"] for item in scan_repository(root)]
            self.assertIn("personal_user_path", rules)

    def test_ignores_archived_script_history(self) -> None:
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            write(root, "scripts/archive/old.py", 'path = r"C:\\Users\\Developer\\Downloads\\asset.glb"\n')
            write(root, "Config/DefaultGame.ini", clean_config())
            self.assertEqual([], scan_repository(root))

    def test_rejects_broad_tatra_cook_root(self) -> None:
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            write(root, "Config/DefaultGame.ini", clean_config()
                + '+DirectoriesToAlwaysCook=(Path="/Game/Dev/Vehicles/Tatra613Donor")\n')
            rules = [item["rule"] for item in scan_repository(root)]
            self.assertIn("forbidden_broad_cook", rules)

    def test_rejects_stale_tatra_content_root(self) -> None:
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            write(root, "Content/Dev/Vehicles/Tatra613Donor/Old.uasset", "pointer")
            write(root, "Config/DefaultGame.ini", clean_config())
            rules = [item["rule"] for item in scan_repository(root)]
            self.assertIn("stale_content_root", rules)

    def test_rejects_superseded_active_tatra_tool(self) -> None:
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            write(root, "scripts/import_tatra_v12_clean.py", "print('old')\n")
            write(root, "Config/DefaultGame.ini", clean_config())
            rules = [item["rule"] for item in scan_repository(root)]
            self.assertIn("forbidden_active_tool", rules)

    def test_clean_repository_shape_passes(self) -> None:
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            write(root, "scripts/tool.py", 'path = os.environ.get("ASSET_SOURCE")\n')
            write(root, "Config/DefaultGame.ini", clean_config())
            self.assertEqual([], scan_repository(root))


if __name__ == "__main__":
    unittest.main()
