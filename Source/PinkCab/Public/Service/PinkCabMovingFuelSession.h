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

struct PINKCAB_API FPinkCabMovingFuelPolicyInputs
{
    FName QueuePolicyId;
    FName SettlementPolicyId;
    FName InsufficientFundsPolicyId;
    float TargetLongitudinalGapCm = 0.0f;
    float GapToleranceCm = 0.0f;
    float MaxConnectionSpeedKmh = 0.0f;

    bool IsSpecified() const;
};

class PINKCAB_API FPinkCabMovingFuelSession
{
public:
    explicit FPinkCabMovingFuelSession(
        const FPinkCabMovingFuelPolicyInputs& InPolicy);

    EPinkCabMovingFuelState GetState() const;
    EPinkCabMovingFuelAbortReason GetAbortReason() const;
    const FPinkCabMovingFuelPolicyInputs& GetPolicy() const;
    const FPinkCabLaneId& GetServiceLaneId() const;

    bool CanConnect(
        const FPinkCabVehicleTelemetry& Telemetry,
        float LongitudinalGapCm,
        const FPinkCabLaneId& InServiceLaneId) const;
    bool TryConnect(
        const FPinkCabRoadGraph& Graph,
        const FPinkCabVehicleTelemetry& Telemetry,
        float LongitudinalGapCm,
        const FPinkCabLaneId& InServiceLaneId);
    bool MaintainConnection(
        const FPinkCabRoadGraph& Graph,
        const FPinkCabVehicleTelemetry& Telemetry,
        float LongitudinalGapCm);
    bool MarkConnected();
    bool BeginFueling();
    bool Complete();
    bool Abort();
    bool NotifyCollision();
    EPinkCabFuelCreditResult ApplySettledFuel(
        FPinkCabFuelTank& Tank,
        const FPinkCabTransactionId& TransactionId,
        float Liters,
        EPinkCabSettlementResult SettlementResult);

private:
    friend class FPinkCabServiceSnapshotCodec;

    bool AbortWithReason(
        EPinkCabMovingFuelAbortReason Reason);

    FPinkCabMovingFuelPolicyInputs Policy;
    EPinkCabMovingFuelState State =
        EPinkCabMovingFuelState::Disconnected;
    EPinkCabMovingFuelAbortReason AbortReason =
        EPinkCabMovingFuelAbortReason::None;
    FPinkCabLaneId ServiceLaneId;
};
