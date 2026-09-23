import json
import os
import unreal

project_root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
src = os.path.join(project_root, "Saved", "Tatra613FaithfulExport")
root = '/Game/Dev/Vehicles/Tatra613DesktopFaithful'
asset_tools = unreal.AssetToolsHelpers.get_asset_tools()

if unreal.EditorAssetLibrary.does_directory_exist(root):
    unreal.EditorAssetLibrary.delete_directory(root)

task = unreal.AssetImportTask()
task.filename = os.path.join(src, 'Tatra613_FaithfulBody.glb')
task.destination_path = root + '/Body'
task.automated = True
task.replace_existing = True
task.save = True
task.options = None
asset_tools.import_asset_tasks([task])
unreal.log('TATRA_FAITHFUL_IMPORT '+str(task.imported_object_paths))

unreal.EditorAssetLibrary.save_directory(root, only_if_is_dirty=False, recursive=True)

body_path = root + '/Body/Tatra613_FaithfulBody/StaticMeshes/Tatra613_FaithfulBody'
body = unreal.load_asset(body_path)
if not body:
    raise RuntimeError('Faithful Tatra body missing after import: ' + body_path)

glass_path = root + '/Common/M_PC_T613_GlassCheap'
glass = asset_tools.create_asset(
    'M_PC_T613_GlassCheap', root + '/Common', unreal.Material, unreal.MaterialFactoryNew())
if not glass:
    raise RuntimeError('Could not create faithful Tatra glass material')

unreal.MaterialEditingLibrary.delete_all_material_expressions(glass)
color = unreal.MaterialEditingLibrary.create_material_expression(
    glass, unreal.MaterialExpressionConstant3Vector, -320, -40)
color.set_editor_property('constant', unreal.LinearColor(0.025, 0.035, 0.045, 1.0))
opacity = unreal.MaterialEditingLibrary.create_material_expression(
    glass, unreal.MaterialExpressionConstant, -320, 80)
opacity.set_editor_property('r', 0.16)
unreal.MaterialEditingLibrary.connect_material_property(
    color, '', unreal.MaterialProperty.MP_BASE_COLOR)
unreal.MaterialEditingLibrary.connect_material_property(
    opacity, '', unreal.MaterialProperty.MP_OPACITY)
glass.set_editor_property('blend_mode', unreal.BlendMode.BLEND_TRANSLUCENT)
glass.set_editor_property('two_sided', True)
glass.set_editor_property('shading_model', unreal.MaterialShadingModel.MSM_DEFAULT_LIT)
unreal.MaterialEditingLibrary.recompile_material(glass)
unreal.EditorAssetLibrary.save_loaded_asset(glass)

mats = body.get_editor_property('static_materials')
slot_names = []
replaced = 0
for index, slot in enumerate(mats):
    material = slot.material_interface
    name = material.get_name() if material else 'NONE'
    slot_names.append(name)
    if 'PC_T613_GlassSlot' in name:
        body.set_material(index, glass)
        replaced += 1
unreal.EditorAssetLibrary.save_loaded_asset(body)

if replaced != 1:
    raise RuntimeError('Expected exactly one faithful glass slot, got '+str(replaced)+' slots='+str(slot_names))

bounds = body.get_bounds()
assets = [str(p) for p in unreal.EditorAssetLibrary.list_assets(root, recursive=True, include_folder=False)]
report = os.path.join(project_root, "Saved", "Tatra613Faithful_UnrealAssets.json")
with open(report, 'w', encoding='utf-8') as f:
    json.dump({
        'imported': [str(x) for x in task.imported_object_paths],
        'assets': assets,
        'body_path': body_path,
        'body_bounds_origin': [bounds.origin.x, bounds.origin.y, bounds.origin.z],
        'body_bounds_extent': [bounds.box_extent.x, bounds.box_extent.y, bounds.box_extent.z],
        'material_slots_before_override': slot_names,
        'glass_replaced': replaced,
    }, f, indent=2)

unreal.log('TATRA_FAITHFUL_IMPORT_DONE '+report)
unreal.log('TATRA_FAITHFUL_GLASS replaced='+str(replaced)+' material='+str(glass))
