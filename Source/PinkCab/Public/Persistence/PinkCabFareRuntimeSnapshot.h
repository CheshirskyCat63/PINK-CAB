#pragma once

#include "CoreMinimal.h"
#include "Core/PinkCabStableId.h"
#include "Taxi/PinkCabFareLoopCoordinator.h"

struct FPinkCabFareSessionSnapshot
{
    FString FareId;
    EPinkCabFareState State = EPinkCabFareState::Boarding;
    bool bPaymentCommitted = false;
    bool bReceiptResolved = false;
    bool bReceiptTaken = false;
    bool bStopoverActive = false;
    bool bRequiresWorkdayEnd = false;
    bool bRequiresRepairRecovery = false;
};

struct FPinkCabTaximeterSnapshot
{
    FPinkCabFarePricingTerms Terms;
    double DistanceKm = 0.0;
    double FareSeconds = 0.0;
    bool bStarted = false;
    bool bRunning = false;
};

struct FPinkCabFarePassengerRecordSnapshot
{
    FString PassengerId;
    EPinkCabPassengerSeat Seat = EPinkCabPassengerSeat::Rear1;
    float MassKg = 0.0f;
    float LongitudinalCm = 0.0f;
};
struct FPinkCabFarePassengerManifestSnapshot
{
    FString FareId;
    TArray<FPinkCabFarePassengerRecordSnapshot> Records;
    float TotalPassengerMassKg = 0.0f;
    bool bBoarded = false;
    bool bExited = false;
};

struct FPinkCabFareRuntimeSnapshot
{
    static constexpr int32 CurrentSchemaVersion = 1;

    int32 SchemaVersion = CurrentSchemaVersion;
    FString FareId;
    EPinkCabFareLoopState LoopState = EPinkCabFareLoopState::Offered;
    bool bInitialized = false;
    FPinkCabFareSessionSnapshot Session;
    FPinkCabTaximeterSnapshot Taximeter;
    FPinkCabFarePassengerManifestSnapshot Manifest;
};

class FPinkCabFareRuntimeSnapshotCodec
{
public:
    static bool Capture(
        const FPinkCabFareLoopCoordinator& Coordinator,
        FPinkCabFareRuntimeSnapshot& OutSnapshot)
    {
        if (!Coordinator.bInitialized)
        {
            return false;
        }
        FPinkCabFareRuntimeSnapshot Snapshot;        if (!Coordinator.FareSession.IsSet()
            || !Coordinator.Taximeter.IsSet()
            || !Coordinator.Manifest.IsSet())
        {
            return false;
        }
        Snapshot.FareId = Coordinator.FareId.Serialize();
        Snapshot.LoopState = Coordinator.State;
        Snapshot.bInitialized = true;

        const FPinkCabFareSession& Session = Coordinator.FareSession.GetValue();
        Snapshot.Session.FareId = Session.FareId.Serialize();
        Snapshot.Session.State = Session.State;
        Snapshot.Session.bPaymentCommitted = Session.bPaymentCommitted;
        Snapshot.Session.bReceiptResolved = Session.bReceiptResolved;
        Snapshot.Session.bReceiptTaken = Session.bReceiptTaken;
        Snapshot.Session.bStopoverActive = Session.bStopoverActive;
        Snapshot.Session.bRequiresWorkdayEnd = Session.bRequiresWorkdayEnd;
        Snapshot.Session.bRequiresRepairRecovery = Session.bRequiresRepairRecovery;

        const FPinkCabTaximeter& Taximeter = Coordinator.Taximeter.GetValue();
        Snapshot.Taximeter.Terms = Taximeter.Terms;
        Snapshot.Taximeter.DistanceKm = Taximeter.DistanceKm;
        Snapshot.Taximeter.FareSeconds = Taximeter.FareSeconds;
        Snapshot.Taximeter.bStarted = Taximeter.bStarted;
        Snapshot.Taximeter.bRunning = Taximeter.bRunning;

        const FPinkCabFarePassengerManifest& Manifest = Coordinator.Manifest.GetValue();
        Snapshot.Manifest.FareId = Manifest.FareId.Serialize();
        Snapshot.Manifest.TotalPassengerMassKg = Manifest.TotalPassengerMassKg;
        Snapshot.Manifest.bBoarded = Manifest.bBoarded;
        Snapshot.Manifest.bExited = Manifest.bExited;        for (const FPinkCabFarePassengerRecord& Record : Manifest.Records)
        {
            Snapshot.Manifest.Records.Add({
                Record.PassengerId.Serialize(),
                Record.Seat,
                Record.MassKg,
                Record.LongitudinalCm});
        }
        if (!Validate(Snapshot))
        {
            return false;
        }
        OutSnapshot = MoveTemp(Snapshot);
        return true;
    }

    static bool Restore(
        const FPinkCabFareRuntimeSnapshot& Snapshot,
        FPinkCabFareLoopCoordinator& OutCoordinator)
    {
        if (!Validate(Snapshot))
        {
            return false;
        }

        FPinkCabFareSession Session(FPinkCabStableId(Snapshot.Session.FareId));
        Session.State = Snapshot.Session.State;
        Session.bPaymentCommitted = Snapshot.Session.bPaymentCommitted;
        Session.bReceiptResolved = Snapshot.Session.bReceiptResolved;
        Session.bReceiptTaken = Snapshot.Session.bReceiptTaken;
        Session.bStopoverActive = Snapshot.Session.bStopoverActive;
        Session.bRequiresWorkdayEnd = Snapshot.Session.bRequiresWorkdayEnd;
        Session.bRequiresRepairRecovery = Snapshot.Session.bRequiresRepairRecovery;
        FPinkCabTaximeter Taximeter(Snapshot.Taximeter.Terms);
        Taximeter.DistanceKm = Snapshot.Taximeter.DistanceKm;
        Taximeter.FareSeconds = Snapshot.Taximeter.FareSeconds;
        Taximeter.bStarted = Snapshot.Taximeter.bStarted;
        Taximeter.bRunning = Snapshot.Taximeter.bRunning;

        FPinkCabFarePassengerManifest Manifest;
        Manifest.FareId = FPinkCabStableId(Snapshot.Manifest.FareId);
        Manifest.TotalPassengerMassKg = Snapshot.Manifest.TotalPassengerMassKg;
        Manifest.bBoarded = Snapshot.Manifest.bBoarded;
        Manifest.bExited = Snapshot.Manifest.bExited;
        for (const FPinkCabFarePassengerRecordSnapshot& Saved : Snapshot.Manifest.Records)
        {
            FPinkCabFarePassengerRecord Record;
            Record.PassengerId = FPinkCabStableId(Saved.PassengerId);
            Record.Seat = Saved.Seat;
            Record.MassKg = Saved.MassKg;
            Record.LongitudinalCm = Saved.LongitudinalCm;
            Manifest.Records.Add(MoveTemp(Record));
        }

        FPinkCabFareLoopCoordinator Coordinator;
        Coordinator.FareId = FPinkCabStableId(Snapshot.FareId);
        Coordinator.FareSession.Emplace(MoveTemp(Session));
        Coordinator.Taximeter.Emplace(MoveTemp(Taximeter));
        Coordinator.Manifest.Emplace(MoveTemp(Manifest));
        Coordinator.State = Snapshot.LoopState;
        Coordinator.bInitialized = true;
        OutCoordinator = MoveTemp(Coordinator);
        return true;
    }

private:
    static bool IsValidId(const FString& Value)
    {
        FPinkCabStableId Parsed;
        return FPinkCabStableId::TryParse(Value, Parsed);
    }

    static bool ValidatePricing(const FPinkCabFarePricingTerms& Terms)
    {
        if (static_cast<uint8>(Terms.Mode) > static_cast<uint8>(EPinkCabFareMode::OffMeter))
        {
            return false;
        }
        return Terms.BaseMinor >= 0
            && Terms.PerKmMinor >= 0
            && Terms.PerMinuteMinor >= 0
            && Terms.AgreedMinor >= 0;
    }

    static bool ValidateManifest(const FPinkCabFarePassengerManifestSnapshot& Manifest)
    {
        if (!IsValidId(Manifest.FareId)
            || Manifest.Records.Num() < 1
            || Manifest.Records.Num() > 5
            || !FMath::IsFinite(Manifest.TotalPassengerMassKg)
            || Manifest.TotalPassengerMassKg <= 0.0f
            || (Manifest.bBoarded && Manifest.bExited))
        {
            return false;
        }

        TSet<FString> PassengerIds;
        TSet<uint8> Seats;
        float TotalMassKg = 0.0f;        for (const FPinkCabFarePassengerRecordSnapshot& Record : Manifest.Records)
        {
            if (!IsValidId(Record.PassengerId)
                || PassengerIds.Contains(Record.PassengerId)
                || static_cast<uint8>(Record.Seat) > static_cast<uint8>(EPinkCabPassengerSeat::Front2)
                || Seats.Contains(static_cast<uint8>(Record.Seat))
                || !FMath::IsFinite(Record.MassKg)
                || !FMath::IsFinite(Record.LongitudinalCm)
                || Record.MassKg <= 0.0f)
            {
                return false;
            }
            PassengerIds.Add(Record.PassengerId);
            Seats.Add(static_cast<uint8>(Record.Seat));
            TotalMassKg += Record.MassKg;
        }
        return FMath::IsNearlyEqual(TotalMassKg, Manifest.TotalPassengerMassKg, 0.01f);
    }

    static bool ValidateSession(const FPinkCabFareSessionSnapshot& Session)
    {
        if (!IsValidId(Session.FareId)
            || static_cast<uint8>(Session.State) > static_cast<uint8>(EPinkCabFareState::Failed)
            || (Session.bReceiptTaken && !Session.bReceiptResolved)
            || (Session.bReceiptResolved && !Session.bPaymentCommitted)
            || (Session.bStopoverActive && Session.State != EPinkCabFareState::Active))
        {
            return false;
        }
        if (Session.bRequiresWorkdayEnd != Session.bRequiresRepairRecovery)
        {
            return false;
        }
        if (Session.bRequiresWorkdayEnd && Session.State != EPinkCabFareState::Failed)
        {
            return false;
        }
        return true;
    }

    static bool ValidateStateMachine(const FPinkCabFareRuntimeSnapshot& Snapshot)
    {
        const bool bStarted = Snapshot.Taximeter.bStarted;
        const bool bRunning = Snapshot.Taximeter.bRunning;
        const bool bBoarded = Snapshot.Manifest.bBoarded;
        const bool bExited = Snapshot.Manifest.bExited;
        switch (Snapshot.LoopState)
        {
        case EPinkCabFareLoopState::Offered:
        case EPinkCabFareLoopState::Pickup:
        case EPinkCabFareLoopState::Declined:
            return Snapshot.Session.State == EPinkCabFareState::Boarding
                && !bStarted && !bRunning && !bBoarded && !bExited;
        case EPinkCabFareLoopState::Boarded:
            return Snapshot.Session.State == EPinkCabFareState::Boarding
                && !bStarted && !bRunning && bBoarded && !bExited;
        case EPinkCabFareLoopState::Active:
            return Snapshot.Session.State == EPinkCabFareState::Active
                && bStarted && bRunning && bBoarded && !bExited;
        case EPinkCabFareLoopState::AwaitingPayment:
            return Snapshot.Session.State == EPinkCabFareState::AwaitingPayment
                && bStarted && !bRunning && bBoarded && !bExited;
        case EPinkCabFareLoopState::Paid:
            return Snapshot.Session.State == EPinkCabFareState::Paid
                && Snapshot.Session.bPaymentCommitted
                && bStarted && !bRunning && bBoarded && !bExited;
        case EPinkCabFareLoopState::Evaded:
            return Snapshot.Session.State == EPinkCabFareState::Evaded
                && !Snapshot.Session.bPaymentCommitted
                && bStarted && !bRunning && bBoarded && !bExited;        case EPinkCabFareLoopState::Complete:
            return (Snapshot.Session.State == EPinkCabFareState::Idle
                    || Snapshot.Session.State == EPinkCabFareState::Evaded)
                && bStarted && !bRunning && !bBoarded && bExited;
        default:
            return false;
        }
    }

    static bool Validate(const FPinkCabFareRuntimeSnapshot& Snapshot)
    {
        if (Snapshot.SchemaVersion != FPinkCabFareRuntimeSnapshot::CurrentSchemaVersion
            || !Snapshot.bInitialized
            || !IsValidId(Snapshot.FareId)
            || static_cast<uint8>(Snapshot.LoopState) > static_cast<uint8>(EPinkCabFareLoopState::Declined)
            || Snapshot.Session.FareId != Snapshot.FareId
            || Snapshot.Manifest.FareId != Snapshot.FareId
            || !ValidateSession(Snapshot.Session)
            || !ValidatePricing(Snapshot.Taximeter.Terms)
            || !FMath::IsFinite(Snapshot.Taximeter.DistanceKm)
            || !FMath::IsFinite(Snapshot.Taximeter.FareSeconds)
            || Snapshot.Taximeter.DistanceKm < 0.0
            || Snapshot.Taximeter.FareSeconds < 0.0
            || (Snapshot.Taximeter.bRunning && !Snapshot.Taximeter.bStarted)
            || !ValidateManifest(Snapshot.Manifest))
        {
            return false;
        }
        return ValidateStateMachine(Snapshot);
    }
};
