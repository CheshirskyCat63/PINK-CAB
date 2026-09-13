#pragma once

#include "CoreMinimal.h"
#include "Core/PinkCabStableId.h"

enum class EPinkCabFareState : uint8
{
    Boarding,
    Active,
    AwaitingPayment,
    Paid,
    ReadyToReset,
    Idle,
    Evaded,
    Failed
};

enum class EPinkCabFareTransitionResult : uint8
{
    Applied,
    Blocked,
    PolicyRequired
};

struct FPinkCabFullStopGate
{
    static bool IsSatisfied(const float SpeedKmh, const float DwellSeconds)
    {
        return SpeedKmh < 0.5f && DwellSeconds >= 0.4f;
    }
};

struct FPinkCabFarePolicy
{
    static EPinkCabFareTransitionResult ResolveOffMeterEconomy()
    {
        return EPinkCabFareTransitionResult::Applied;
    }
};

struct FPinkCabActiveFareSlot
{
    bool TryActivate(const FPinkCabStableId& FareId)
    {
        if (bOccupied || !FareId.IsValid()) return false;
        ActiveFareId = FareId;
        bOccupied = true;
        return true;
    }

    void Clear()
    {
        ActiveFareId = FPinkCabStableId();
        bOccupied = false;
    }

    bool IsOccupied() const { return bOccupied; }

private:
    FPinkCabStableId ActiveFareId;
    bool bOccupied = false;
};

class FPinkCabFareSession
{
public:
    explicit FPinkCabFareSession(const FPinkCabStableId& InFareId)
        : FareId(InFareId) {}

    EPinkCabFareState GetState() const { return State; }
    bool RequiresWorkdayEnd() const { return bRequiresWorkdayEnd; }
    bool RequiresRepairRecovery() const { return bRequiresRepairRecovery; }

    EPinkCabFareTransitionResult TryStartMeter(const bool bAllPassengersSeated, const bool bDoorLatched)
    {
        if (State != EPinkCabFareState::Boarding || !bAllPassengersSeated || !bDoorLatched)
            return EPinkCabFareTransitionResult::Blocked;
        State = EPinkCabFareState::Active;
        return EPinkCabFareTransitionResult::Applied;
    }

    EPinkCabFareTransitionResult TryStopMeter(const bool bDestinationEligible, const bool bFullStop)
    {
        if (State != EPinkCabFareState::Active || !bDestinationEligible || !bFullStop)
            return EPinkCabFareTransitionResult::Blocked;
        State = EPinkCabFareState::AwaitingPayment;
        return EPinkCabFareTransitionResult::Applied;
    }

    EPinkCabFareTransitionResult CommitPaymentSwipe()
    {
        if (State != EPinkCabFareState::AwaitingPayment)
            return EPinkCabFareTransitionResult::Blocked;
        bPaymentCommitted = true;
        State = EPinkCabFareState::Paid;
        return EPinkCabFareTransitionResult::Applied;
    }

    EPinkCabFareTransitionResult ResolveReceipt(const bool bTaken)
    {
        if (State != EPinkCabFareState::Paid)
            return EPinkCabFareTransitionResult::Blocked;
        bReceiptResolved = true;
        bReceiptTaken = bTaken;
        return EPinkCabFareTransitionResult::Applied;
    }

    EPinkCabFareTransitionResult MarkPassengersExited()
    {
        if (State != EPinkCabFareState::Paid)
            return EPinkCabFareTransitionResult::Blocked;
        State = EPinkCabFareState::ReadyToReset;
        return EPinkCabFareTransitionResult::Applied;
    }

    EPinkCabFareTransitionResult ResetToIdle()
    {
        if (State != EPinkCabFareState::ReadyToReset)
            return EPinkCabFareTransitionResult::Blocked;
        State = EPinkCabFareState::Idle;
        return EPinkCabFareTransitionResult::Applied;
    }

    EPinkCabFareTransitionResult TryUnpaidEscape(const bool bExitPathOpen)
    {
        if (State != EPinkCabFareState::AwaitingPayment || bPaymentCommitted || !bExitPathOpen)
            return EPinkCabFareTransitionResult::Blocked;
        State = EPinkCabFareState::Evaded;
        return EPinkCabFareTransitionResult::Applied;
    }

    EPinkCabFareTransitionResult BeginPassengerStopover()
    {
        if (State != EPinkCabFareState::Active)
            return EPinkCabFareTransitionResult::Blocked;
        bStopoverActive = true;
        return EPinkCabFareTransitionResult::Applied;
    }

    EPinkCabFareTransitionResult MarkStopoverNoReturn()
    {
        if (State != EPinkCabFareState::Active || !bStopoverActive)
            return EPinkCabFareTransitionResult::Blocked;
        State = EPinkCabFareState::Evaded;
        bStopoverActive = false;
        return EPinkCabFareTransitionResult::Applied;
    }

    bool ShouldAccrueFareTime(const bool bHardPaused) const
    {
        return State == EPinkCabFareState::Active && !bHardPaused;
    }

    EPinkCabFareTransitionResult FailForSeriousCrash()
    {
        if (State != EPinkCabFareState::Active && State != EPinkCabFareState::AwaitingPayment)
            return EPinkCabFareTransitionResult::Blocked;
        State = EPinkCabFareState::Failed;
        bRequiresWorkdayEnd = true;
        bRequiresRepairRecovery = true;
        return EPinkCabFareTransitionResult::Applied;
    }

private:
    FPinkCabStableId FareId;
    EPinkCabFareState State = EPinkCabFareState::Boarding;
    bool bPaymentCommitted = false;
    bool bReceiptResolved = false;
    bool bReceiptTaken = false;
    bool bStopoverActive = false;
    bool bRequiresWorkdayEnd = false;
    bool bRequiresRepairRecovery = false;
};
