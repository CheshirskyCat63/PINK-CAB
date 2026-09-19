#pragma once

#include "CoreMinimal.h"
#include "Core/PinkCabStableId.h"
#include "Vehicle/PinkCabVehicleHealthState.h"
#include "Vehicle/PinkCabVehicleLoadState.h"

struct FPinkCabVehicleHealthStateSnapshot
{
    TArray<float> ChannelHealth;
    uint32 FunctionalDamageSerial = 0;
    float ClutchTemperature01 = 0.0f;
    float BrakeTemperature01 = 0.0f;
};

struct FPinkCabVehicleLoadItemStateSnapshot
{
    float MassKg = 0.0f;
    float LongitudinalCm = 0.0f;
};

struct FPinkCabVehicleLoadStateSnapshot
{
    float FuelMassKg = 0.0f;
    float FuelLongitudinalCm = 0.0f;
    float HeroineMassKg = 0.0f;
    float DaughterMassKg = 0.0f;
    TArray<FPinkCabVehicleLoadItemStateSnapshot> Passengers;
    TArray<FPinkCabVehicleLoadItemStateSnapshot> FarePassengers;
    FString FarePassengerGroupId;
    bool bFarePassengerGroupActive = false;
};

struct FPinkCabVehicleStateSnapshot
{
    FPinkCabVehicleHealthStateSnapshot Health;
    FPinkCabVehicleLoadStateSnapshot Load;
};

class PINKCABVEHICLE_API FPinkCabVehicleStateSnapshotCodec
{
public:
    static bool Capture(
        const FPinkCabVehicleHealthState& Health,
        const FPinkCabVehicleLoadState& Load,
        FPinkCabVehicleStateSnapshot& OutSnapshot);
    static bool Restore(
        const FPinkCabVehicleStateSnapshot& Snapshot,
        FPinkCabVehicleHealthState& OutHealth,
        FPinkCabVehicleLoadState& OutLoad);
    static bool Validate(const FPinkCabVehicleStateSnapshot& Snapshot);

private:
    static bool ValidateHealth(const FPinkCabVehicleHealthStateSnapshot& Health);
    static bool ValidateLoad(const FPinkCabVehicleLoadStateSnapshot& Load);
    static bool ValidateLoadScalars(const FPinkCabVehicleLoadStateSnapshot& Load);
    static bool ValidateLoadItems(const FPinkCabVehicleLoadStateSnapshot& Load);
    static bool ValidateFareGroup(const FPinkCabVehicleLoadStateSnapshot& Load);
    static bool ValidateItem(const FPinkCabVehicleLoadItemStateSnapshot& Item);
};
