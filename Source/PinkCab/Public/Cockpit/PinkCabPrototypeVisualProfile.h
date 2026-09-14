#pragma once

#include "CoreMinimal.h"
#include "UObject/SoftObjectPath.h"

struct FPinkCabPrototypeVisualProfile
{
    FName ProfileId = NAME_None;
    FSoftObjectPath VehicleMeshPath;
    FSoftObjectPath PhysicsAssetPath;
    FSoftObjectPath DriverMeshPath;
    FTransform CockpitRootTransform = FTransform::Identity;
    FTransform DriverTransform = FTransform::Identity;
    TArray<FName> WheelBones;

    bool IsValid() const
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

    static FPinkCabPrototypeVisualProfile EpicSportsCarManny()
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
};
