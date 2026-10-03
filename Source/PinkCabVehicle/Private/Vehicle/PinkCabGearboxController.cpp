#include "Vehicle/PinkCabGearboxController.h"

#include "Vehicle/PinkCabGearEngagementValidator.h"
#include "Vehicle/PinkCabChaosPhysicalProfile.h"

FPinkCabGearboxControllerConfig::FPinkCabGearboxControllerConfig()
{
    const FPinkCabChaosPhysicalProfile Profile =
        FPinkCabChaosPhysicalProfile::ForVariant(EPinkCabCalibrationVariant::Nominal);
    check(Profile.ForwardGearRatios.Value.Num() == 5);
    check(Profile.ReverseGearRatios.Value.Num() == 1);
    check(Profile.RearWheel.WheelRadiusCm.Value > 0.0f);
    const double CircumferenceM =
        2.0 * PI * Profile.RearWheel.WheelRadiusCm.Value / 100.0;
    const double RpmFactor =
        1000.0 * Profile.FinalDriveRatio.Value / (60.0 * CircumferenceM);
    for (int32 Gear = 1; Gear <= 5; ++Gear)
    {
        RpmPerKmh[Gear] = static_cast<float>(
            RpmFactor * Profile.ForwardGearRatios.Value[Gear - 1]);
    }
    ReverseRpmPerKmh = static_cast<float>(
        RpmFactor * Profile.ReverseGearRatios.Value[0]);
    EngineRpmEnvelope = Profile.GetEngineRpmEnvelope();
}

FPinkCabGearboxController::FPinkCabGearboxController(
    const FPinkCabGearboxControllerConfig& InConfig)
    : Config(InConfig)
{
    ExpectedCoupledRpm = Config.EngineRpmEnvelope.IdleRpm;
}

float FPinkCabGearboxController::ComputeClutchCoupling(const float ClutchPedal) const
{
    return FPinkCabGearEngagementValidator::ComputeClutchCoupling(Config, ClutchPedal);
}

float FPinkCabGearboxController::ExpectedEngineRpmForGear(
    const int32 Gear,
    const float SpeedKmh) const
{
    return FPinkCabGearEngagementValidator::ExpectedEngineRpmForGear(Config, Gear, SpeedKmh);
}

bool FPinkCabGearboxController::MoveGate(const float X, const float Y)
{
    return FPinkCabHGateGeometry::MoveGate(GateState, X, Y);
}

bool FPinkCabGearboxController::ApplyLeverDriverDelta(
    const float DriverRightCounts,
    const float DriverForwardCounts)
{
    return FPinkCabHGateGeometry::ApplyDriverDelta(
        GateState, DriverRightCounts, DriverForwardCounts);
}

void FPinkCabGearboxController::CancelPendingRequest()
{
    FPinkCabHGateGeometry::ResetToGear(GateState, EngagedGear);
}

bool FPinkCabGearboxController::RequestGear(
    const int32 Gear,
    const FPinkCabGearEngagementContext& Context)
{
    if (Gear < -1 || Gear > 5)
    {
        return false;
    }

    GateState.RequestedGear = Gear;
    ++EventSerial;
    const FPinkCabGearEngagementDecision Decision =
        FPinkCabGearEngagementValidator::EvaluateRequest(Config, Gear, Context);
    ExpectedCoupledRpm = Decision.ExpectedCoupledRpm;
    LastResult = Decision.Result;
    if (Decision.bAccepted)
    {
        EngagedGear = Gear;
    }
    return Decision.bAccepted;
}

bool FPinkCabGearboxController::RequestGearByDelta(
    const int32 Delta,
    const FPinkCabGearEngagementContext& Context)
{
    if (Delta == 0)
    {
        return false;
    }
    const int32 Candidate = FMath::Clamp(GateState.RequestedGear + Delta, -1, 5);
    return RequestGear(Candidate, Context);
}

EPinkCabGearEngagementResult FPinkCabGearboxController::EvaluateCurrentEngagement(
    const FPinkCabGearEngagementContext& Context)
{
    if (GateState.RequestedGear != EngagedGear)
    {
        if (FPinkCabGearEngagementValidator::CanRetryPending(
                Config, GateState.RequestedGear, Context))
        {
            RequestGear(GateState.RequestedGear, Context);
        }
        return LastResult;
    }

    if (EngagedGear == 0)
    {
        ExpectedCoupledRpm = Config.EngineRpmEnvelope.IdleRpm;
        LastResult = EPinkCabGearEngagementResult::Neutral;
        return LastResult;
    }

    ExpectedCoupledRpm = ExpectedEngineRpmForGear(EngagedGear, Context.SpeedKmh);
    if (FPinkCabGearEngagementValidator::IsDangerousOverrev(
            Config, EngagedGear, Context, ExpectedCoupledRpm))
    {
        if (LastResult != EPinkCabGearEngagementResult::DangerousOverrev)
        {
            ++EventSerial;
        }
        LastResult = EPinkCabGearEngagementResult::DangerousOverrev;
    }
    return LastResult;
}

void FPinkCabGearboxController::ForceState(
    const int32 InRequestedGear,
    const int32 InEngagedGear)
{
    EngagedGear = FMath::Clamp(InEngagedGear, -1, 5);
    FPinkCabHGateGeometry::ResetToGear(GateState, InRequestedGear);
}
