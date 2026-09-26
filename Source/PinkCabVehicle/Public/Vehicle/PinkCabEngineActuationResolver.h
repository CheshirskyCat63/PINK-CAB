#pragma once

#include "CoreMinimal.h"

struct PINKCABVEHICLE_API FPinkCabEngineActuationInput
{
    float HealthClampedControlThrottle01 = 0.0f;
    float EngineRpm = 0.0f;
    float MaxRpm = 0.0f;
    float EngineTorqueCurveNm = 0.0f;
};

struct PINKCABVEHICLE_API FPinkCabEngineActuationResult
{
    float EngineThrottlePreLimiter01 = 0.0f;
    float EngineThrottleFinal01 = 0.0f;
    float RequestedEngineTorqueAfterLimiterHealthNm = 0.0f;
};

struct PINKCABVEHICLE_API FPinkCabEngineActuationResolver
{
    static FPinkCabEngineActuationResult Resolve(
        const FPinkCabEngineActuationInput& Input);
};
