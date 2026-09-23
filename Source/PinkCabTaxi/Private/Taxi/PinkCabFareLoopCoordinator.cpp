#include "Taxi/PinkCabFareLoopCoordinator.h"

bool FPinkCabFareLoopCoordinator::TryCreate(
    const FPinkCabStableId& FareId,
    const FPinkCabOrder& Order,
    const FPinkCabFarePricingTerms& PricingTerms,
    const FPinkCabFarePassengerManifest& Manifest,
    FPinkCabFareLoopCoordinator& OutCoordinator)
{
    if (!FareId.IsValid()
        || !Order.IsValid()
        || Order.FareMode != PricingTerms.Mode)
    {
        return false;
    }

    const TConstArrayView<FPinkCabFarePassengerRecord> Records =
        Manifest.GetRecords();
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

EPinkCabFareLoopState FPinkCabFareLoopCoordinator::GetState() const
{
    return State;
}

int64 FPinkCabFareLoopCoordinator::GetFareMinor() const
{
    return Taximeter.IsSet() ? Taximeter->GetFareMinor() : 0;
}

bool FPinkCabFareLoopCoordinator::TryAcceptPickup(
    bool bPickupEligible,
    bool bFullStop)
{
    if (!bInitialized
        || State != EPinkCabFareLoopState::Offered
        || !bPickupEligible
        || !bFullStop)
    {
        return false;
    }

    State = EPinkCabFareLoopState::Pickup;
    return true;
}

bool FPinkCabFareLoopCoordinator::TryDecline(bool bFullStop)
{
    if (!bInitialized
        || State != EPinkCabFareLoopState::Offered
        || !bFullStop)
    {
        return false;
    }

    State = EPinkCabFareLoopState::Declined;
    return true;
}

bool FPinkCabFareLoopCoordinator::TryBoard(
    bool bFullStop,
    bool bDoorOpen,
    FPinkCabVehicleLoadState& LoadState)
{
    if (!bInitialized
        || State != EPinkCabFareLoopState::Pickup
        || !Manifest.GetValue().TryBoard(bFullStop, bDoorOpen, LoadState))
    {
        return false;
    }

    State = EPinkCabFareLoopState::Boarded;
    return true;
}

bool FPinkCabFareLoopCoordinator::TryStartRide(bool bDoorLatched)
{
    if (!bInitialized || State != EPinkCabFareLoopState::Boarded)
    {
        return false;
    }
    if (FareSession.GetValue().TryStartMeter(true, bDoorLatched)
        != EPinkCabFareTransitionResult::Applied)
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

void FPinkCabFareLoopCoordinator::TickFare(
    double DistanceDeltaKm,
    double DeltaSeconds,
    bool bHardPaused)
{
    if (bInitialized && State == EPinkCabFareLoopState::Active)
    {
        Taximeter.GetValue().Tick(
            DistanceDeltaKm,
            DeltaSeconds,
            bHardPaused);
    }
}

bool FPinkCabFareLoopCoordinator::TryArrive(
    bool bDestinationEligible,
    bool bFullStop)
{
    if (!bInitialized || State != EPinkCabFareLoopState::Active)
    {
        return false;
    }
    if (FareSession.GetValue().TryStopMeter(
        bDestinationEligible,
        bFullStop) != EPinkCabFareTransitionResult::Applied)
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

bool FPinkCabFareLoopCoordinator::TryPay(
    int64 TipMinor,
    FPinkCabEconomyLedger& Ledger,
    FPinkCabFareSettlementService& Settlement)
{
    if (!bInitialized
        || State != EPinkCabFareLoopState::AwaitingPayment)
    {
        return false;
    }

    const FPinkCabFareSettlementResult Result = Settlement.CommitPaid(
        FareId,
        GetFareMinor(),
        TipMinor,
        Ledger);
    if (Result.Disposition != EPinkCabFareSettlementDisposition::Committed
        && Result.Disposition != EPinkCabFareSettlementDisposition::Replayed)
    {
        return false;
    }
    if (FareSession.GetValue().CommitPaymentSwipe()
        != EPinkCabFareTransitionResult::Applied)
    {
        return false;
    }

    State = EPinkCabFareLoopState::Paid;
    return true;
}

bool FPinkCabFareLoopCoordinator::TryEvade(
    bool bExitPathOpen,
    FPinkCabFareSettlementService& Settlement)
{
    if (!bInitialized
        || State != EPinkCabFareLoopState::AwaitingPayment
        || !bExitPathOpen)
    {
        return false;
    }

    const FPinkCabFareSettlementResult Result =
        Settlement.RecordEvasion(FareId);
    if (Result.Disposition != EPinkCabFareSettlementDisposition::Committed
        && Result.Disposition != EPinkCabFareSettlementDisposition::Replayed)
    {
        return false;
    }
    if (FareSession.GetValue().TryUnpaidEscape(true)
        != EPinkCabFareTransitionResult::Applied)
    {
        return false;
    }

    State = EPinkCabFareLoopState::Evaded;
    return true;
}

bool FPinkCabFareLoopCoordinator::TryExit(
    bool bFullStop,
    bool bDoorOpen,
    FPinkCabVehicleLoadState& LoadState)
{
    if (!bInitialized
        || (State != EPinkCabFareLoopState::Paid
            && State != EPinkCabFareLoopState::Evaded))
    {
        return false;
    }

    const EPinkCabFareState ExpectedFareState =
        State == EPinkCabFareLoopState::Paid
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
        if (FareSession.GetValue().MarkPassengersExited()
                != EPinkCabFareTransitionResult::Applied
            || FareSession.GetValue().ResetToIdle()
                != EPinkCabFareTransitionResult::Applied)
        {
            return false;
        }
    }

    State = EPinkCabFareLoopState::Complete;
    return true;
}

bool FPinkCabFareLoopCoordinator::FailForTerminalRecovery(
    FPinkCabVehicleLoadState& LoadState)
{
    if (!bInitialized
        || (State != EPinkCabFareLoopState::Active
            && State != EPinkCabFareLoopState::AwaitingPayment)
        || !FareSession.IsSet()
        || !Taximeter.IsSet()
        || !Manifest.IsSet()
        || !Manifest->IsBoarded()
        || Manifest->HasExited()
        || !LoadState.HasFarePassengerGroup(FareId))
    {
        return false;
    }

    if (FareSession->FailForSeriousCrash()
        != EPinkCabFareTransitionResult::Applied)
    {
        return false;
    }
    if (Taximeter->IsRunning() && !Taximeter->Stop())
    {
        return false;
    }
    if (!Manifest->TryTerminalRecoveryExit(LoadState))
    {
        return false;
    }

    State = EPinkCabFareLoopState::Failed;
    return true;
}

bool FPinkCabFareLoopCoordinator::CanAcceptNextFare() const
{
    return State == EPinkCabFareLoopState::Complete
        || State == EPinkCabFareLoopState::Declined;
}
