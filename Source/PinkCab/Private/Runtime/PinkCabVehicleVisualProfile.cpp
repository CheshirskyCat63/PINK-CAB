#include "Runtime/PinkCabVehicleVisualProfile.h"

bool FPinkCabVehiclePresentationPart::IsValid() const
{
    return !PartId.IsNone()
        && !Mesh.IsNull()
        && !LocalTransform.ContainsNaN()
        && !(bOwnerNoSee && bOnlyOwnerSee);
}

bool FPinkCabVehicleArticulationDefinition::IsValid() const
{
    return !ArticulationId.IsNone()
        && !PivotLocal.ContainsNaN()
        && !AxisLocal.ContainsNaN()
        && !AxisLocal.IsNearlyZero()
        && FMath::IsFinite(OpenAngleDegrees)
        && FMath::IsFinite(TravelSeconds)
        && TravelSeconds > KINDA_SMALL_NUMBER
        && PartIds.Num() > 0;
}

FPinkCabVehicleVisualProfile FPinkCabVehicleVisualProfile::Fallback()
{
    FPinkCabVehicleVisualProfile Result;
    Result.ProfileId = TEXT("PinkCab.Visual.Fallback");
    return Result;
}

bool FPinkCabVehicleVisualProfile::IsValid() const
{
    if (ProfileId.IsNone()
        || !FPinkCabCockpitVisualBinding::ValidateUnique(CockpitBindings))
    {
        return false;
    }

    TSet<FName> Seen;
    for (const FPinkCabVehiclePresentationPart& Part : PresentationParts)
    {
        if (!Part.IsValid() || Seen.Contains(Part.PartId))
        {
            return false;
        }
        Seen.Add(Part.PartId);
    }

    TSet<FName> SeenArticulations;
    TSet<FName> ArticulatedParts;
    for (const FPinkCabVehicleArticulationDefinition& Definition : Articulations)
    {
        if (!Definition.IsValid()
            || SeenArticulations.Contains(Definition.ArticulationId))
        {
            return false;
        }
        SeenArticulations.Add(Definition.ArticulationId);
        for (const FName PartId : Definition.PartIds)
        {
            if (!Seen.Contains(PartId) || ArticulatedParts.Contains(PartId))
            {
                return false;
            }
            ArticulatedParts.Add(PartId);
        }
    }
    return true;
}

bool FPinkCabVehicleVisualProfile::HasExteriorAsset() const
{
    return !ExteriorStaticMesh.IsNull() || !ExteriorSkeletalMesh.IsNull();
}

bool FPinkCabVehicleVisualProfile::HasVisualAsset() const
{
    return HasExteriorAsset() || PresentationParts.Num() > 0;
}

bool FPinkCabVehicleVisualProfile::HasCabinAsset() const
{
    return !CabinStaticMesh.IsNull()
        || !CabinSkeletalMesh.IsNull()
        || (bUseExteriorAsCabinWhenCabinMissing && HasExteriorAsset());
}
