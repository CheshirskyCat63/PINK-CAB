#include "Vehicle/PinkCabVehicleDamageProfile.h"

bool FPinkCabVehicleDamageZoneSpec::IsValid() const
{
    return !ZoneId.IsNone()
        && Channel != EPinkCabVehicleHealthChannel::Count
        && Channel != EPinkCabVehicleHealthChannel::CosmeticBody
        && FMath::IsFinite(FunctionalThreshold)
        && FunctionalThreshold >= 0.0f
        && FunctionalThreshold <= 1.0f;
}

FPinkCabVehicleDamageProfile::FPinkCabVehicleDamageProfile(FName InProfileId)
    : ProfileId(InProfileId)
{
}

FName FPinkCabVehicleDamageProfile::GetProfileId() const { return ProfileId; }

bool FPinkCabVehicleDamageProfile::TryAddZone(
    FName ZoneId,
    EPinkCabVehicleHealthChannel Channel,
    float FunctionalThreshold)
{
    FPinkCabVehicleDamageZoneSpec Spec{ZoneId, Channel, FunctionalThreshold};
    if (!Spec.IsValid()) return false;

    for (const FPinkCabVehicleDamageZoneSpec& Existing : Zones)
    {
        if (Existing.ZoneId == ZoneId) return false;
    }

    Zones.Add(Spec);
    return true;
}

bool FPinkCabVehicleDamageProfile::ResolveFunctionalHit(
    FName ZoneId,
    float CollisionSeverity,
    FPinkCabVehicleHitEvent& OutEvent) const
{
    if (!FMath::IsFinite(CollisionSeverity)) return false;

    for (const FPinkCabVehicleDamageZoneSpec& Spec : Zones)
    {
        if (Spec.ZoneId != ZoneId) continue;
        if (CollisionSeverity <= Spec.FunctionalThreshold) return false;
        OutEvent = FPinkCabVehicleHitEvent(
            Spec.Channel,
            FMath::Clamp(CollisionSeverity, 0.0f, 1.0f),
            false);
        return true;
    }
    return false;
}
