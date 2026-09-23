#pragma once

#include "CoreMinimal.h"
#include "Taxi/PinkCabFareSession.h"
#include "Taxi/PinkCabFarePassengerManifest.h"
#include "Taxi/PinkCabOrder.h"
#include "Taxi/PinkCabTaximeter.h"
#include "Economy/PinkCabFareSettlementService.h"

enum class EPinkCabFareLoopState : uint8
{
    Offered,
    Pickup,
    Boarded,
    Active,
    AwaitingPayment,
    Paid,
    Evaded,
    Complete,
    Declined,
    Failed
};

class PINKCABTAXI_API FPinkCabFareLoopCoordinator
{
public:
    static bool TryCreate(
        const FPinkCabStableId& FareId,
        const FPinkCabOrder& Order,
        const FPinkCabFarePricingTerms& PricingTerms,
        const FPinkCabFarePassengerManifest& Manifest,
        FPinkCabFareLoopCoordinator& OutCoordinator);

    EPinkCabFareLoopState GetState() const;
    int64 GetFareMinor() const;
    bool TryAcceptPickup(bool bPickupEligible, bool bFullStop);
    bool TryDecline(bool bFullStop);
    bool TryBoard(
        bool bFullStop,
        bool bDoorOpen,
        FPinkCabVehicleLoadState& LoadState);
    bool TryStartRide(bool bDoorLatched);
    void TickFare(
        double DistanceDeltaKm,
        double DeltaSeconds,
        bool bHardPaused);
    bool TryArrive(bool bDestinationEligible, bool bFullStop);
    bool TryPay(
        int64 TipMinor,
        FPinkCabEconomyLedger& Ledger,
        FPinkCabFareSettlementService& Settlement);
    bool TryEvade(
        bool bExitPathOpen,
        FPinkCabFareSettlementService& Settlement);
    bool TryExit(
        bool bFullStop,
        bool bDoorOpen,
        FPinkCabVehicleLoadState& LoadState);
    bool FailForTerminalRecovery(FPinkCabVehicleLoadState& LoadState);
    bool CanAcceptNextFare() const;

private:
    friend class FPinkCabFareRuntimeSnapshotCodec;

    FPinkCabStableId FareId;
    TOptional<FPinkCabFareSession> FareSession;
    TOptional<FPinkCabTaximeter> Taximeter;
    TOptional<FPinkCabFarePassengerManifest> Manifest;
    EPinkCabFareLoopState State = EPinkCabFareLoopState::Offered;
    bool bInitialized = false;
};
