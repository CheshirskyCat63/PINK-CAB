#include "Vehicle/PinkCabVehicleDefinition.h"

#include "Engine/SkeletalMesh.h"
#include "PhysicsEngine/PhysicsAsset.h"

namespace
{
bool FailDefinition(FString* OutReason, const TCHAR* Reason)
{
    if (OutReason)
    {
        *OutReason = Reason;
    }
    return false;
}

bool HasPresentationPart(
    const FPinkCabVehicleVisualProfile& Profile,
    const FName PartId)
{
    return Profile.PresentationParts.ContainsByPredicate(
        [PartId](const FPinkCabVehiclePresentationPart& Part)
        {
            return Part.PartId == PartId;
        });
}

bool ValidateRequiredAssets(
    const UPinkCabVehicleDefinition& Definition,
    FString* OutReason)
{
    if (Definition.PhysicsCarrierMesh.IsNull())
    {
        return FailDefinition(OutReason, TEXT("PhysicsCarrierMesh is required"));
    }
    if (Definition.PhysicsAsset.IsNull())
    {
        return FailDefinition(OutReason, TEXT("PhysicsAsset is required"));
    }
    if (!Definition.PhysicsCarrierMesh.LoadSynchronous())
    {
        return FailDefinition(OutReason, TEXT("PhysicsCarrierMesh does not resolve"));
    }
    if (!Definition.PhysicsAsset.LoadSynchronous())
    {
        return FailDefinition(OutReason, TEXT("PhysicsAsset does not resolve"));
    }
    if (!Definition.DriverMesh.IsNull() && !Definition.DriverMesh.LoadSynchronous())
    {
        return FailDefinition(OutReason, TEXT("DriverMesh does not resolve"));
    }
    return true;
}

bool ValidateWheelBindings(
    const UPinkCabVehicleDefinition& Definition,
    FString* OutReason)
{
    if (Definition.Wheels.Num() != 4)
    {
        return FailDefinition(OutReason, TEXT("Exactly four wheel bindings are required"));
    }

    TSet<FName> WheelIds;
    TSet<FName> BoneNames;
    TSet<FName> PresentationPartIds;
    for (const FPinkCabVehicleWheelBinding& Wheel : Definition.Wheels)
    {
        if (!Wheel.IsValid())
        {
            return FailDefinition(OutReason, TEXT("Wheel binding contains an empty semantic"));
        }
        if (WheelIds.Contains(Wheel.WheelId)
            || BoneNames.Contains(Wheel.BoneName)
            || PresentationPartIds.Contains(Wheel.PresentationPartId))
        {
            return FailDefinition(OutReason, TEXT("Wheel ids, bones and visual parts must be unique"));
        }
        if (!HasPresentationPart(Definition.VisualProfile, Wheel.PresentationPartId))
        {
            return FailDefinition(OutReason, TEXT("Visual wheel part is missing from VisualProfile"));
        }
        WheelIds.Add(Wheel.WheelId);
        BoneNames.Add(Wheel.BoneName);
        PresentationPartIds.Add(Wheel.PresentationPartId);
    }
    return true;
}
}

bool FPinkCabVehicleWheelBinding::IsValid() const
{
    return !WheelId.IsNone()
        && !BoneName.IsNone()
        && !PresentationPartId.IsNone();
}

bool UPinkCabVehicleDefinition::IsValid(FString* OutReason) const
{
    if (OutReason)
    {
        OutReason->Reset();
    }

    if (VehicleId.IsNone())
    {
        return FailDefinition(OutReason, TEXT("VehicleId is required"));
    }
    if (!ValidateRequiredAssets(*this, OutReason))
    {
        return false;
    }
    if (DriverTransform.ContainsNaN() || DriverHeadTransform.ContainsNaN())
    {
        return FailDefinition(OutReason, TEXT("Driver transforms must be finite"));
    }
    if (!VisualProfile.IsValid())
    {
        return FailDefinition(OutReason, TEXT("VisualProfile is invalid"));
    }
    return ValidateWheelBindings(*this, OutReason);
}

FPinkCabVehicleVisualProfile UPinkCabVehicleDefinition::BuildVisualProfile() const
{
    FPinkCabVehicleVisualProfile Result = VisualProfile;
    Result.DriverHeadTransform = DriverHeadTransform;
    return Result;
}
