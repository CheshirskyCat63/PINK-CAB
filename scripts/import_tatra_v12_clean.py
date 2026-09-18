import unreal, os
srcdir=r"C:\Users\CheCat\Downloads\tatra_613_1975-1996\v12_clean"
dst='/Game/Dev/Vehicles/Tatra613ArchiveV12Clean'
asset_tools=unreal.AssetToolsHelpers.get_asset_tools()
for fn in ['Tatra613_V12_Body.glb','Tatra613_V12_Steering.glb','Tatra613_V12_Wheel.glb']:
    task=unreal.AssetImportTask()
    task.filename=os.path.join(srcdir,fn)
    task.destination_path=dst
    task.automated=True
    task.replace_existing=True
    task.save=True
    asset_tools.import_asset_tasks([task])
    unreal.log('V12_CLEAN_IMPORT '+fn+' -> '+repr(task.imported_object_paths))
unreal.EditorAssetLibrary.save_directory(dst, only_if_is_dirty=False, recursive=True)
