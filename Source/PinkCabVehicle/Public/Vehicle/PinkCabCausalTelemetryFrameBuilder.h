#pragma once

#include "CoreMinimal.h"
#include "Vehicle/PinkCabCausalTelemetry.h"
#include "Vehicle/PinkCabVehicleControlState.h"
#include "Vehicle/PinkCabVehicleTelemetry.h"

struct PINKCABVEHICLE_API FPinkCabCausalTelemetryBuildInput
{
    double TimestampSeconds = 0.0;
    float DeltaSeconds = 0.0f;
    EPinkCabCausalIgnitionState IgnitionState = EPinkCabCausalIgnitionState::Off;

    FName ModelId;
    FName ProfileId;
    int32 ProfileSchemaVersion = 0;
    int32 CalibrationVersion = 0;
    uint64 ProfileHash = 0;

    FPinkCabCausalControlTelemetry Control;
    FPinkCabVehicleControlState Controls;
    FPinkCabVehicleTelemetry Vehicle;

    float EngineHealthFactor01 = 1.0f;
    float WorldSpeedMps = 0.0f;
    float VehicleMassKg = 0.0f;
};

struct PINKCABVEHICLE_API FPinkCabCausalTelemetryFrameBuilder
{
    static FPinkCabCausalTelemetryFrame Build(
        const FPinkCabCausalTelemetryBuildInput& Input);
};
