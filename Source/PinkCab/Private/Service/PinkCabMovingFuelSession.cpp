#include "Service/PinkCabMovingFuelSession.h"

bool FPinkCabMovingFuelPolicyInputs::IsSpecified() const
{
    return !QueuePolicyId.IsNone()
        && !SettlementPolicyId.IsNone()
        && !InsufficientFundsPolicyId.IsNone()
        && GapToleranceCm > 0.0f
        && MaxConnectionSpeedKmh > 0.0f;
}

FPinkCabMovingFuelSession::FPinkCabMovingFuelSession(
    const FPinkCabMovingFuelPolicyInputs& InPolicy)
    : Policy(InPolicy)
{
}

EPinkCabMovingFuelState FPinkCabMovingFuelSession::GetState() const
{
    return State;
}

EPinkCabMovingFuelAbortReason
FPinkCabMovingFuelSession::GetAbortReason() const
{
    return AbortReason;
}

const FPinkCabMovingFuelPolicyInputs&
FPinkCabMovingFuelSession::GetPolicy() const
{
    return Policy;
}

const FPinkCabLaneId&
FPinkCabMovingFuelSession::GetServiceLaneId() const
{
    return ServiceLaneId;
}

bool FPinkCabMovingFuelSession::CanConnect(
    const FPinkCabVehicleTelemetry& Telemetry,
    float LongitudinalGapCm,
    const FPinkCabLaneId& InServiceLaneId) const
{
    if (!Policy.IsSpecified() || !InServiceLaneId.IsValid())
    {
        return false;
    }
    if (FMath::Abs(Telemetry.SpeedKmh)
        > Policy.MaxConnectionSpeedKmh)
    {
        return false;
    }
    return FMath::Abs(
        LongitudinalGapCm - Policy.TargetLongitudinalGapCm)
        <= Policy.GapToleranceCm;
}

bool FPinkCabMovingFuelSession::TryConnect(
    const FPinkCabRoadGraph& Graph,
    const FPinkCabVehicleTelemetry& Telemetry,
    float LongitudinalGapCm,
    const FPinkCabLaneId& InServiceLaneId)
{
    if (State != EPinkCabMovingFuelState::Disconnected)
    {
        return false;
    }
    if (!Graph.FindLane(InServiceLaneId))
    {
        return false;
    }
    if (!CanConnect(
            Telemetry,
            LongitudinalGapCm,
            InServiceLaneId))
    {
        return false;
    }

    ServiceLaneId = InServiceLaneId;
    AbortReason = EPinkCabMovingFuelAbortReason::None;
    State = EPinkCabMovingFuelState::Connected;
    return true;
}

bool FPinkCabMovingFuelSession::MaintainConnection(
    const FPinkCabRoadGraph& Graph,
    const FPinkCabVehicleTelemetry& Telemetry,
    float LongitudinalGapCm)
{
    if (State != EPinkCabMovingFuelState::Connected
        && State != EPinkCabMovingFuelState::Fueling)
    {
        return false;
    }
    if (!ServiceLaneId.IsValid()
        || !Graph.FindLane(ServiceLaneId))
    {
        AbortWithReason(
            EPinkCabMovingFuelAbortReason::InvalidLane);
        return false;
    }
    if (!CanConnect(
            Telemetry,
            LongitudinalGapCm,
            ServiceLaneId))
    {
        AbortWithReason(
            EPinkCabMovingFuelAbortReason::ToleranceLost);
        return false;
    }
    return true;
}

bool FPinkCabMovingFuelSession::MarkConnected()
{
    if (State != EPinkCabMovingFuelState::Disconnected
        || !Policy.IsSpecified())
    {
        return false;
    }

    AbortReason = EPinkCabMovingFuelAbortReason::None;
    State = EPinkCabMovingFuelState::Connected;
    return true;
}

bool FPinkCabMovingFuelSession::BeginFueling()
{
    if (State != EPinkCabMovingFuelState::Connected)
    {
        return false;
    }
    State = EPinkCabMovingFuelState::Fueling;
    return true;
}

bool FPinkCabMovingFuelSession::Complete()
{
    if (State != EPinkCabMovingFuelState::Fueling)
    {
        return false;
    }
    State = EPinkCabMovingFuelState::Completed;
    return true;
}

bool FPinkCabMovingFuelSession::Abort()
{
    return AbortWithReason(
        EPinkCabMovingFuelAbortReason::Explicit);
}

bool FPinkCabMovingFuelSession::NotifyCollision()
{
    return AbortWithReason(
        EPinkCabMovingFuelAbortReason::Collision);
}

EPinkCabFuelCreditResult
FPinkCabMovingFuelSession::ApplySettledFuel(
    FPinkCabFuelTank& Tank,
    const FPinkCabTransactionId& TransactionId,
    float Liters,
    EPinkCabSettlementResult SettlementResult)
{
    if (State != EPinkCabMovingFuelState::Fueling)
    {
        return EPinkCabFuelCreditResult::InvalidState;
    }
    if (SettlementResult != EPinkCabSettlementResult::Committed
        && SettlementResult != EPinkCabSettlementResult::Duplicate)
    {
        return EPinkCabFuelCreditResult::SettlementNotCommitted;
    }
    return Tank.CreditFuelOnce(TransactionId, Liters);
}

bool FPinkCabMovingFuelSession::AbortWithReason(
    EPinkCabMovingFuelAbortReason Reason)
{
    if (State != EPinkCabMovingFuelState::Connected
        && State != EPinkCabMovingFuelState::Fueling)
    {
        return false;
    }

    AbortReason = Reason;
    State = EPinkCabMovingFuelState::Aborted;
    return true;
}
