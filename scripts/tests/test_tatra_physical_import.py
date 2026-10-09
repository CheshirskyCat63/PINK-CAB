"""Run the importer against a temporary filesystem and a bounded Unreal stub."""
import hashlib
import json
import os
from pathlib import Path
import runpy
import sys
import tempfile
import unittest
from unittest.mock import MagicMock, patch

SCRIPT = Path(__file__).resolve().parents[1] / "import_tatra_physical_proxy.py"


class PhysicalImportReceiptTests(unittest.TestCase):
    def run_import(self, root, blocked=False):
        source = root / "source"
        source.mkdir()
        data = b"physical-import-test-fixture"
        (source / "Tatra613_Physical.fbx").write_bytes(data)
        bones = ["Root", "Phys_Wheel_FL", "Phys_Wheel_FR", "Phys_Wheel_BL", "Phys_Wheel_BR"]
        (source / "Tatra613_Physical.recipe.json").write_text(json.dumps({
            "fbx_sha256": hashlib.sha256(data).hexdigest(),
            "wheel_bones_cm": {name: [0, 0, 0] for name in bones[1:]},
        }), encoding="utf-8")
        saved = root / "Saved"
        if blocked:
            saved.mkdir()
            (saved / "VF90").write_text("not a directory", encoding="utf-8")
        unreal = MagicMock()
        unreal.Paths.project_saved_dir.return_value = str(saved)
        unreal.EditorAssetLibrary.does_asset_exist.return_value = False
        mesh_class = type("TestSkeletalMesh", (), {"get_path_name": lambda self: "/Game/TestPhysicalMesh"})
        unreal.SkeletalMesh = mesh_class
        unreal.load_asset.return_value = mesh_class()
        component = unreal.new_object.return_value
        component.get_num_bones.return_value = len(bones)
        component.get_bone_name.side_effect = lambda index: bones[index]
        unreal.FbxImportUI.return_value.skeletal_mesh_import_data.get_editor_property.return_value = 1.0
        importer = unreal.AssetToolsHelpers.get_asset_tools.return_value.import_asset_tasks

        def check_output_ready(tasks):
            self.assertTrue((saved / "VF90").is_dir(), "receipt directory must exist before asset import")
        importer.side_effect = check_output_ready
        with patch.dict(sys.modules, {"unreal": unreal}), patch.dict(os.environ, {
            "PINKCAB_TATRA_PHYSICAL_SOURCE": str(source),
        }):
            if blocked:
                with self.assertRaises(OSError):
                    runpy.run_path(str(SCRIPT))
                importer.assert_not_called()
            else:
                runpy.run_path(str(SCRIPT))
                importer.assert_called_once()
                report = json.loads((saved / "VF90" / "task5-physical-import.json").read_text())
                self.assertEqual(report["bones"], bones)
                self.assertEqual(report["mesh"], "/Game/TestPhysicalMesh")

    def test_clean_saved_directory_is_created_before_import(self):
        with tempfile.TemporaryDirectory() as directory:
            self.run_import(Path(directory))

    def test_unusable_receipt_directory_prevents_asset_side_effects(self):
        with tempfile.TemporaryDirectory() as directory:
            self.run_import(Path(directory), blocked=True)


if __name__ == "__main__":
    unittest.main()
