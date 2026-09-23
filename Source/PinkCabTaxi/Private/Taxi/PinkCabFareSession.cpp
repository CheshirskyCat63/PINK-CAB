#include "Taxi/PinkCabFareSession.h"

bool FPinkCabFullStopGate::IsSatisfied(float SpeedKmh, float DwellSeconds)
{
    return SpeedKmh < 0.5f && DwellSeconds >= 0.4f;
}

EPinkCabFareTransitionResult FPinkCabFarePolicy::ResolveOffMeterEconomy()
{
    return EPinkCabFareTransitionResult::Applied;
}

bool FPinkCabActiveFareSlot::TryActivate(const FPinkCabStableId& FareId)
{
    if (bOccupied || !FareId.IsValid()) return false;
    ActiveFareId = FareId;
    bOccupied = true;
    return true;
}

void FPinkCabActiveFareSlot::Clear()
{
    ActiveFareId = FPinkCabStableId();
    bOccupied = false;
}

bool FPinkCabActiveFareSlot::IsOccupied() const
{
    return bOccupied;
}

FPinkCabFareSession::FPinkCabFareSession(const FPinkCabStableId& InFareId)
    : FareId(InFareId)
{
}

EPinkCabFareState FPinkCabFareSession::GetState() const { return State; }
bool FPinkCabFareSession::RequiresWorkdayEnd() const { return bRequiresWorkdayEnd; }
bool FPinkCabFareSession::RequiresRepairRecovery() const { return bRequiresRepairRecovery; }

EPinkCabFareTransitionResult FPinkCabFareSession::TryStartMeter(
    bool bAllPassengersSeated,
    bool bDoorLatched)
{
    if (State != EPinkCabFareState::Boarding
        || !bAllPassengersSeated
        || !bDoorLatched)
    {
        return EPinkCabFareTransitionResult::Blocked;
    }
    State = EPinkCabFareState::Active;
    return EPinkCabFareTransitionResult::Applied;
}

EPinkCabFareTransitionResult FPinkCabFareSession::TryStopMeter(
    bool bDestinationEligible,
    bool bFullStop)
{
    if (State != EPinkCabFareState::Active
        || !bDestinationEligible
        || !bFullStop)
    {
        return EPinkCabFareTransitionResult::Blocked;
    }
    State = EPinkCabFareState::AwaitingPayment;
    return EPinkCabFareTransitionResult::Applied;
}

EPinkCabFareTransitionResult FPinkCabFareSession::CommitPaymentSwipe()
{
    if (State != EPinkCabFareState::AwaitingPayment)
    {
        return EPinkCabFareTransitionResult::Blocked;
    }
    bPaymentCommitted = true;
    State = EPinkCabFareState::Paid;
    return EPinkCabFareTransitionResult::Applied;
}

EPinkCabFareTransitionResult FPinkCabFareSession::ResolveReceipt(bool bTaken)
{
    if (State != EPinkCabFareState::Paid)
    {
        return EPinkCabFareTransitionResult::Blocked;
    }
    bReceiptResolved = true;
    bReceiptTaken = bTaken;
    return EPinkCabFareTransitionResult::Applied;
}

EPinkCabFareTransitionResult FPinkCabFareSession::MarkPassengersExited()
{
    if (State != EPinkCabFareState::Paid)
    {
        return EPinkCabFareTransitionResult::Blocked;
    }
    State = EPinkCabFareState::ReadyToReset;
    return EPinkCabFareTransitionResult::Applied;
}

EPinkCabFareTransitionResult FPinkCabFareSession::ResetToIdle()
{
    if (State != EPinkCabFareState::ReadyToReset)
    {
        return EPinkCabFareTransitionResult::Blocked;
    }
    State = EPinkCabFareState::Idle;
    return EPinkCabFareTransitionResult::Applied;
}

EPinkCabFareTransitionResult FPinkCabFareSession::TryUnpaidEscape(
    bool bExitPathOpen)
{
    if (State != EPinkCabFareState::AwaitingPayment
        || bPaymentCommitted
        || !bExitPathOpen)
    {
        return EPinkCabFareTransitionResult::Blocked;
    }
    State = EPinkCabFareState::Evaded;
    return EPinkCabFareTransitionResult::Applied;
}

EPinkCabFareTransitionResult FPinkCabFareSession::BeginPassengerStopover()
{
    if (State != EPinkCabFareState::Active)
    {
        return EPinkCabFareTransitionResult::Blocked;
    }
    bStopoverActive = true;
    return EPinkCabFareTransitionResult::Applied;
}

EPinkCabFareTransitionResult FPinkCabFareSession::MarkStopoverNoReturn()
{
    if (State != EPinkCabFareState::Active || !bStopoverActive)
    {
        return EPinkCabFareTransitionResult::Blocked;
    }
    State = EPinkCabFareState::Evaded;
    bStopoverActive = false;
    return EPinkCabFareTransitionResult::Applied;
}

bool FPinkCabFareSession::ShouldAccrueFareTime(bool bHardPaused) const
{
    return State == EPinkCabFareState::Active && !bHardPaused;
}

EPinkCabFareTransitionResult FPinkCabFareSession::FailForSeriousCrash()
{
    if (State != EPinkCabFareState::Active
        && State != EPinkCabFareState::AwaitingPayment)
    {
        return EPinkCabFareTransitionResult::Blocked;
    }
    State = EPinkCabFareState::Failed;
    bRequiresWorkdayEnd = true;
    bRequiresRepairRecovery = true;
    return EPinkCabFareTransitionResult::Applied;
}
