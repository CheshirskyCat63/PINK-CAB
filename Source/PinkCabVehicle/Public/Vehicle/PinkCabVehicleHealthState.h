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

struct PINKCABVEHICLE_API FPinkCabVehicleHealthState
{
    FPinkCabVehicleHealthState();

    float GetHealth(EPinkCabVehicleHealthChannel Channel) const;
    uint32 GetFunctionalDamageSerial() const;
    float GetClutchTemperature01() const;
    float GetBrakeTemperature01() const;
    void SetClutchTemperature01(float Value);
    void SetBrakeTemperature01(float Value);
    bool ApplyFunctionalDamage(EPinkCabVehicleHealthChannel Channel, float Severity);
    bool RestoreFunctionalHealthTo(EPinkCabVehicleHealthChannel Channel, float TargetHealth01);

private:
    friend class FPinkCabVehicleSnapshotCodec;
    friend class FPinkCabVehicleStateSnapshotCodec;

    float Health[static_cast<int32>(EPinkCabVehicleHealthChannel::Count)] = {};
    uint32 FunctionalDamageSerial = 0;
    float ClutchTemperature01 = 0.0f;
    float BrakeTemperature01 = 0.0f;
};
