#include "Vehicle/PinkCabVehicleLoadState.h"

FPinkCabVehicleLoadItem::FPinkCabVehicleLoadItem() = default;

FPinkCabVehicleLoadItem::FPinkCabVehicleLoadItem(float InMassKg, float InLongitudinalCm)
    : MassKg(FMath::Max(0.0f, InMassKg))
    , LongitudinalCm(InLongitudinalCm)
{
}

void FPinkCabVehicleLoadState::SetFuelMassKg(float InMassKg, float InLongitudinalCm)
{
    FuelMassKg = FMath::Max(0.0f, InMassKg);
    FuelLongitudinalCm = InLongitudinalCm;
}

float FPinkCabVehicleLoadState::GetFuelMassKg() const { return FuelMassKg; }

void FPinkCabVehicleLoadState::SetCrew(float InHeroineMassKg, float InDaughterMassKg)
{
    HeroineMassKg = FMath::Max(0.0f, InHeroineMassKg);
    DaughterMassKg = FMath::Max(0.0f, InDaughterMassKg);
}

void FPinkCabVehicleLoadState::AddPassenger(const FPinkCabVehicleLoadItem& Item)
{
    Passengers.Add(Item);
}

bool FPinkCabVehicleLoadState::TrySetFarePassengerGroup(
    const FPinkCabStableId& GroupId,
    TConstArrayView<FPinkCabVehicleLoadItem> Items)
{
    if (bFarePassengerGroupActive || !GroupId.IsValid() || Items.IsEmpty() || Items.Num() > 5)
    {
        return false;
    }

    FarePassengerGroupId = GroupId;
    FarePassengers.Reset(Items.Num());
    FarePassengers.Append(Items.GetData(), Items.Num());
    bFarePassengerGroupActive = true;
    return true;
}

bool FPinkCabVehicleLoadState::RemoveFarePassengerGroup(const FPinkCabStableId& GroupId)
{
    if (!bFarePassengerGroupActive || GroupId != FarePassengerGroupId)
    {
        return false;
    }

    FarePassengers.Reset();
    FarePassengerGroupId = FPinkCabStableId();
    bFarePassengerGroupActive = false;
    return true;
}

bool FPinkCabVehicleLoadState::HasFarePassengerGroup(const FPinkCabStableId& GroupId) const
{
    return bFarePassengerGroupActive && GroupId == FarePassengerGroupId;
}

float FPinkCabVehicleLoadState::GetTotalMassKg(const FPinkCabTatraProfile& Profile) const
{
    float Total = Profile.BaseVehicleMassKg + FuelMassKg + HeroineMassKg + DaughterMassKg;
    for (const FPinkCabVehicleLoadItem& Item : Passengers) Total += Item.MassKg;
    for (const FPinkCabVehicleLoadItem& Item : FarePassengers) Total += Item.MassKg;
    return Total;
}

float FPinkCabVehicleLoadState::GetLongitudinalCgInputCm(const FPinkCabTatraProfile& Profile) const
{
    const float TotalMassKg = GetTotalMassKg(Profile);
    if (TotalMassKg <= KINDA_SMALL_NUMBER) return 0.0f;

    float WeightedCmKg = FuelMassKg * FuelLongitudinalCm;
    for (const FPinkCabVehicleLoadItem& Item : Passengers)
    {
        WeightedCmKg += Item.MassKg * Item.LongitudinalCm;
    }
    for (const FPinkCabVehicleLoadItem& Item : FarePassengers)
    {
        WeightedCmKg += Item.MassKg * Item.LongitudinalCm;
    }
    return WeightedCmKg / TotalMassKg;
}
