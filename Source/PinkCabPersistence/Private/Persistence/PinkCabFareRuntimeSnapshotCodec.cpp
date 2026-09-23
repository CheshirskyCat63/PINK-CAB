#include "Persistence/PinkCabFareRuntimeSnapshotCodec.h"

bool FPinkCabFareRuntimeSnapshotCodec::Capture(
    const FPinkCabFareLoopCoordinator& Coordinator,
    FPinkCabFareRuntimeSnapshot& OutSnapshot)
{
    if (!Coordinator.bInitialized
        || !Coordinator.FareSession.IsSet()
        || !Coordinator.Taximeter.IsSet()
        || !Coordinator.Manifest.IsSet())
    {
        return false;
    }

    FPinkCabFareRuntimeSnapshot Snapshot;
    Snapshot.FareId = Coordinator.FareId.Serialize();
    Snapshot.LoopState = Coordinator.State;
    Snapshot.bInitialized = true;

    const FPinkCabFareSession& Session =
        Coordinator.FareSession.GetValue();
    Snapshot.Session.FareId = Session.FareId.Serialize();
    Snapshot.Session.State = Session.State;
    Snapshot.Session.bPaymentCommitted = Session.bPaymentCommitted;
    Snapshot.Session.bReceiptResolved = Session.bReceiptResolved;
    Snapshot.Session.bReceiptTaken = Session.bReceiptTaken;
    Snapshot.Session.bStopoverActive = Session.bStopoverActive;
    Snapshot.Session.bRequiresWorkdayEnd = Session.bRequiresWorkdayEnd;
    Snapshot.Session.bRequiresRepairRecovery =
        Session.bRequiresRepairRecovery;

    const FPinkCabTaximeter& Taximeter =
        Coordinator.Taximeter.GetValue();
    Snapshot.Taximeter.Terms = Taximeter.Terms;
    Snapshot.Taximeter.DistanceKm = Taximeter.DistanceKm;
    Snapshot.Taximeter.FareSeconds = Taximeter.FareSeconds;
    Snapshot.Taximeter.bStarted = Taximeter.bStarted;
    Snapshot.Taximeter.bRunning = Taximeter.bRunning;

    const FPinkCabFarePassengerManifest& Manifest =
        Coordinator.Manifest.GetValue();
    Snapshot.Manifest.FareId = Manifest.FareId.Serialize();
    Snapshot.Manifest.TotalPassengerMassKg =
        Manifest.TotalPassengerMassKg;
    Snapshot.Manifest.bBoarded = Manifest.bBoarded;
    Snapshot.Manifest.bExited = Manifest.bExited;

    for (const FPinkCabFarePassengerRecord& Record : Manifest.Records)
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

bool FPinkCabFareRuntimeSnapshotCodec::Restore(
    const FPinkCabFareRuntimeSnapshot& Snapshot,
    FPinkCabFareLoopCoordinator& OutCoordinator)
{
    if (!Validate(Snapshot))
    {
        return false;
    }

    FPinkCabFareSession Session(
        FPinkCabStableId(Snapshot.Session.FareId));
    Session.State = Snapshot.Session.State;
    Session.bPaymentCommitted = Snapshot.Session.bPaymentCommitted;
    Session.bReceiptResolved = Snapshot.Session.bReceiptResolved;
    Session.bReceiptTaken = Snapshot.Session.bReceiptTaken;
    Session.bStopoverActive = Snapshot.Session.bStopoverActive;
    Session.bRequiresWorkdayEnd = Snapshot.Session.bRequiresWorkdayEnd;
    Session.bRequiresRepairRecovery =
        Snapshot.Session.bRequiresRepairRecovery;

    FPinkCabTaximeter Taximeter(Snapshot.Taximeter.Terms);
    Taximeter.DistanceKm = Snapshot.Taximeter.DistanceKm;
    Taximeter.FareSeconds = Snapshot.Taximeter.FareSeconds;
    Taximeter.bStarted = Snapshot.Taximeter.bStarted;
    Taximeter.bRunning = Snapshot.Taximeter.bRunning;

    FPinkCabFarePassengerManifest Manifest;
    Manifest.FareId = FPinkCabStableId(Snapshot.Manifest.FareId);
    Manifest.TotalPassengerMassKg =
        Snapshot.Manifest.TotalPassengerMassKg;
    Manifest.bBoarded = Snapshot.Manifest.bBoarded;
    Manifest.bExited = Snapshot.Manifest.bExited;

    for (const FPinkCabFarePassengerRecordSnapshot& Saved
        : Snapshot.Manifest.Records)
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
