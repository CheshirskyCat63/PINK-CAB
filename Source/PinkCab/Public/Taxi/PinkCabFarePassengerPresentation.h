#pragma once

#include "CoreMinimal.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Taxi/PinkCabFarePassengerManifest.h"

struct FPinkCabPassengerPlaceholderLayout
{
    float ForwardOffsetCm = 0.0f;
    float SpacingCm = 65.0f;
    float HeightCm = 90.0f;

    bool IsValid() const
    {
        return FMath::IsFinite(ForwardOffsetCm)
            && FMath::IsFinite(SpacingCm)
            && FMath::IsFinite(HeightCm)
            && SpacingCm > 0.0f;
    }
};

using FPinkCabPassengerPlaceholderTransforms =
    TArray<FTransform, TInlineAllocator<5>>;

class FPinkCabFarePassengerPresentation
{
public:
    static bool BuildLocalTransforms(
        const FPinkCabFarePassengerManifest& Manifest,
        const FPinkCabPassengerPlaceholderLayout& Layout,
        FPinkCabPassengerPlaceholderTransforms& OutTransforms)
    {
        const int32 Count = Manifest.GetRecords().Num();
        if (!Layout.IsValid() || Count < 1 || Count > 5)
        {
            OutTransforms.Reset();
            return false;
        }

        OutTransforms.Reset(Count);
        const float CenterIndex = 0.5f * static_cast<float>(Count - 1);
        for (int32 Index = 0; Index < Count; ++Index)
        {
            const float LateralCm =
                (static_cast<float>(Index) - CenterIndex) * Layout.SpacingCm;
            OutTransforms.Emplace(
                FRotator::ZeroRotator,
                FVector(Layout.ForwardOffsetCm, LateralCm, Layout.HeightCm),
                FVector::OneVector);
        }
        return true;
    }

    static int32 SyncInstances(
        UInstancedStaticMeshComponent& Instances,
        const FPinkCabFarePassengerManifest& Manifest,
        const FPinkCabPassengerPlaceholderLayout& Layout)
    {
        FPinkCabPassengerPlaceholderTransforms Transforms;
        Instances.ClearInstances();
        if (!BuildLocalTransforms(Manifest, Layout, Transforms))
        {
            return 0;
        }
        for (const FTransform& Transform : Transforms)
        {
            Instances.AddInstance(Transform);
        }
        return Instances.GetInstanceCount();
    }

    static void Clear(UInstancedStaticMeshComponent& Instances)
    {
        Instances.ClearInstances();
    }
};
