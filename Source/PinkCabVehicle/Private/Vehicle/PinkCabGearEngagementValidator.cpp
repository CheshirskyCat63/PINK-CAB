#include "Vehicle/PinkCabGearEngagementValidator.h"

float FPinkCabGearEngagementValidator::ComputeClutchCoupling(
    const FPinkCabGearboxControllerConfig& Config,
    const float ClutchPedal)
{
    const float Released = FMath::Clamp(1.0f - ClutchPedal, 0.0f, 1.0f);
    return FMath::Pow(Released, FMath::Max(Config.ClutchCurveExponent, 1.0f));
}

float FPinkCabGearEngagementValidator::ExpectedEngineRpmForGear(
    const FPinkCabGearboxControllerConfig& Config,
    const int32 Gear,
    const float SpeedKmh)
{
    if (Gear == 0) return Config.IdleRpm;
    float RpmPerKmh = Config.ReverseRpmPerKmh;
    if (Gear > 0 && Gear <= 5)
    {
        RpmPerKmh = Config.RpmPerKmh[Gear];
    }
    // For a non-neutral gear the mechanically coupled shaft speed follows
    // road speed. At standstill it is zero, not engine idle; treating idle as
    // a floor makes N->gear at 0 km/h look falsely rev-matched without clutch.
    return FMath::Abs(SpeedKmh) * RpmPerKmh;
}

FPinkCabGearEngagementDecision FPinkCabGearEngagementValidator::EvaluateRequest(
    const FPinkCabGearboxControllerConfig& Config,
    const int32 Gear,
    const FPinkCabGearEngagementContext& Context)
{
    FPinkCabGearEngagementDecision Decision;
    Decision.ExpectedCoupledRpm = ExpectedEngineRpmForGear(Config, Gear, Context.SpeedKmh);
    if (Gear == 0)
    {
        Decision.bAccepted = true;
        Decision.Result = EPinkCabGearEngagementResult::Neutral;
        return Decision;
    }
    if (Gear == -1 && FMath::Abs(Context.SpeedKmh) > Config.ReverseLockoutSpeedKmh)
    {
        Decision.Result = EPinkCabGearEngagementResult::ReverseLockout;
        return Decision;
    }
    if (Context.ClutchPedal >= Config.ClutchDisengagedThreshold)
    {
        Decision.bAccepted = true;
        Decision.Result = EPinkCabGearEngagementResult::ClutchDisengagedAccepted;
        return Decision;
    }

    const float RpmMismatch = FMath::Abs(Context.EngineRpm - Decision.ExpectedCoupledRpm);
    const float AppliedLoad = FMath::Max(FMath::Abs(Context.Throttle), FMath::Abs(Context.Brake));
    const float HealthPenalty = FMath::Lerp(
        0.65f, 1.0f, FMath::Clamp(Context.GearboxHealth, 0.0f, 1.0f));
    const float AllowedMismatch = Config.ClutchlessMatchToleranceRpm * HealthPenalty;
    if (AppliedLoad <= Config.ClutchlessLoadThreshold && RpmMismatch <= AllowedMismatch)
    {
        Decision.bAccepted = true;
        Decision.Result = EPinkCabGearEngagementResult::MatchedClutchlessAccepted;
        return Decision;
    }
    Decision.Result = EPinkCabGearEngagementResult::GrindRefused;
    return Decision;
}

bool FPinkCabGearEngagementValidator::CanRetryPending(
    const FPinkCabGearboxControllerConfig& Config,
    const int32 RequestedGear,
    const FPinkCabGearEngagementContext& Context)
{
    const float PendingRpm = ExpectedEngineRpmForGear(Config, RequestedGear, Context.SpeedKmh);
    const float RpmMismatch = FMath::Abs(Context.EngineRpm - PendingRpm);
    const float AppliedLoad = FMath::Max(FMath::Abs(Context.Throttle), FMath::Abs(Context.Brake));
    const float HealthPenalty = FMath::Lerp(
        0.65f, 1.0f, FMath::Clamp(Context.GearboxHealth, 0.0f, 1.0f));
    return Context.ClutchPedal >= Config.ClutchDisengagedThreshold
        || (AppliedLoad <= Config.ClutchlessLoadThreshold
            && RpmMismatch <= Config.ClutchlessMatchToleranceRpm * HealthPenalty);
}

bool FPinkCabGearEngagementValidator::IsDangerousOverrev(
    const FPinkCabGearboxControllerConfig& Config,
    const int32 EngagedGear,
    const FPinkCabGearEngagementContext& Context,
    const float ExpectedCoupledRpm)
{
    if (EngagedGear == 0) return false;
    const float Coupling = ComputeClutchCoupling(Config, Context.ClutchPedal);
    return Coupling > 0.75f && ExpectedCoupledRpm > Config.MaxSafeEngineRpm;
}
