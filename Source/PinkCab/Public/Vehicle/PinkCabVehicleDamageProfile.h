#pragma once

#include "CoreMinimal.h"
#include "Vehicle/PinkCabVehicleHitEvent.h"

struct FPinkCabVehicleDamageZoneSpec
{
    FName ZoneId = NAME_None;
    EPinkCabVehicleHealthChannel Channel = EPinkCabVehicleHealthChannel::CosmeticBody;
    float FunctionalThreshold = 1.0f;

    bool IsValid() const
    {
        return !ZoneId.IsNone()
            && Channel != EPinkCabVehicleHealthChannel::Count
            && Channel != EPinkCabVehicleHealthChannel::CosmeticBody
            && FMath::IsFinite(FunctionalThreshold)
            && FunctionalThreshold >= 0.0f
            && FunctionalThreshold <= 1.0f;
    }
};

struct FPinkCabVehicleDamageProfile
{
    explicit FPinkCabVehicleDamageProfile(FName InProfileId = NAME_None)
        : ProfileId(InProfileId)
    {
    }

    FName GetProfileId() const { return ProfileId; }
    bool TryAddZone(FName ZoneId, EPinkCabVehicleHealthChannel Channel, float FunctionalThreshold)
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

    bool ResolveFunctionalHit(FName ZoneId, float CollisionSeverity, FPinkCabVehicleHitEvent& OutEvent) const
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

private:
    FName ProfileId = NAME_None;
    TArray<FPinkCabVehicleDamageZoneSpec> Zones;
};
