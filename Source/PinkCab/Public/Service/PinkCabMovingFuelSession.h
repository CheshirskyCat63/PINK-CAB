#pragma once

#include "CoreMinimal.h"
#include "Service/PinkCabFuelTank.h"
#include "Economy/PinkCabEconomyLedger.h"
#include "Vehicle/PinkCabVehicleTelemetry.h"
#include "World/PinkCabRoadGraph.h"

enum class EPinkCabMovingFuelState : uint8
{
    Disconnected,
    Connected,
    Fueling,
    Completed,
    Aborted
};

struct FPinkCabMovingFuelPolicyInputs
{
    FName QueuePolicyId;
    FName SettlementPolicyId;
    FName InsufficientFundsPolicyId;
    float TargetLongitudinalGapCm = 0.0f;
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
    const FPinkCabMovingFuelPolicyInputs& GetPolicy() const { return Policy; }

    bool CanConnect(
        const FPinkCabVehicleTelemetry& Telemetry,
        float LongitudinalGapCm,
        const FPinkCabLaneId& ServiceLaneId) const
    {
        if (!Policy.IsSpecified() || !ServiceLaneId.IsValid())
        {
            return false;
        }
        if (FMath::Abs(Telemetry.SpeedKmh) > Policy.MaxConnectionSpeedKmh)
        {
            return false;
        }
        return FMath::Abs(LongitudinalGapCm - Policy.TargetLongitudinalGapCm)
            <= Policy.GapToleranceCm;
    }

    bool MarkConnected()
    {
        if (State != EPinkCabMovingFuelState::Disconnected || !Policy.IsSpecified())
        {
            return false;
        }
        State = EPinkCabMovingFuelState::Connected;
        return true;
    }

    bool BeginFueling()
    {
        if (State != EPinkCabMovingFuelState::Connected)
        {
            return false;
        }
        State = EPinkCabMovingFuelState::Fueling;
        return true;
    }

    EPinkCabFuelCreditResult ApplySettledFuel(
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

private:
    FPinkCabMovingFuelPolicyInputs Policy;
    EPinkCabMovingFuelState State = EPinkCabMovingFuelState::Disconnected;
};
