#include "Vehicle/PinkCabVehicleLoadState.h"
#include "Vehicle/PinkCabVehicleMassProperties.h"

FPinkCabVehicleLoadItem::FPinkCabVehicleLoadItem() = default;

FPinkCabVehicleLoadItem::FPinkCabVehicleLoadItem(float InMassKg, float InLongitudinalCm)
    : MassKg(FMath::Max(0.0f, InMassKg))
    , LongitudinalCm(InLongitudinalCm)
{
}

FPinkCabVehicleLoadItem::FPinkCabVehicleLoadItem(float InMassKg, const FVector& InCenterCm)
    : MassKg(FMath::Max(0.0f, InMassKg))
    , LongitudinalCm(InCenterCm.X), LateralCm(InCenterCm.Y), VerticalCm(InCenterCm.Z)
{
}

void FPinkCabVehicleLoadState::SetFuelMassKg(float InMassKg, float InLongitudinalCm)
{
    FuelMassKg = FMath::Max(0.0f, InMassKg);
    FuelLongitudinalCm = InLongitudinalCm;
    FuelLateralCm = 0.0f;
    FuelVerticalCm = FPinkCabTatraProfile::DefaultFuelHeightCm;
}

void FPinkCabVehicleLoadState::SetFuelMassKg(float InMassKg, const FVector& InCenterCm)
{
    FuelMassKg = FMath::Max(0.0f, InMassKg);
    FuelLongitudinalCm = InCenterCm.X;
    FuelLateralCm = InCenterCm.Y;
    FuelVerticalCm = InCenterCm.Z;
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
    FPinkCabVehicleMassProperties Properties;
    return TryGetMassProperties(Profile, Properties) ? Properties.CenterCm.X : 0.0f;
}
