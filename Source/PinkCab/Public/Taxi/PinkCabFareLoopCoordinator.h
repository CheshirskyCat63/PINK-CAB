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
    Declined
};

class FPinkCabFareLoopCoordinator
{
public:
    static bool TryCreate(
        const FPinkCabStableId& FareId,
        const FPinkCabOrder& Order,
        const FPinkCabFarePricingTerms& PricingTerms,
        const FPinkCabFarePassengerManifest& Manifest,
        FPinkCabFareLoopCoordinator& OutCoordinator)
    {        if (!FareId.IsValid() || !Order.IsValid() || Order.FareMode != PricingTerms.Mode)
        {
            return false;
        }

        const TConstArrayView<FPinkCabFarePassengerRecord> Records = Manifest.GetRecords();
        if (Records.Num() != Order.PassengerIdentityIds.Num())
        {
            return false;
        }
        for (int32 Index = 0; Index < Records.Num(); ++Index)
        {
            if (Records[Index].PassengerId != Order.PassengerIdentityIds[Index])
            {
                return false;
            }
        }

        FPinkCabFareLoopCoordinator Built;
        Built.FareId = FareId;
        Built.FareSession.Emplace(FareId);
        Built.Taximeter.Emplace(PricingTerms);
        Built.Manifest.Emplace(Manifest);
        Built.bInitialized = true;
        OutCoordinator = MoveTemp(Built);
        return true;
    }

    EPinkCabFareLoopState GetState() const { return State; }
    int64 GetFareMinor() const { return Taximeter.IsSet() ? Taximeter->GetFareMinor() : 0; }
    bool TryAcceptPickup(const bool bPickupEligible, const bool bFullStop)
    {
        if (!bInitialized || State != EPinkCabFareLoopState::Offered || !bPickupEligible || !bFullStop)
        {
            return false;
        }
        State = EPinkCabFareLoopState::Pickup;
        return true;
    }

    bool TryDecline(const bool bFullStop)
    {
        if (!bInitialized || State != EPinkCabFareLoopState::Offered || !bFullStop)
        {
            return false;
        }
        State = EPinkCabFareLoopState::Declined;
        return true;
    }

    bool TryBoard(const bool bFullStop, const bool bDoorOpen, FPinkCabVehicleLoadState& LoadState)
    {
        if (!bInitialized || State != EPinkCabFareLoopState::Pickup
            || !Manifest.GetValue().TryBoard(bFullStop, bDoorOpen, LoadState))
        {
            return false;
        }
        State = EPinkCabFareLoopState::Boarded;
        return true;
    }
    bool TryStartRide(const bool bDoorLatched)
    {
        if (!bInitialized || State != EPinkCabFareLoopState::Boarded)
        {
            return false;
        }
        if (FareSession.GetValue().TryStartMeter(true, bDoorLatched) != EPinkCabFareTransitionResult::Applied)
        {
            return false;
        }
        if (!Taximeter.GetValue().Start())
        {
            return false;
        }
        State = EPinkCabFareLoopState::Active;
        return true;
    }

    void TickFare(const double DistanceDeltaKm, const double DeltaSeconds, const bool bHardPaused)
    {
        if (bInitialized && State == EPinkCabFareLoopState::Active)
        {
            Taximeter.GetValue().Tick(DistanceDeltaKm, DeltaSeconds, bHardPaused);
        }
    }

    bool TryArrive(const bool bDestinationEligible, const bool bFullStop)
    {
        if (!bInitialized || State != EPinkCabFareLoopState::Active)
        {
            return false;
        }        if (FareSession.GetValue().TryStopMeter(bDestinationEligible, bFullStop) != EPinkCabFareTransitionResult::Applied)
        {
            return false;
        }
        if (!Taximeter.GetValue().Stop())
        {
            return false;
        }
        State = EPinkCabFareLoopState::AwaitingPayment;
        return true;
    }

    bool TryPay(
        const int64 TipMinor,
        FPinkCabEconomyLedger& Ledger,
        FPinkCabFareSettlementService& Settlement)
    {
        if (!bInitialized || State != EPinkCabFareLoopState::AwaitingPayment)
        {
            return false;
        }
        const FPinkCabFareSettlementResult Result = Settlement.CommitPaid(
            FareId, GetFareMinor(), TipMinor, Ledger);
        if (Result.Disposition != EPinkCabFareSettlementDisposition::Committed
            && Result.Disposition != EPinkCabFareSettlementDisposition::Replayed)
        {
            return false;
        }
        if (FareSession.GetValue().CommitPaymentSwipe() != EPinkCabFareTransitionResult::Applied)
        {
            return false;
        }
        State = EPinkCabFareLoopState::Paid;
        return true;
    }
    bool TryEvade(const bool bExitPathOpen, FPinkCabFareSettlementService& Settlement)
    {
        if (!bInitialized || State != EPinkCabFareLoopState::AwaitingPayment || !bExitPathOpen)
        {
            return false;
        }
        const FPinkCabFareSettlementResult Result = Settlement.RecordEvasion(FareId);
        if (Result.Disposition != EPinkCabFareSettlementDisposition::Committed
            && Result.Disposition != EPinkCabFareSettlementDisposition::Replayed)
        {
            return false;
        }
        if (FareSession.GetValue().TryUnpaidEscape(true) != EPinkCabFareTransitionResult::Applied)
        {
            return false;
        }
        State = EPinkCabFareLoopState::Evaded;
        return true;
    }

    bool TryExit(const bool bFullStop, const bool bDoorOpen, FPinkCabVehicleLoadState& LoadState)
    {
        if (!bInitialized || (State != EPinkCabFareLoopState::Paid && State != EPinkCabFareLoopState::Evaded))
        {
            return false;
        }
        const EPinkCabFareState ExpectedFareState = State == EPinkCabFareLoopState::Paid
            ? EPinkCabFareState::Paid
            : EPinkCabFareState::Evaded;
        if (FareSession.GetValue().GetState() != ExpectedFareState)
        {
            return false;
        }
        if (!Manifest.GetValue().TryExit(bFullStop, bDoorOpen, LoadState))
        {
            return false;
        }

        if (State == EPinkCabFareLoopState::Paid)
        {
            if (FareSession.GetValue().MarkPassengersExited() != EPinkCabFareTransitionResult::Applied
                || FareSession.GetValue().ResetToIdle() != EPinkCabFareTransitionResult::Applied)
            {
                return false;
            }
        }

        State = EPinkCabFareLoopState::Complete;
        return true;
    }

    bool CanAcceptNextFare() const
    {
        return State == EPinkCabFareLoopState::Complete || State == EPinkCabFareLoopState::Declined;
    }

private:
    friend class FPinkCabFareRuntimeSnapshotCodec;

    FPinkCabStableId FareId;
    TOptional<FPinkCabFareSession> FareSession;
    TOptional<FPinkCabTaximeter> Taximeter;
    TOptional<FPinkCabFarePassengerManifest> Manifest;
    EPinkCabFareLoopState State = EPinkCabFareLoopState::Offered;
    bool bInitialized = false;
};
