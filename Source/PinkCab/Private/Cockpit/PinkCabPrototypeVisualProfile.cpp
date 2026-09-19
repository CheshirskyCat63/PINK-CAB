#include "Cockpit/PinkCabPrototypeVisualProfile.h"

bool FPinkCabPrototypeVisualProfile::IsValid() const
{
    if (ProfileId.IsNone() || !VehicleMeshPath.IsValid()
        || !PhysicsAssetPath.IsValid() || WheelBones.Num() != 4)
    {
        return false;
    }
    TSet<FName> Unique;
    for (const FName Bone : WheelBones)
    {
        if (Bone.IsNone()) return false;
        Unique.Add(Bone);
    }
    return Unique.Num() == 4;
}

FPinkCabPrototypeVisualProfile FPinkCabPrototypeVisualProfile::EpicSportsCarManny()
{
    FPinkCabPrototypeVisualProfile Result;
    Result.ProfileId = TEXT("Epic.SportsCar.Manny");
    Result.VehicleMeshPath = FSoftObjectPath(
        TEXT("/Game/Vehicles/SportsCar/SKM_SportsCar.SKM_SportsCar"));
    Result.PhysicsAssetPath = FSoftObjectPath(
        TEXT("/Game/Vehicles/SportsCar/PA_SportsCar.PA_SportsCar"));
    Result.DriverMeshPath = FSoftObjectPath(
        TEXT("/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple.SKM_Manny_Simple"));
    Result.CockpitRootTransform = FTransform::Identity;
    Result.DriverTransform = FTransform(
        FRotator(0.0f, 0.0f, 0.0f),
        FVector(-35.0f, -38.0f, -55.0f),
        FVector(0.92f));
    Result.WheelBones = {
        TEXT("Phys_Wheel_FL"), TEXT("Phys_Wheel_FR"),
        TEXT("Phys_Wheel_BL"), TEXT("Phys_Wheel_BR")};
    return Result;
}
