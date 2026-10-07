import unreal, os, json

project_root=os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
source_root=os.environ.get(
    "PINKCAB_TATRA_RIG06_PACKAGE",
    r"E:\CHESHIRE_DIVISION\SourceAssets\PINK-CAB\Tatra613\working\export\TATRA613_RIG06_UE")
fbx=os.path.join(source_root,"TATRA613_CHAOS_RIG_06_TEXTURED_OPENABLES.fbx")
dest="/Game/Dev/Vehicles/Tatra613Rig06"
unreal.SystemLibrary.execute_console_command(None, "Interchange.FeatureFlags.Import.FBX False")
asset_tools=unreal.AssetToolsHelpers.get_asset_tools()

if not os.path.isfile(fbx):
    raise RuntimeError("Missing RIG06 FBX: "+fbx)

if unreal.EditorAssetLibrary.does_directory_exist(dest):
    unreal.EditorAssetLibrary.delete_directory(dest)

ui=unreal.FbxImportUI()
ui.set_editor_property("import_mesh", True)
ui.set_editor_property("import_as_skeletal", True)
ui.set_editor_property("import_animations", False)
ui.set_editor_property("import_materials", True)
ui.set_editor_property("import_textures", True)
ui.set_editor_property("create_physics_asset", False)
ui.set_editor_property("automated_import_should_detect_type", False)
try:
    ui.set_editor_property("mesh_type_to_import", unreal.FBXImportType.FBXIT_SKELETAL_MESH)
except Exception as e:
    unreal.log_warning("mesh_type_to_import property unavailable: "+str(e))
try:
    smid=ui.get_editor_property("skeletal_mesh_import_data")
    smid.set_editor_property("import_mesh_lods", False)
    smid.set_editor_property("import_morph_targets", False)
    smid.set_editor_property("preserve_smoothing_groups", True)
    smid.set_editor_property("import_meshes_in_bone_hierarchy", True)
except Exception as e:
    unreal.log_warning("skeletal import detail warning: "+str(e))

task=unreal.AssetImportTask()
task.filename=fbx
task.destination_path=dest
task.destination_name="SK_Tatra613_Rig06"
task.automated=True
task.replace_existing=True
task.replace_existing_settings=True
task.save=True
task.options=ui
asset_tools.import_asset_tasks([task])

unreal.EditorAssetLibrary.save_directory(dest,only_if_is_dirty=False,recursive=True)
assets=unreal.EditorAssetLibrary.list_assets(dest,recursive=True,include_folder=False)
classes={}
skeletal=[]
for p in assets:
    a=unreal.load_asset(p)
    cn=a.get_class().get_name() if a else "<missing>"
    classes.setdefault(cn,[]).append(str(p))
    if cn=="SkeletalMesh":
        skeletal.append(a)

if len(skeletal)!=1:
    raise RuntimeError("Expected one SkeletalMesh, got "+str([a.get_name() for a in skeletal]))

sk=skeletal[0]
bone_names=[]
try:
    ref=sk.get_editor_property("ref_skeleton")
    bone_names=[str(x) for x in ref.get_raw_bone_names()]
except Exception as e:
    unreal.log_warning("ref_skeleton Python access unavailable: "+str(e))
    try:
        skeleton=sk.get_editor_property("skeleton")
        if skeleton:
            bone_tree=skeleton.get_reference_skeleton()
            bone_names=[str(x) for x in bone_tree.get_raw_bone_names()]
    except Exception as e2:
        unreal.log_warning("skeleton bone read unavailable: "+str(e2))

material_slots=[]
for prop in ("materials","skeletal_materials"):
    try:
        arr=sk.get_editor_property(prop)
        for i,m in enumerate(arr):
            slot=None
            iface=None
            for sn in ("material_slot_name","imported_material_slot_name"):
                try:
                    v=m.get_editor_property(sn)
                    if v: slot=str(v)
                except Exception: pass
            try:
                iface=m.get_editor_property("material_interface")
            except Exception: pass
            material_slots.append({"index":i,"slot":slot,"material":str(iface.get_path_name()) if iface else None})
        if material_slots: break
    except Exception:
        pass

required={"root","Phys_Wheel_FL","Phys_Wheel_FR","Phys_Wheel_BL","Phys_Wheel_BR",
          "Steering_Wheel","Cabin_GearLever","Cabin_Handbrake",
          "Door_FL","Door_FR","Door_RL","Door_RR","Trunk_Front","Hood_Rear"}
if bone_names:
    missing=sorted(required-set(bone_names))
    if missing:
        raise RuntimeError("RIG06 imported but missing required bones: "+str(missing))
else:
    missing=[]

report={
    "source":fbx,
    "destination":dest,
    "assets":assets,
    "classes":classes,
    "skeletal_mesh":sk.get_path_name(),
    "bones":bone_names,
    "bone_count":len(bone_names),
    "required_missing":missing,
    "materials":material_slots,
}
out=os.path.join(project_root,"Saved","Tatra613Rig06Import_report.json")
os.makedirs(os.path.dirname(out),exist_ok=True)
with open(out,"w",encoding="utf-8") as f: json.dump(report,f,indent=2)
unreal.log("TATRA_RIG06_IMPORT_OK "+json.dumps(report))
