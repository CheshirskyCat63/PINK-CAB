#pragma once

#include "CoreMinimal.h"
#include "Vehicle/PinkCabVehicleControlState.h"
#include "Interaction/PinkCabSemanticInputRouter.h"

struct FPinkCabVehicleInputFrame
{
    static FPinkCabVehicleInputFrame FromDigital(
        bool bInGazeHeld,
        bool bClutchHeld,
        bool bBrakeHeld,
        bool bThrottleHeld)
    {
        FPinkCabVehicleInputFrame Result;
        Result.bGazeHeld = bInGazeHeld;
        Result.Clutch = bClutchHeld ? 1.0f : 0.0f;
        Result.Brake = bBrakeHeld ? 1.0f : 0.0f;
        Result.Throttle = bThrottleHeld ? 1.0f : 0.0f;
        return Result;
    }

    template <typename TIsKeyDown>
    static FPinkCabVehicleInputFrame FromRouter(
        const FPinkCabSemanticInputRouter& Router,
        TIsKeyDown&& IsKeyDown)
    {
        const auto Down = [&Router, &IsKeyDown](EPinkCabSemanticAction Action)
        {
            const FKey Key = Router.GetKeyForAction(Action);
            return Key.IsValid() && IsKeyDown(Key);
        };
        return FromDigital(
            Down(EPinkCabSemanticAction::GazeHold),
            Down(EPinkCabSemanticAction::Clutch),
            Down(EPinkCabSemanticAction::Brake),
            Down(EPinkCabSemanticAction::Throttle));
    }

    FPinkCabVehicleControlState ToControlState(float Steering, float Handbrake) const
    {
        FPinkCabVehicleControlState Result;
        Result.SetSteering(Steering);
        Result.SetThrottle(Throttle);
        Result.SetBrake(Brake);
        Result.SetClutch(Clutch);
        Result.SetHandbrake(Handbrake);
        return Result;
    }

    bool bGazeHeld = false;
    float Throttle = 0.0f;
    float Brake = 0.0f;
    float Clutch = 0.0f;
};
