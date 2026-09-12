#pragma once

#include "CoreMinimal.h"

enum class EPinkCabVehicleHealthChannel : uint8
{
    Wheel,
    Tire,
    Alignment,
    Suspension,
    Brake,
    Door,
    Lamp,
    EngineOil,
    EngineFan,
    OilCooler,
    Airflow,
    CosmeticBody,
    Count
};

struct FPinkCabVehicleHealthState
{
    FPinkCabVehicleHealthState()
    {
        for (float& Value : Health)
        {
            Value = 1.0f;
        }
    }

    float GetHealth(EPinkCabVehicleHealthChannel Channel) const
    {
        const int32 Index = static_cast<int32>(Channel);
        return (Index >= 0 && Index < static_cast<int32>(EPinkCabVehicleHealthChannel::Count))
            ? Health[Index]
            : 0.0f;
    }

    uint32 GetFunctionalDamageSerial() const { return FunctionalDamageSerial; }

    bool ApplyFunctionalDamage(EPinkCabVehicleHealthChannel Channel, float Severity)
    {
        const int32 Index = static_cast<int32>(Channel);
        if (Index < 0 || Index >= static_cast<int32>(EPinkCabVehicleHealthChannel::CosmeticBody))
        {
            return false;
        }
        Health[Index] = FMath::Clamp(Health[Index] - FMath::Clamp(Severity, 0.0f, 1.0f), 0.0f, 1.0f);
        ++FunctionalDamageSerial;
        return true;
    }

private:
    float Health[static_cast<int32>(EPinkCabVehicleHealthChannel::Count)] = {};
    uint32 FunctionalDamageSerial = 0;
};
