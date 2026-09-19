#pragma once

#include "CoreMinimal.h"
#include "Taxi/PinkCabPassengerTemplate.h"
#include "Vehicle/PinkCabVehicleLoadState.h"

struct FPinkCabFarePassengerInput
{
    FPinkCabStableId PassengerId;
    float ResolvedMassKg = 0.0f;
};

struct PINKCABTAXI_API FPinkCabPassengerLoadLayout
{
    float RearLongitudinalCm = 0.0f;
    float FrontLongitudinalCm = 0.0f;

    float ResolveLongitudinalCm(EPinkCabPassengerSeat Seat) const;
};

struct FPinkCabFarePassengerRecord
{
    FPinkCabStableId PassengerId;
    EPinkCabPassengerSeat Seat = EPinkCabPassengerSeat::Rear1;
    float MassKg = 0.0f;
    float LongitudinalCm = 0.0f;
};

class PINKCABTAXI_API FPinkCabFarePassengerManifest
{
public:
    static bool TryCreate(
        const FPinkCabStableId& FareId,
        TConstArrayView<FPinkCabFarePassengerInput> Inputs,
        FPinkCabFarePassengerManifest& OutManifest,
        const FPinkCabPassengerLoadLayout& Layout = {});

    TConstArrayView<FPinkCabFarePassengerRecord> GetRecords() const;
    float GetTotalPassengerMassKg() const;
    bool IsBoarded() const;
    bool HasExited() const;

    bool TryBoard(
        bool bFullStop,
        bool bDoorOpen,
        FPinkCabVehicleLoadState& LoadState);
    bool TryExit(
        bool bFullStop,
        bool bDoorOpen,
        FPinkCabVehicleLoadState& LoadState);
    bool TryTerminalRecoveryExit(FPinkCabVehicleLoadState& LoadState);

private:
    friend class FPinkCabFareRuntimeSnapshotCodec;

    FPinkCabStableId FareId;
    TArray<FPinkCabFarePassengerRecord, TInlineAllocator<5>> Records;
    float TotalPassengerMassKg = 0.0f;
    bool bBoarded = false;
    bool bExited = false;
};
