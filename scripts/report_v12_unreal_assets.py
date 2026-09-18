import unreal
paths=[
'/Game/Dev/Vehicles/Tatra613ArchiveV12Clean/Tatra613_V12_Body/StaticMeshes/Tatra613_V12_Body.Tatra613_V12_Body',
'/Game/Dev/Vehicles/Tatra613ArchiveV12Clean/Tatra613_V12_Steering/StaticMeshes/Tatra613_V12_Steering.Tatra613_V12_Steering',
'/Game/Dev/Vehicles/Tatra613ArchiveV12Clean/Tatra613_V12_Wheel/StaticMeshes/Tatra613_V12_Wheel.Tatra613_V12_Wheel']
for p in paths:
    a=unreal.EditorAssetLibrary.load_asset(p)
    unreal.log('ASSET '+p+' loaded='+str(a is not None))
    if a:
        b=a.get_bounds()
        unreal.log('BOUNDS origin='+str(b.origin)+' extent='+str(b.box_extent)+' radius='+str(b.sphere_radius))
