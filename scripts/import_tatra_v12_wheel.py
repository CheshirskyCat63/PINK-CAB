import os
import unreal

srcdir=os.environ.get("PINKCAB_TATRA_V12_CLEAN_DIR")
if not srcdir:
    raise RuntimeError("PINKCAB_TATRA_V12_CLEAN_DIR is required")

src=os.path.join(srcdir,"Tatra613_V12_Wheel.glb")
base_root="/Game/Dev/Vehicles/Tatra613ArchiveV12Clean"
dst=base_root+"/Tatra613_V12_Wheel"
stale_roots=[
    base_root+"/Tatra613_V12_Body",
    base_root+"/Tatra613_V12_Steering",
]
if not os.path.isfile(src):
    raise RuntimeError("Tatra wheel GLB missing: "+src)

for stale in stale_roots:
    if unreal.EditorAssetLibrary.does_directory_exist(stale):
        unreal.EditorAssetLibrary.delete_directory(stale)
if unreal.EditorAssetLibrary.does_directory_exist(dst):
    unreal.EditorAssetLibrary.delete_directory(dst)

asset_tools=unreal.AssetToolsHelpers.get_asset_tools()
task=unreal.AssetImportTask()
task.filename=src
task.destination_path="/Game/Dev/Vehicles/Tatra613ArchiveV12Clean"
task.automated=True
task.replace_existing=True
task.save=True
asset_tools.import_asset_tasks([task])

unreal.EditorAssetLibrary.save_directory(
    base_root,
    only_if_is_dirty=False,
    recursive=True)

unexpected=[
    path for path in unreal.EditorAssetLibrary.list_assets(
        base_root,recursive=True,include_folder=False)
    if not str(path).startswith(dst+"/")
]
if unexpected:
    raise RuntimeError("Unexpected assets remain in canonical V12Clean root: "+str(unexpected))

wheel_path=(
    "/Game/Dev/Vehicles/Tatra613ArchiveV12Clean/Tatra613_V12_Wheel/"
    "StaticMeshes/Tatra613_V12_Wheel.Tatra613_V12_Wheel"
)
wheel=unreal.EditorAssetLibrary.load_asset(wheel_path)
if not wheel:
    raise RuntimeError("Validated Tatra wheel asset missing after import: "+wheel_path)
unreal.log("TATRA_V12_WHEEL_IMPORT_OK "+wheel_path)
