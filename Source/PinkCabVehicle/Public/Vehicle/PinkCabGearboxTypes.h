#pragma once

#include "CoreMinimal.h"
#include "Vehicle/PinkCabEngineRpmEnvelope.h"

enum class EPinkCabGearEngagementResult : uint8
{
    None,
    Neutral,
    ClutchDisengagedAccepted,
    MatchedClutchlessAccepted,
    GrindRefused,
    ReverseLockout,
    DangerousOverrev
};

struct FPinkCabGearEngagementContext
{
    float ClutchPedal = 0.0f;
    float EngineRpm = 0.0f;
    float SpeedKmh = 0.0f;
    float Throttle = 0.0f;
    float Brake = 0.0f;
    float GearboxHealth = 1.0f;
};

struct PINKCABVEHICLE_API FPinkCabGearboxControllerConfig
{
    FPinkCabGearboxControllerConfig();

    float ClutchDisengagedThreshold = 0.85f;
    float ClutchCurveExponent = 1.35f;
    float ClutchlessMatchToleranceRpm = 250.0f;
    float ClutchlessLoadThreshold = 0.15f;
    float ReverseLockoutSpeedKmh = 5.0f;
    FPinkCabEngineRpmEnvelope EngineRpmEnvelope;
    // Derived once from the physical profile; never a second ratio authority.
    float RpmPerKmh[6] = {};
    float ReverseRpmPerKmh = 0.0f;
};
