#pragma once

#include "CoreMinimal.h"

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

struct FPinkCabGearboxControllerConfig
{
    float ClutchDisengagedThreshold = 0.85f;
    float ClutchCurveExponent = 1.35f;
    float ClutchlessMatchToleranceRpm = 250.0f;
    float ClutchlessLoadThreshold = 0.15f;
    float ReverseLockoutSpeedKmh = 5.0f;
    float MaxSafeEngineRpm = 6500.0f;
    float IdleRpm = 750.0f;
    float RpmPerKmh[6] = {
        0.0f, 121.5251f, 58.1207f, 39.6277f, 29.0603f, 22.4557f};
    float ReverseRpmPerKmh = 121.5251f;
};
