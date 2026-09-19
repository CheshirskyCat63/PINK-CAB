#include "Vehicle/PinkCabVehicleInputFrame.h"

FPinkCabVehicleInputFrame
FPinkCabVehicleInputFrame::FromDigital(
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

FPinkCabVehicleControlState
FPinkCabVehicleInputFrame::ToControlState(
    float Steering,
    float Handbrake) const
{
    FPinkCabVehicleControlState Result;
    Result.SetSteering(Steering);
    Result.SetThrottle(Throttle);
    Result.SetBrake(Brake);
    Result.SetClutch(Clutch);
    Result.SetHandbrake(Handbrake);
    return Result;
}
