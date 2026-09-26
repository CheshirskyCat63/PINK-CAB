#include "Vehicle/PinkCabEngineActuationResolver.h"

#include "Vehicle/PinkCabThrottleResponse.h"

namespace
{
float ApplyPinkCabRevLimiter(
    const float RequestedEngineThrottle,
    const float EngineRpm,
    const float MaxRpm)
{
    if (MaxRpm <= KINDA_SMALL_NUMBER)
    {
        return RequestedEngineThrottle;
    }

    // Preserve the existing PINK CAB control-boundary limiter exactly.
    constexpr float SoftLimiterStartRatio = 0.965f;
    constexpr float HardLimiterCutRatio = 0.995f;
    const float SoftStartRpm = MaxRpm * SoftLimiterStartRatio;
    const float HardCutRpm = MaxRpm * HardLimiterCutRatio;
    if (EngineRpm <= SoftStartRpm)
    {
        return RequestedEngineThrottle;
    }
    if (EngineRpm >= HardCutRpm)
    {
        return 0.0f;
    }

    const float Remaining = 1.0f - FMath::Clamp(
        (EngineRpm - SoftStartRpm) / (HardCutRpm - SoftStartRpm),
        0.0f,
        1.0f);
    return RequestedEngineThrottle * Remaining;
}
}

FPinkCabEngineActuationResult FPinkCabEngineActuationResolver::Resolve(
    const FPinkCabEngineActuationInput& Input)
{
    FPinkCabEngineActuationResult Result;
    Result.EngineThrottlePreLimiter01 =
        FPinkCabThrottleResponse::ToEngineThrottle(
            Input.HealthClampedControlThrottle01);
    Result.EngineThrottleFinal01 = ApplyPinkCabRevLimiter(
        Result.EngineThrottlePreLimiter01,
        Input.EngineRpm,
        Input.MaxRpm);
    Result.RequestedEngineTorqueAfterLimiterHealthNm =
        FMath::Max(Input.EngineTorqueCurveNm, 0.0f)
        * FMath::Clamp(Result.EngineThrottleFinal01, 0.0f, 1.0f);
    return Result;
}
