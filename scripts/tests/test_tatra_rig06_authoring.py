"""Regression contract for the RIG06 V23/V24 authoring/import pipeline."""

from pathlib import Path
import ast
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

    def test_v24_adds_only_source_pivot_instrument_bindings(self):
        text = (SCRIPTS / "prepare_tatra_rig06_v24.py").read_text(encoding="utf-8-sig")
        self.assertIn("TATRA613_CHAOS_RIG_23_MATERIAL_UV.blend", text)
        self.assertIn("TATRA613_CHAOS_RIG_24_CABIN_BINDINGS.blend", text)
        self.assertIn("geometry_unchanged", text)
        self.assertIn("existing_bones_unchanged", text)
        self.assertIn("t613_needle_speedo", text)
        self.assertIn("t613_needle_tacho", text)
        self.assertIn("t613_needle_fuel", text)
        self.assertIn("t613_needle_temp", text)
        names = set(re.findall(r'"(Cabin_(?:Speedometer|Tachometer|Fuel|Temperature)Needle)"', text))
        self.assertEqual(len(names), 4, sorted(names))
        self.assertNotIn("primitive_cube_add", text)
        self.assertNotIn("modifiers.new", text)

    def test_v24_and_ue_import_guard_the_complete_runtime_bone_inventory(self):
        exporter = (SCRIPTS / "prepare_tatra_rig06_v24.py").read_text(encoding="utf-8-sig")
        importer = (SCRIPTS / "import_tatra_rig06.py").read_text(encoding="utf-8-sig")
        runtime = (SCRIPTS.parent / "Source/PinkCab/Private/Runtime/PinkCabChaosTatraPawnRig06.cpp").read_text(
            encoding="utf-8-sig"
        )
        runtime_block = re.search(
            r"static const FName RequiredBones\[\] = \{(.*?)\};", runtime, re.S
        )
        self.assertIsNotNone(runtime_block)
        runtime_names = set(re.findall(r'TEXT\("([^"]+)"\)', runtime_block.group(1)))
        self.assertEqual(len(runtime_names), 32)

        def declared_names(source):
            tree = ast.parse(source)
            declaration = next(
                n for n in tree.body
                if isinstance(n, ast.Assign)
                and any(isinstance(target, ast.Name) and target.id == "REQUIRED_RIG06_BONES"
                        for target in n.targets)
            )
            return set(ast.literal_eval(declaration.value)), tree

        export_names, export_tree = declared_names(exporter)
        import_names, _ = declared_names(importer)
        self.assertEqual(export_names, runtime_names)
        self.assertEqual(import_names, runtime_names)
        self.assertLess(exporter.index("require_rig06_bones(b.name"), exporter.index("bpy.ops.wm.save_as_mainfile"))
        self.assertLess(exporter.index("required_missing=require_rig06_bones"), exporter.index('print("RIG24_UE_EXPORT_OK"'))
        self.assertIn('if not bone_names:', importer)
        self.assertIn('raise RuntimeError("RIG06 imported but skeletal bone inventory could not be verified")', importer)
        self.assertIn('if missing:', importer)

        # Exercise the actual Blender guard in isolation, without importing bpy.
        guard = next(
            n for n in export_tree.body
            if isinstance(n, ast.FunctionDef) and n.name == "require_rig06_bones"
        )
        declaration = next(
            n for n in export_tree.body
            if isinstance(n, ast.Assign)
            and any(isinstance(target, ast.Name) and target.id == "REQUIRED_RIG06_BONES"
                    for target in n.targets)
        )
        namespace = {}
        module = ast.fix_missing_locations(ast.Module(body=[declaration, guard], type_ignores=[]))
        exec(compile(module, "<isolated-rig06-bone-contract>", "exec"), namespace)
        require = namespace["require_rig06_bones"]
        self.assertEqual(require(runtime_names), [])
        for missing_bone in ("Cabin_Stalk_L", "Cabin_Radio", "Mirror_L"):
            with self.subTest(bone=missing_bone), self.assertRaisesRegex(RuntimeError, missing_bone):
                require(runtime_names - {missing_bone})

    def test_import_preserves_authored_normals_and_generates_mikk_tangents(self):
        text = (SCRIPTS / "import_tatra_rig06.py").read_text(encoding="utf-8-sig")
        self.assertIn("TATRA613_RIG24_UE", text)
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

    def test_material_pipeline_uses_v24_and_normalizes_all_source_textures(self):
        text = (SCRIPTS / "configure_tatra_rig06_materials.py").read_text(
            encoding="utf-8-sig"
        )
        self.assertIn("TATRA613_RIG24_UE", text)
        self.assertIn('for filename in texture_files:', text)
        self.assertIn('name.endswith("_BaseColor")', text)
        self.assertIn('name.endswith("_Normal")', text)
        self.assertIn('name.endswith("_ORM")', text)
        self.assertIn("TC_NORMALMAP", text)
        self.assertIn("TC_MASKS", text)


if __name__ == "__main__":
    unittest.main()
