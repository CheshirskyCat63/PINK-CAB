import json
import os
import unreal

project_root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
src = os.path.join(project_root, "Saved", "Tatra613DesktopExport")
root = '/Game/Dev/Vehicles/Tatra613DesktopClean'
asset_tools = unreal.AssetToolsHelpers.get_asset_tools()

entry = ('Tatra613_Desktop_Body.glb', root + '/Body')
filename, destination = entry
task = unreal.AssetImportTask()
task.filename = os.path.join(src, filename)
task.destination_path = destination
task.automated = True
task.replace_existing = True
task.save = True
task.options = None
asset_tools.import_asset_tasks([task])
unreal.log('DESKTOP_TATRA_IMPORT '+filename+' -> '+str(task.imported_object_paths))

unreal.EditorAssetLibrary.save_directory(root, only_if_is_dirty=False, recursive=True)

# Interchange may import alpha through an opaque glTF parent. Force exactly one cheap,
# visible window material in Unreal. No refraction, no texture sampling, no model edits.
glass_path = root + '/Common/M_PC_T613_GlassCheap'
glass = unreal.load_asset(glass_path)
if not glass:
    glass = asset_tools.create_asset(
        'M_PC_T613_GlassCheap', root + '/Common', unreal.Material, unreal.MaterialFactoryNew())
if not glass:
    raise RuntimeError('Could not create Tatra glass material')

unreal.MaterialEditingLibrary.delete_all_material_expressions(glass)
color = unreal.MaterialEditingLibrary.create_material_expression(
    glass, unreal.MaterialExpressionConstant3Vector, -320, -40)
color.set_editor_property('constant', unreal.LinearColor(0.04, 0.055, 0.065, 1.0))
opacity = unreal.MaterialEditingLibrary.create_material_expression(
    glass, unreal.MaterialExpressionConstant, -320, 80)
opacity.set_editor_property('r', 0.15)

unreal.MaterialEditingLibrary.connect_material_property(
    color, '', unreal.MaterialProperty.MP_BASE_COLOR)
unreal.MaterialEditingLibrary.connect_material_property(
    opacity, '', unreal.MaterialProperty.MP_OPACITY)

glass.set_editor_property('blend_mode', unreal.BlendMode.BLEND_TRANSLUCENT)
glass.set_editor_property('two_sided', True)
glass.set_editor_property('shading_model', unreal.MaterialShadingModel.MSM_DEFAULT_LIT)
unreal.MaterialEditingLibrary.recompile_material(glass)
unreal.EditorAssetLibrary.save_loaded_asset(glass)

body_path = root + '/Body/Tatra613_Desktop_Body/StaticMeshes/Tatra613_Desktop_Body'
body = unreal.load_asset(body_path)
if not body:
    raise RuntimeError('Faithful desktop Tatra body missing after import')

mats = body.get_editor_property('static_materials')
replaced = 0
for slot in mats:
    material = slot.material_interface
    if material and material.get_name() == 'PC_T613_GlassCheap':
        slot.material_interface = glass
        replaced += 1
body.set_editor_property('static_materials', mats)
unreal.EditorAssetLibrary.save_loaded_asset(body)
if replaced != 1:
    raise RuntimeError('Expected exactly one Tatra glass material slot, got '+str(replaced))

assets = [str(p) for p in unreal.EditorAssetLibrary.list_assets(root, recursive=True, include_folder=False)]
report = os.path.join(project_root, "Saved", "Tatra613Desktop_UnrealAssets.json")
with open(report, 'w', encoding='utf-8') as f:
    json.dump({'imported': [str(x) for x in task.imported_object_paths], 'assets': assets}, f, indent=2)

unreal.log('DESKTOP_TATRA_FAITHFUL_IMPORT_DONE '+report)
unreal.log('DESKTOP_GLASS_FIX replaced='+str(replaced)+' material='+str(glass))
