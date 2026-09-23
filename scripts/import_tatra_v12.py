import unreal

src = r"C:\Users\CheCat\Downloads\tatra_613_1975-1996\scene.gltf"
dst = "/Game/Dev/Vehicles/Tatra613ArchiveV12"

asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
task = unreal.AssetImportTask()
task.filename = src
task.destination_path = dst
task.automated = True
task.replace_existing = True
task.save = True
asset_tools.import_asset_tasks([task])
unreal.log("V12_IMPORT_PATHS=" + repr(task.imported_object_paths))
unreal.EditorAssetLibrary.save_directory(dst, only_if_is_dirty=False, recursive=True)
