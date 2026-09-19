#pragma once

#include "CoreMinimal.h"
#include "UObject/SoftObjectPath.h"

struct PINKCAB_API FPinkCabPrototypeVisualProfile
{
    FName ProfileId = NAME_None;
    FSoftObjectPath VehicleMeshPath;
    FSoftObjectPath PhysicsAssetPath;
    FSoftObjectPath DriverMeshPath;
    FTransform CockpitRootTransform = FTransform::Identity;
    FTransform DriverTransform = FTransform::Identity;
    TArray<FName> WheelBones;

    bool IsValid() const;
    static FPinkCabPrototypeVisualProfile EpicSportsCarManny();
};
