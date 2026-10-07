"""Regression contract for the RIG06 V23 authoring/import pipeline."""

from pathlib import Path
import re
import unittest

SCRIPTS = Path(__file__).resolve().parents[1]


class TatraRig06AuthoringContract(unittest.TestCase):
    def test_v23_source_is_geometry_guarded_and_uv_scoped(self):
        text = (SCRIPTS / "prepare_tatra_rig06_v23.py").read_text(encoding="utf-8-sig")
        self.assertIn("TATRA613_CHAOS_RIG_22_FINAL.blend", text)
        self.assertIn("TATRA613_CHAOS_RIG_23_MATERIAL_UV.blend", text)
        self.assertIn("geometry_bones_unchanged", text)
        self.assertIn("if before != after", text)
        self.assertIn("use_tspace=False", text)
        names = set(re.findall(r'"(DONOR_(?:Pedal|Mount|Bearing|Spacer)_[A-Za-z]+)"', text))
        self.assertEqual(len(names), 12, sorted(names))

    def test_import_preserves_authored_normals_and_generates_mikk_tangents(self):
        text = (SCRIPTS / "import_tatra_rig06.py").read_text(encoding="utf-8-sig")
        self.assertIn("TATRA613_RIG23_UE", text)
        self.assertIn("FBXNIM_IMPORT_NORMALS", text)
        self.assertNotIn("FBXNIM_IMPORT_NORMALS_AND_TANGENTS", text)
        self.assertIn("MIKK_T_SPACE", text)
        self.assertRegex(
            text,
            r'set_editor_property\(\s*"compute_weighted_normals",\s*False\s*\)',
        )
        self.assertNotIn("delete_directory(dest)", text)
        self.assertIn("scripted_add_filename(fbx,0,\"\")", text)
        self.assertIn("replace_existing_settings=not reimporting", text)
        self.assertIn('set_editor_property("import_materials", not reimporting)', text)
        self.assertIn('set_editor_property("import_textures", not reimporting)', text)
        self.assertIn("only_if_is_dirty=True", text)
        self.assertIn("RIG06 import did not preserve authored normals", text)

    def test_material_pipeline_uses_v23_and_normalizes_all_source_textures(self):
        text = (SCRIPTS / "configure_tatra_rig06_materials.py").read_text(
            encoding="utf-8-sig"
        )
        self.assertIn("TATRA613_RIG23_UE", text)
        self.assertIn('for filename in texture_files:', text)
        self.assertIn('name.endswith("_BaseColor")', text)
        self.assertIn('name.endswith("_Normal")', text)
        self.assertIn('name.endswith("_ORM")', text)
        self.assertIn("TC_NORMALMAP", text)
        self.assertIn("TC_MASKS", text)


if __name__ == "__main__":
    unittest.main()
