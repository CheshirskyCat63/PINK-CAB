#pragma once

#include "CoreMinimal.h"
#include "Vehicle/PinkCabVehicleHitEvent.h"

struct PINKCABVEHICLE_API FPinkCabVehicleDamageZoneSpec
{
    FName ZoneId = NAME_None;
    EPinkCabVehicleHealthChannel Channel = EPinkCabVehicleHealthChannel::CosmeticBody;
    float FunctionalThreshold = 1.0f;

    bool IsValid() const;
};

struct PINKCABVEHICLE_API FPinkCabVehicleDamageProfile
{
    explicit FPinkCabVehicleDamageProfile(FName InProfileId = NAME_None);

    FName GetProfileId() const;
    bool TryAddZone(FName ZoneId, EPinkCabVehicleHealthChannel Channel, float FunctionalThreshold);
    bool ResolveFunctionalHit(FName ZoneId, float CollisionSeverity, FPinkCabVehicleHitEvent& OutEvent) const;

private:
    FName ProfileId = NAME_None;
    TArray<FPinkCabVehicleDamageZoneSpec> Zones;
};
