#include "Vehicle/PinkCabLaunchController.h"

bool FPinkCabLaunchController::BeginLaunchAttempt()
{
    if (MotionMode != EPinkCabVehicleMotionMode::Stationary || bLaunchActive)
    {
        return false;
    }

    bLaunchActive = true;
    bThrottleDoseRequired = false;
    ThrottleTarget = InitialThrottleTarget;
    ++LaunchSerial;
    return true;
}

bool FPinkCabLaunchController::ApplyThrottleDoseSteps(int32 SignedSteps)
{
    if (SignedSteps == 0)
    {
        return false;
    }

    ThrottleTarget = FMath::Clamp(
        ThrottleTarget + static_cast<float>(SignedSteps) * ThrottleDoseStep,
        0.0f,
        1.0f);
    bThrottleDoseRequired = false;
    return true;
}

void FPinkCabLaunchController::NotifyMotionMode(EPinkCabVehicleMotionMode InMode)
{
    MotionMode = InMode;
    if (MotionMode == EPinkCabVehicleMotionMode::Moving)
    {
        bLaunchActive = false;
    }
}

EPinkCabVehicleMotionMode FPinkCabLaunchController::GetMotionMode() const
{
    return MotionMode;
}

uint32 FPinkCabLaunchController::GetLaunchSerial() const
{
    return LaunchSerial;
}

bool FPinkCabLaunchController::IsLaunchActive() const
{
    return bLaunchActive;
}

bool FPinkCabLaunchController::RequiresThrottleDose() const
{
    return bThrottleDoseRequired;
}

float FPinkCabLaunchController::GetThrottleTarget() const
{
    return ThrottleTarget;
}
