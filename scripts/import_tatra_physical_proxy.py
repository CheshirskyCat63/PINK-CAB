"""Import only the new invisible physical proxy. Accepted RIG24 visuals are untouched."""
import hashlib
import json
import os
from pathlib import Path
import unreal

source = Path(os.environ['PINKCAB_TATRA_PHYSICAL_SOURCE'])
recipe = json.loads((source / 'Tatra613_Physical.recipe.json').read_text(encoding='utf-8'))
fbx = source / 'Tatra613_Physical.fbx'
if hashlib.sha256(fbx.read_bytes()).hexdigest() != recipe['fbx_sha256']:
    raise RuntimeError('Physical FBX hash differs from authoring receipt')
destination = '/Game/Dev/Vehicles/Tatra613Physics/V1'
mesh_path = destination + '/SK_Tatra613_Physical'
reimport = unreal.EditorAssetLibrary.does_asset_exist(mesh_path)
if reimport and os.environ.get('PINKCAB_TATRA_PHYSICAL_REIMPORT') != '1':
    raise RuntimeError('Physical asset already exists: explicit owned-proxy reimport required')
unreal.SystemLibrary.execute_console_command(None, 'Interchange.FeatureFlags.Import.FBX False')
ui = unreal.FbxImportUI()
ui.import_mesh = True
ui.import_as_skeletal = True
ui.import_animations = False
ui.import_materials = False
ui.import_textures = False
ui.create_physics_asset = False
ui.automated_import_should_detect_type = False
ui.mesh_type_to_import = unreal.FBXImportType.FBXIT_SKELETAL_MESH
settings = ui.skeletal_mesh_import_data
settings.set_editor_property('import_uniform_scale', 1.0)
settings.set_editor_property('import_meshes_in_bone_hierarchy', False)
settings.set_editor_property('import_morph_targets', False)
settings.set_editor_property('import_mesh_lods', False)
settings.set_editor_property('normal_import_method', unreal.FBXNormalImportMethod.FBXNIM_COMPUTE_NORMALS)
task = unreal.AssetImportTask()
task.filename = str(fbx)
task.destination_path = destination
task.destination_name = 'SK_Tatra613_Physical'
task.automated = True
task.replace_existing = reimport
task.replace_existing_settings = reimport
task.save = True
task.options = ui
if reimport:
    old = unreal.load_asset(mesh_path)
    data = old.get_editor_property('asset_import_data')
    data.set_editor_property('import_uniform_scale', 1.0)
    data.scripted_add_filename(str(fbx), 0, '')
    unreal.EditorAssetLibrary.save_loaded_asset(old, only_if_is_dirty=True)
unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
mesh = unreal.load_asset(mesh_path)
if not mesh or not isinstance(mesh, unreal.SkeletalMesh):
    raise RuntimeError('Physical mesh import failed')
component = unreal.new_object(unreal.SkeletalMeshComponent)
component.set_skeletal_mesh_asset(mesh)
names = [str(component.get_bone_name(i)) for i in range(component.get_num_bones())]
required = ['Root'] + list(recipe['wheel_bones_cm'])
if any(n not in names for n in required):
    raise RuntimeError(f'Missing physical bones: {names}')
report = {'source_recipe_sha256': hashlib.sha256((source/'Tatra613_Physical.recipe.json').read_bytes()).hexdigest(),
          'mesh': mesh.get_path_name(), 'bones': names,
          'import_scale': settings.get_editor_property('import_uniform_scale'),
          'imported_paths': list(task.imported_object_paths)}
output = Path(unreal.Paths.project_saved_dir()) / 'VF90' / 'task5-physical-import.json'
output.write_text(json.dumps(report, indent=2), encoding='utf-8')
unreal.log('TATRA_PHYSICAL_IMPORT=PASS ' + json.dumps(report))
