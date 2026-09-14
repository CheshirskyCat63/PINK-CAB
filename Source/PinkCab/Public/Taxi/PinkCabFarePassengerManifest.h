#pragma once

#include "CoreMinimal.h"
#include "Taxi/PinkCabPassengerTemplate.h"
#include "Vehicle/PinkCabVehicleLoadState.h"

struct FPinkCabFarePassengerInput
{
    FPinkCabStableId PassengerId;
    float ResolvedMassKg = 0.0f;
};

struct FPinkCabPassengerLoadLayout
{
    float RearLongitudinalCm = 0.0f;
    float FrontLongitudinalCm = 0.0f;

    float ResolveLongitudinalCm(const EPinkCabPassengerSeat Seat) const
    {
        return Seat == EPinkCabPassengerSeat::Front1 || Seat == EPinkCabPassengerSeat::Front2
            ? FrontLongitudinalCm
            : RearLongitudinalCm;
    }
};

struct FPinkCabFarePassengerRecord
{
    FPinkCabStableId PassengerId;
    EPinkCabPassengerSeat Seat = EPinkCabPassengerSeat::Rear1;
    float MassKg = 0.0f;
    float LongitudinalCm = 0.0f;
};

class FPinkCabFarePassengerManifest
{
public:
    static bool TryCreate(
        const FPinkCabStableId& FareId,
        TConstArrayView<FPinkCabFarePassengerInput> Inputs,
        FPinkCabFarePassengerManifest& OutManifest,
        const FPinkCabPassengerLoadLayout& Layout = {})
    {
        if (!FareId.IsValid() || Inputs.Num() < 1 || Inputs.Num() > 5)
        {
            return false;
        }

        for (int32 Index = 0; Index < Inputs.Num(); ++Index)
        {
            if (!Inputs[Index].PassengerId.IsValid()
                || !FMath::IsFinite(Inputs[Index].ResolvedMassKg)
                || Inputs[Index].ResolvedMassKg <= 0.0f)
            {
                return false;
            }
            for (int32 Other = Index + 1; Other < Inputs.Num(); ++Other)
            {
                if (Inputs[Index].PassengerId == Inputs[Other].PassengerId)
                {
                    return false;
                }
            }
        }

        TArray<EPinkCabPassengerSeat> Seats;
        if (!FPinkCabPassengerGroup::TryAssignSeats(Inputs.Num(), Seats))
        {
            return false;
        }

        FPinkCabFarePassengerManifest Built;
        Built.FareId = FareId;
        Built.Records.Reserve(Inputs.Num());
        for (int32 Index = 0; Index < Inputs.Num(); ++Index)
        {
            FPinkCabFarePassengerRecord Record;
            Record.PassengerId = Inputs[Index].PassengerId;
            Record.Seat = Seats[Index];
            Record.MassKg = Inputs[Index].ResolvedMassKg;
            Record.LongitudinalCm = Layout.ResolveLongitudinalCm(Seats[Index]);
            Built.TotalPassengerMassKg += Record.MassKg;
            Built.Records.Add(Record);
        }

        OutManifest = MoveTemp(Built);
        return true;
    }

    TConstArrayView<FPinkCabFarePassengerRecord> GetRecords() const
    {
        return TConstArrayView<FPinkCabFarePassengerRecord>(Records.GetData(), Records.Num());
    }

    float GetTotalPassengerMassKg() const { return TotalPassengerMassKg; }
    bool IsBoarded() const { return bBoarded; }
    bool HasExited() const { return bExited; }

    bool TryBoard(const bool bFullStop, const bool bDoorOpen, FPinkCabVehicleLoadState& LoadState)
    {
        if (!bFullStop || !bDoorOpen || bBoarded || bExited || !FareId.IsValid())
        {
            return false;
        }

        TArray<FPinkCabVehicleLoadItem, TInlineAllocator<5>> Items;
        Items.Reserve(Records.Num());
        for (const FPinkCabFarePassengerRecord& Record : Records)
        {
            Items.Emplace(Record.MassKg, Record.LongitudinalCm);
        }
        if (!LoadState.TrySetFarePassengerGroup(FareId, Items))
        {
            return false;
        }

        bBoarded = true;
        return true;
    }

    bool TryExit(const bool bFullStop, const bool bDoorOpen, FPinkCabVehicleLoadState& LoadState)
    {
        if (!bFullStop || !bDoorOpen || !bBoarded || bExited)
        {
            return false;
        }
        if (!LoadState.RemoveFarePassengerGroup(FareId))
        {
            return false;
        }
        bBoarded = false;
        bExited = true;
        return true;
    }

private:
    friend class FPinkCabFareRuntimeSnapshotCodec;

    FPinkCabStableId FareId;
    TArray<FPinkCabFarePassengerRecord, TInlineAllocator<5>> Records;
    float TotalPassengerMassKg = 0.0f;
    bool bBoarded = false;
    bool bExited = false;
};
