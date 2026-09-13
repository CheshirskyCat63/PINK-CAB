#pragma once

#include "CoreMinimal.h"
#include "Service/PinkCabFuelTank.h"
#include "Economy/PinkCabEconomyLedger.h"
#include "Vehicle/PinkCabVehicleTelemetry.h"
#include "World/PinkCabRoadGraph.h"

class FPinkCabServiceSnapshotCodec;

enum class EPinkCabMovingFuelState : uint8
{
    Disconnected,
    Connected,
    Fueling,
    Completed,
    Aborted
};

enum class EPinkCabMovingFuelAbortReason : uint8
{
    None,
    ToleranceLost,
    Collision,
    Explicit,
    InvalidLane
};

struct FPinkCabMovingFuelPolicyInputs
{
    FName QueuePolicyId;
    FName SettlementPolicyId;
    FName InsufficientFundsPolicyId;    float TargetLongitudinalGapCm = 0.0f;
    float GapToleranceCm = 0.0f;
    float MaxConnectionSpeedKmh = 0.0f;

    bool IsSpecified() const
    {
        return !QueuePolicyId.IsNone() && !SettlementPolicyId.IsNone()
            && !InsufficientFundsPolicyId.IsNone() && GapToleranceCm > 0.0f
            && MaxConnectionSpeedKmh > 0.0f;
    }
};

class FPinkCabMovingFuelSession
{
public:
    explicit FPinkCabMovingFuelSession(const FPinkCabMovingFuelPolicyInputs& InPolicy)
        : Policy(InPolicy)
    {
    }

    EPinkCabMovingFuelState GetState() const { return State; }
    EPinkCabMovingFuelAbortReason GetAbortReason() const { return AbortReason; }
    const FPinkCabMovingFuelPolicyInputs& GetPolicy() const { return Policy; }
    const FPinkCabLaneId& GetServiceLaneId() const { return ServiceLaneId; }

    bool CanConnect(
        const FPinkCabVehicleTelemetry& Telemetry,
        float LongitudinalGapCm,
        const FPinkCabLaneId& InServiceLaneId) const    {
        if (!Policy.IsSpecified() || !InServiceLaneId.IsValid()) return false;
        if (FMath::Abs(Telemetry.SpeedKmh) > Policy.MaxConnectionSpeedKmh) return false;
        return FMath::Abs(LongitudinalGapCm - Policy.TargetLongitudinalGapCm)
            <= Policy.GapToleranceCm;
    }

    bool TryConnect(
        const FPinkCabRoadGraph& Graph,
        const FPinkCabVehicleTelemetry& Telemetry,
        float LongitudinalGapCm,
        const FPinkCabLaneId& InServiceLaneId)
    {
        if (State != EPinkCabMovingFuelState::Disconnected) return false;
        if (!Graph.FindLane(InServiceLaneId)) return false;
        if (!CanConnect(Telemetry, LongitudinalGapCm, InServiceLaneId)) return false;
        ServiceLaneId = InServiceLaneId;
        AbortReason = EPinkCabMovingFuelAbortReason::None;
        State = EPinkCabMovingFuelState::Connected;
        return true;
    }

    bool MaintainConnection(
        const FPinkCabRoadGraph& Graph,
        const FPinkCabVehicleTelemetry& Telemetry,
        float LongitudinalGapCm)
    {
        if (State != EPinkCabMovingFuelState::Connected
            && State != EPinkCabMovingFuelState::Fueling) return false;        if (!ServiceLaneId.IsValid() || !Graph.FindLane(ServiceLaneId))
        {
            AbortWithReason(EPinkCabMovingFuelAbortReason::InvalidLane);
            return false;
        }
        if (!CanConnect(Telemetry, LongitudinalGapCm, ServiceLaneId))
        {
            AbortWithReason(EPinkCabMovingFuelAbortReason::ToleranceLost);
            return false;
        }
        return true;
    }

    bool MarkConnected()
    {
        if (State != EPinkCabMovingFuelState::Disconnected || !Policy.IsSpecified()) return false;
        AbortReason = EPinkCabMovingFuelAbortReason::None;
        State = EPinkCabMovingFuelState::Connected;
        return true;
    }

    bool BeginFueling()
    {
        if (State != EPinkCabMovingFuelState::Connected) return false;
        State = EPinkCabMovingFuelState::Fueling;
        return true;
    }

    bool Complete()
    {
        if (State != EPinkCabMovingFuelState::Fueling) return false;
        State = EPinkCabMovingFuelState::Completed;
        return true;
    }
    bool Abort()
    {
        return AbortWithReason(EPinkCabMovingFuelAbortReason::Explicit);
    }

    bool NotifyCollision()
    {
        return AbortWithReason(EPinkCabMovingFuelAbortReason::Collision);
    }

    EPinkCabFuelCreditResult ApplySettledFuel(
        FPinkCabFuelTank& Tank,
        const FPinkCabTransactionId& TransactionId,
        float Liters,
        EPinkCabSettlementResult SettlementResult)
    {
        if (State != EPinkCabMovingFuelState::Fueling)
            return EPinkCabFuelCreditResult::InvalidState;
        if (SettlementResult != EPinkCabSettlementResult::Committed
            && SettlementResult != EPinkCabSettlementResult::Duplicate)
            return EPinkCabFuelCreditResult::SettlementNotCommitted;
        return Tank.CreditFuelOnce(TransactionId, Liters);
    }

private:
    friend class FPinkCabServiceSnapshotCodec;

    bool AbortWithReason(EPinkCabMovingFuelAbortReason Reason)
    {
        if (State != EPinkCabMovingFuelState::Connected
            && State != EPinkCabMovingFuelState::Fueling) return false;
        AbortReason = Reason;
        State = EPinkCabMovingFuelState::Aborted;
        return true;
    }

    FPinkCabMovingFuelPolicyInputs Policy;    EPinkCabMovingFuelState State = EPinkCabMovingFuelState::Disconnected;
    EPinkCabMovingFuelAbortReason AbortReason = EPinkCabMovingFuelAbortReason::None;
    FPinkCabLaneId ServiceLaneId;
};
