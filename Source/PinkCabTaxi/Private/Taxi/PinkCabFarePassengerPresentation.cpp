#include "Taxi/PinkCabFarePassengerPresentation.h"

bool FPinkCabPassengerPlaceholderLayout::IsValid() const
{
    return FMath::IsFinite(ForwardOffsetCm)
        && FMath::IsFinite(SpacingCm)
        && FMath::IsFinite(HeightCm)
        && SpacingCm > 0.0f;
}

bool FPinkCabFarePassengerPresentation::BuildLocalTransforms(
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

int32 FPinkCabFarePassengerPresentation::SyncInstances(
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

void FPinkCabFarePassengerPresentation::Clear(
    UInstancedStaticMeshComponent& Instances)
{
    Instances.ClearInstances();
}
