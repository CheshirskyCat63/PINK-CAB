#pragma once

#include "CoreMinimal.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Taxi/PinkCabFarePassengerManifest.h"

struct PINKCABTAXI_API FPinkCabPassengerPlaceholderLayout
{
    float ForwardOffsetCm = 0.0f;
    float SpacingCm = 65.0f;
    float HeightCm = 90.0f;

    bool IsValid() const;
};

using FPinkCabPassengerPlaceholderTransforms =
    TArray<FTransform, TInlineAllocator<5>>;

class PINKCABTAXI_API FPinkCabFarePassengerPresentation
{
public:
    static bool BuildLocalTransforms(
        const FPinkCabFarePassengerManifest& Manifest,
        const FPinkCabPassengerPlaceholderLayout& Layout,
        FPinkCabPassengerPlaceholderTransforms& OutTransforms);
    static int32 SyncInstances(
        UInstancedStaticMeshComponent& Instances,
        const FPinkCabFarePassengerManifest& Manifest,
        const FPinkCabPassengerPlaceholderLayout& Layout);
    static void Clear(UInstancedStaticMeshComponent& Instances);
};
