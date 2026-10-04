import re
import unittest
from pathlib import Path

REPO = Path(__file__).resolve().parents[2]
AUTHOR = REPO / "scripts" / "vehicles" / "author_vehicle.py"
WRAPPER = REPO / "scripts" / "vehicles" / "author-vehicle.ps1"


class AuthorVehicleContractTests(unittest.TestCase):
    def setUp(self):
        self.author = AUTHOR.read_text(encoding="utf-8")
        self.wrapper = WRAPPER.read_text(encoding="utf-8")

    def test_author_uses_manifest_contract_and_stable_digest(self):
        self.assertIn("load_manifest", self.author)
        self.assertIn("manifest_digest", self.author)
        self.assertIn("manifest_digest", self.author)
        self.assertIn("receipt", self.author.lower())

    def test_author_updates_exact_asset_in_place(self):
        self.assertIn("does_asset_exist", self.author)
        self.assertIn("load_asset", self.author)
        self.assertIn("create_asset", self.author)
        self.assertIn("save_loaded_asset", self.author)
        self.assertNotIn("delete_asset(", self.author)
        self.assertNotIn("delete_directory(", self.author)

    def test_author_fails_closed_on_missing_assets(self):
        self.assertRegex(self.author, r"raise RuntimeError")
        self.assertIn("load_required_asset", self.author)
        self.assertIn("validate_definition_fields", self.author)

    def test_wrapper_runs_exact_unreal_python_commandlet(self):
        self.assertIn("UnrealEditor-Cmd.exe", self.wrapper)
        self.assertIn("-run=pythonscript", self.wrapper)
        self.assertIn("author_vehicle.py", self.wrapper)
        self.assertIn("PINKCAB_VEHICLE_MANIFEST", self.wrapper)
        self.assertIn("PINKCAB_VEHICLE_RECEIPT", self.wrapper)

    def test_wrapper_does_not_mutate_source_or_git(self):
        lowered = self.wrapper.lower()
        self.assertNotIn("git reset", lowered)
        self.assertNotIn("git clean", lowered)
        self.assertNotIn("remove-item", lowered)
        self.assertNotIn("copy-item", lowered)

    def test_author_has_idempotent_receipt_fields(self):
        for token in (
            '"manifest_digest"',
            '"definition_asset"',
            '"vehicle_id"',
            '"presentation_part_count"',
            '"articulation_count"',
            '"wheel_count"',
        ):
            self.assertIn(token, self.author)


if __name__ == "__main__":
    unittest.main()
