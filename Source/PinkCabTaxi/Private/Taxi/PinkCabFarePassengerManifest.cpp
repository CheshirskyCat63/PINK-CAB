#include "Taxi/PinkCabFarePassengerManifest.h"

float FPinkCabPassengerLoadLayout::ResolveLongitudinalCm(
    EPinkCabPassengerSeat Seat) const
{
    return Seat == EPinkCabPassengerSeat::Front1
        || Seat == EPinkCabPassengerSeat::Front2
        ? FrontLongitudinalCm
        : RearLongitudinalCm;
}

bool FPinkCabFarePassengerManifest::TryCreate(
    const FPinkCabStableId& FareId,
    TConstArrayView<FPinkCabFarePassengerInput> Inputs,
    FPinkCabFarePassengerManifest& OutManifest,
    const FPinkCabPassengerLoadLayout& Layout)
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

TConstArrayView<FPinkCabFarePassengerRecord>
FPinkCabFarePassengerManifest::GetRecords() const
{
    return TConstArrayView<FPinkCabFarePassengerRecord>(
        Records.GetData(),
        Records.Num());
}

float FPinkCabFarePassengerManifest::GetTotalPassengerMassKg() const
{
    return TotalPassengerMassKg;
}

bool FPinkCabFarePassengerManifest::IsBoarded() const
{
    return bBoarded;
}

bool FPinkCabFarePassengerManifest::HasExited() const
{
    return bExited;
}

bool FPinkCabFarePassengerManifest::TryBoard(
    bool bFullStop,
    bool bDoorOpen,
    FPinkCabVehicleLoadState& LoadState)
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

bool FPinkCabFarePassengerManifest::TryExit(
    bool bFullStop,
    bool bDoorOpen,
    FPinkCabVehicleLoadState& LoadState)
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

bool FPinkCabFarePassengerManifest::TryTerminalRecoveryExit(
    FPinkCabVehicleLoadState& LoadState)
{
    if (!bBoarded
        || bExited
        || !FareId.IsValid()
        || !LoadState.HasFarePassengerGroup(FareId))
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
