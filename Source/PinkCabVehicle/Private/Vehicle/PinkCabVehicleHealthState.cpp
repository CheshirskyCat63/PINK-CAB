#include "Vehicle/PinkCabVehicleHealthState.h"

FPinkCabVehicleHealthState::FPinkCabVehicleHealthState()
{
    for (float& Value : Health)
    {
        Value = 1.0f;
    }
}

float FPinkCabVehicleHealthState::GetHealth(EPinkCabVehicleHealthChannel Channel) const
{
    const int32 Index = static_cast<int32>(Channel);
    return (Index >= 0 && Index < static_cast<int32>(EPinkCabVehicleHealthChannel::Count))
        ? Health[Index]
        : 0.0f;
}

uint32 FPinkCabVehicleHealthState::GetFunctionalDamageSerial() const { return FunctionalDamageSerial; }
float FPinkCabVehicleHealthState::GetClutchTemperature01() const { return ClutchTemperature01; }
float FPinkCabVehicleHealthState::GetBrakeTemperature01() const { return BrakeTemperature01; }

void FPinkCabVehicleHealthState::SetClutchTemperature01(float Value)
{
    ClutchTemperature01 = FMath::Clamp(Value, 0.0f, 1.0f);
}

void FPinkCabVehicleHealthState::SetBrakeTemperature01(float Value)
{
    BrakeTemperature01 = FMath::Clamp(Value, 0.0f, 1.0f);
}

bool FPinkCabVehicleHealthState::ApplyFunctionalDamage(
    EPinkCabVehicleHealthChannel Channel,
    float Severity)
{
    const int32 Index = static_cast<int32>(Channel);
    if (Index < 0 || Index >= static_cast<int32>(EPinkCabVehicleHealthChannel::CosmeticBody))
    {
        return false;
    }

    Health[Index] = FMath::Clamp(
        Health[Index] - FMath::Clamp(Severity, 0.0f, 1.0f),
        0.0f,
        1.0f);
    ++FunctionalDamageSerial;
    return true;
}

bool FPinkCabVehicleHealthState::RestoreFunctionalHealthTo(
    EPinkCabVehicleHealthChannel Channel,
    float TargetHealth01)
{
    const int32 Index = static_cast<int32>(Channel);
    if (Index < 0 || Index >= static_cast<int32>(EPinkCabVehicleHealthChannel::CosmeticBody))
    {
        return false;
    }

    const float Target = FMath::Clamp(TargetHealth01, 0.0f, 1.0f);
    if (Target <= Health[Index] + KINDA_SMALL_NUMBER)
    {
        return false;
    }

    Health[Index] = Target;
    ++FunctionalDamageSerial;
    return true;
}
