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

class PINKCABVEHICLE_API FPinkCabPedalDosingController
{
public:
    explicit FPinkCabPedalDosingController(const FPinkCabPedalDosingControllerConfig& InConfig = {});

    EPinkCabPedalWheelRecipient ApplyWheelSteps(
        bool bClutchHeld,
        bool bBrakeHeld,
        bool bThrottleHeld,
        int32 SignedSteps,
        FPinkCabLaunchController& Launch,
        FPinkCabCockpitState& Cockpit);

    FPinkCabPedalTargets ResolveTargets(
        bool bBrakeHeld,
        bool bThrottleHeld,
        float ThrottleTarget) const;
    float GetBrakeTarget() const;

private:
    FPinkCabPedalDosingControllerConfig Config;
    float BrakeTarget = 0.45f;
};
