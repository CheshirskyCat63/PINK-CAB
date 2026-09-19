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

struct PINKCABTAXI_API FPinkCabFullStopGate
{
    static bool IsSatisfied(float SpeedKmh, float DwellSeconds);
};

struct PINKCABTAXI_API FPinkCabFarePolicy
{
    static EPinkCabFareTransitionResult ResolveOffMeterEconomy();
};

struct PINKCABTAXI_API FPinkCabActiveFareSlot
{
    bool TryActivate(const FPinkCabStableId& FareId);
    void Clear();
    bool IsOccupied() const;

private:
    FPinkCabStableId ActiveFareId;
    bool bOccupied = false;
};

class PINKCABTAXI_API FPinkCabFareSession
{
public:
    explicit FPinkCabFareSession(const FPinkCabStableId& InFareId);

    EPinkCabFareState GetState() const;
    bool RequiresWorkdayEnd() const;
    bool RequiresRepairRecovery() const;

    EPinkCabFareTransitionResult TryStartMeter(
        bool bAllPassengersSeated,
        bool bDoorLatched);
    EPinkCabFareTransitionResult TryStopMeter(
        bool bDestinationEligible,
        bool bFullStop);
    EPinkCabFareTransitionResult CommitPaymentSwipe();
    EPinkCabFareTransitionResult ResolveReceipt(bool bTaken);
    EPinkCabFareTransitionResult MarkPassengersExited();
    EPinkCabFareTransitionResult ResetToIdle();
    EPinkCabFareTransitionResult TryUnpaidEscape(bool bExitPathOpen);
    EPinkCabFareTransitionResult BeginPassengerStopover();
    EPinkCabFareTransitionResult MarkStopoverNoReturn();
    bool ShouldAccrueFareTime(bool bHardPaused) const;
    EPinkCabFareTransitionResult FailForSeriousCrash();

private:
    friend class FPinkCabFareRuntimeSnapshotCodec;

    FPinkCabStableId FareId;
    EPinkCabFareState State = EPinkCabFareState::Boarding;
    bool bPaymentCommitted = false;
    bool bReceiptResolved = false;
    bool bReceiptTaken = false;
    bool bStopoverActive = false;
    bool bRequiresWorkdayEnd = false;
    bool bRequiresRepairRecovery = false;
};
