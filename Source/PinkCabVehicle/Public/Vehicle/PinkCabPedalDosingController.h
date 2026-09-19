#pragma once

#include "CoreMinimal.h"
#include "Vehicle/PinkCabLaunchController.h"
#include "Vehicle/PinkCabCockpitState.h"

enum class EPinkCabPedalWheelRecipient : uint8
{
    None,
    Throttle,
    Brake,
    ClutchRelease
};

struct FPinkCabPedalTargets
{
    float Brake = 0.0f;
    float Throttle = 0.0f;
};

struct FPinkCabPedalDosingControllerConfig
{
    float BrakeDefault = 0.45f;
    float BrakeStep = 0.05f;
};

class FPinkCabPedalDosingController
{
public:
    explicit FPinkCabPedalDosingController(
        const FPinkCabPedalDosingControllerConfig& InConfig = {})
        : Config(InConfig)
        , BrakeTarget(FMath::Clamp(InConfig.BrakeDefault, 0.0f, 1.0f))
    {
    }

    EPinkCabPedalWheelRecipient ApplyWheelSteps(
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

    FPinkCabPedalTargets ResolveTargets(
        bool bBrakeHeld,
        bool bThrottleHeld,
        float ThrottleTarget) const
    {
        FPinkCabPedalTargets Result;
        Result.Brake = bBrakeHeld ? BrakeTarget : 0.0f;
        Result.Throttle = bThrottleHeld ? FMath::Clamp(ThrottleTarget, 0.0f, 1.0f) : 0.0f;
        return Result;
    }
    float GetBrakeTarget() const { return BrakeTarget; }

private:
    FPinkCabPedalDosingControllerConfig Config;
    float BrakeTarget = 0.45f;
};
