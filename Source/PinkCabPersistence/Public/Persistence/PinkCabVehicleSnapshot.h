#pragma once

#include "CoreMinimal.h"
#include "Vehicle/PinkCabVehicleStateSnapshot.h"

using FPinkCabVehicleHealthSnapshot = FPinkCabVehicleHealthStateSnapshot;
using FPinkCabVehicleLoadItemSnapshot = FPinkCabVehicleLoadItemStateSnapshot;
using FPinkCabVehicleLoadSnapshot = FPinkCabVehicleLoadStateSnapshot;

struct FPinkCabVehicleSnapshot
{
    static constexpr int32 LegacySchemaVersion = 1;
    static constexpr int32 DivergentSchemaVersion = 2;
    static constexpr int32 CurrentSchemaVersion = 3;

    static constexpr int32 LegacySchema1HealthChannelCount = 12;
    static constexpr int32 RuntimeSchema2HealthChannelCount = 14;
    static constexpr int32 PresentationSchema2HealthChannelCount = 16;

    int32 SchemaVersion = CurrentSchemaVersion;
    FPinkCabVehicleHealthSnapshot Health;
    FPinkCabVehicleLoadSnapshot Load;
};

class PINKCABPERSISTENCE_API FPinkCabVehicleSnapshotCodec
{
public:
    static bool Capture(
        const FPinkCabVehicleHealthState& Health,
        const FPinkCabVehicleLoadState& Load,
        FPinkCabVehicleSnapshot& OutSnapshot);
    static bool Restore(
        const FPinkCabVehicleSnapshot& Snapshot,
        FPinkCabVehicleHealthState& OutHealth,
        FPinkCabVehicleLoadState& OutLoad);

private:
    enum class ESchemaLayout : uint8
    {
        Invalid,
        LegacyV1,
        RuntimeV2,
        PresentationV2,
        Current
    };

    static ESchemaLayout ResolveSchemaLayout(
        const FPinkCabVehicleSnapshot& Snapshot);
    static void InitializeHealth(
        FPinkCabVehicleHealthStateSnapshot& Health);
    static void MigrateLegacyV1(
        const FPinkCabVehicleSnapshot& Snapshot,
        FPinkCabVehicleHealthStateSnapshot& Health);
    static void MigrateRuntimeV2(
        const FPinkCabVehicleSnapshot& Snapshot,
        FPinkCabVehicleHealthStateSnapshot& Health);
    static void MigratePresentationV2(
        const FPinkCabVehicleSnapshot& Snapshot,
        FPinkCabVehicleHealthStateSnapshot& Health);
    static void MigrateCurrent(
        const FPinkCabVehicleSnapshot& Snapshot,
        FPinkCabVehicleHealthStateSnapshot& Health);
    static void MigrateHealth(
        const FPinkCabVehicleSnapshot& Snapshot,
        FPinkCabVehicleHealthStateSnapshot& Health);

    static bool ValidateItem(
        const FPinkCabVehicleLoadItemSnapshot& Item);
    static bool ValidateHealthChannels(
        const FPinkCabVehicleSnapshot& Snapshot);
    static bool ValidateThermalFields(
        const FPinkCabVehicleSnapshot& Snapshot,
        ESchemaLayout Layout);
    static bool ValidateLoadScalars(
        const FPinkCabVehicleSnapshot& Snapshot);
    static bool ValidateLoadItems(
        const FPinkCabVehicleSnapshot& Snapshot);
    static bool ValidateFarePassengerGroup(
        const FPinkCabVehicleSnapshot& Snapshot);
    static bool Validate(
        const FPinkCabVehicleSnapshot& Snapshot);
};
