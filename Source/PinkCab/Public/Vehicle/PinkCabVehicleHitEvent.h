#pragma once

#include "Vehicle/PinkCabVehicleHealthState.h"

struct FPinkCabVehicleHitEvent
{
    FPinkCabVehicleHitEvent() = default;

    FPinkCabVehicleHitEvent(
        EPinkCabVehicleHealthChannel InChannel,
        float InSeverity,
        bool bInCosmeticOnly)
        : Channel(InChannel)
        , Severity(InSeverity)
        , bCosmeticOnly(bInCosmeticOnly)
    {
    }

    EPinkCabVehicleHealthChannel Channel = EPinkCabVehicleHealthChannel::CosmeticBody;
    float Severity = 0.0f;
    bool bCosmeticOnly = false;
};
