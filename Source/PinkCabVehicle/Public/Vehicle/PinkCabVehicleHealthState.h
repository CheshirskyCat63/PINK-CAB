#pragma once

#include "CoreMinimal.h"

enum class EPinkCabVehicleHealthChannel : uint8
{
    Wheel,
    Tire,
    Alignment,
    Suspension,
    Brake,
    BrakeHeat,
    BrakeHydraulic,
    Clutch,
    Gearbox,
    Door,
    Lamp,
    Glass,
    EngineOil,
    EngineHead,
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
    float GetClutchTemperature01() const { return ClutchTemperature01; }
    float GetBrakeTemperature01() const { return BrakeTemperature01; }

    void SetClutchTemperature01(float Value)
    {
        ClutchTemperature01 = FMath::Clamp(Value, 0.0f, 1.0f);
    }

    void SetBrakeTemperature01(float Value)
    {
        BrakeTemperature01 = FMath::Clamp(Value, 0.0f, 1.0f);
    }

    bool ApplyFunctionalDamage(EPinkCabVehicleHealthChannel Channel, float Severity)
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

    bool RestoreFunctionalHealthTo(
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

private:
    friend class FPinkCabVehicleSnapshotCodec;
    friend class FPinkCabVehicleStateSnapshotCodec;

    float Health[static_cast<int32>(EPinkCabVehicleHealthChannel::Count)] = {};
    uint32 FunctionalDamageSerial = 0;
    float ClutchTemperature01 = 0.0f;
    float BrakeTemperature01 = 0.0f;
};
