#pragma once

#include "CoreMinimal.h"
#include "Vehicle/PinkCabVehicleControlState.h"
#include "Interaction/PinkCabSemanticInputRouter.h"

struct PINKCAB_API FPinkCabVehicleInputFrame
{
    static FPinkCabVehicleInputFrame FromDigital(
        bool bInGazeHeld,
        bool bClutchHeld,
        bool bBrakeHeld,
        bool bThrottleHeld);

    template <typename TIsKeyDown>
    static FPinkCabVehicleInputFrame FromRouter(
        const FPinkCabSemanticInputRouter& Router,
        TIsKeyDown&& IsKeyDown);

    FPinkCabVehicleControlState ToControlState(
        float Steering,
        float Handbrake) const;

    bool bGazeHeld = false;
    float Throttle = 0.0f;
    float Brake = 0.0f;
    float Clutch = 0.0f;
};

#include "Vehicle/Detail/PinkCabVehicleInputFrame.inl"
