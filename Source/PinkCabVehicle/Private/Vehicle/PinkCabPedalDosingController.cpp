#include "Vehicle/PinkCabPedalDosingController.h"

FPinkCabPedalDosingController::FPinkCabPedalDosingController(
    const FPinkCabPedalDosingControllerConfig& InConfig)
    : Config(InConfig)
    , BrakeTarget(FMath::Clamp(InConfig.BrakeDefault, 0.0f, 1.0f))
{
}

EPinkCabPedalWheelRecipient FPinkCabPedalDosingController::ApplyWheelSteps(
    bool bClutchHeld,
    bool bBrakeHeld,
    bool bThrottleHeld,
    int32 SignedSteps,
    FPinkCabLaunchController& Launch,
    FPinkCabCockpitState& Cockpit)
{
    if (SignedSteps == 0)
    {
        return EPinkCabPedalWheelRecipient::None;
    }

    if (bThrottleHeld)
    {
        Launch.ApplyThrottleDoseSteps(SignedSteps);
        return EPinkCabPedalWheelRecipient::Throttle;
    }

    if (bBrakeHeld)
    {
        BrakeTarget = FMath::Clamp(
            BrakeTarget + static_cast<float>(SignedSteps) * Config.BrakeStep,
            0.0f,
            1.0f);
        return EPinkCabPedalWheelRecipient::Brake;
    }

    if (bClutchHeld)
    {
        Cockpit.AdjustClutchReleaseSpeed(SignedSteps);
        return EPinkCabPedalWheelRecipient::ClutchRelease;
    }

    return EPinkCabPedalWheelRecipient::None;
}

FPinkCabPedalTargets FPinkCabPedalDosingController::ResolveTargets(
    bool bBrakeHeld,
    bool bThrottleHeld,
    float ThrottleTarget) const
{
    FPinkCabPedalTargets Result;
    Result.Brake = bBrakeHeld ? BrakeTarget : 0.0f;
    Result.Throttle = bThrottleHeld ? FMath::Clamp(ThrottleTarget, 0.0f, 1.0f) : 0.0f;
    return Result;
}

float FPinkCabPedalDosingController::GetBrakeTarget() const
{
    return BrakeTarget;
}
