import os
import unreal

srcdir=os.environ.get("PINKCAB_TATRA_V12_CLEAN_DIR")
if not srcdir:
    raise RuntimeError("PINKCAB_TATRA_V12_CLEAN_DIR is required")

src=os.path.join(srcdir,"Tatra613_V12_Wheel.glb")
dst="/Game/Dev/Vehicles/Tatra613ArchiveV12Clean/Tatra613_V12_Wheel"
if not os.path.isfile(src):
    raise RuntimeError("Tatra wheel GLB missing: "+src)

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
    "/Game/Dev/Vehicles/Tatra613ArchiveV12Clean",
    only_if_is_dirty=False,
    recursive=True)

wheel_path=(
    "/Game/Dev/Vehicles/Tatra613ArchiveV12Clean/Tatra613_V12_Wheel/"
    "StaticMeshes/Tatra613_V12_Wheel.Tatra613_V12_Wheel"
)
wheel=unreal.EditorAssetLibrary.load_asset(wheel_path)
if not wheel:
    raise RuntimeError("Validated Tatra wheel asset missing after import: "+wheel_path)
unreal.log("TATRA_V12_WHEEL_IMPORT_OK "+wheel_path)
