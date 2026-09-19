#include "Persistence/PinkCabFareRuntimeSnapshotCodec.h"

bool FPinkCabFareRuntimeSnapshotCodec::IsValidId(
    const FString& Value)
{
    FPinkCabStableId Parsed;
    return FPinkCabStableId::TryParse(Value, Parsed);
}

bool FPinkCabFareRuntimeSnapshotCodec::ValidatePricing(
    const FPinkCabFarePricingTerms& Terms)
{
    if (static_cast<uint8>(Terms.Mode)
        > static_cast<uint8>(EPinkCabFareMode::OffMeter))
    {
        return false;
    }

    return Terms.BaseMinor >= 0
        && Terms.PerKmMinor >= 0
        && Terms.PerMinuteMinor >= 0
        && Terms.AgreedMinor >= 0;
}

bool FPinkCabFareRuntimeSnapshotCodec::ValidateManifestHeader(
    const FPinkCabFarePassengerManifestSnapshot& Manifest)
{
    return IsValidId(Manifest.FareId)
        && Manifest.Records.Num() >= 1
        && Manifest.Records.Num() <= 5
        && FMath::IsFinite(Manifest.TotalPassengerMassKg)
        && Manifest.TotalPassengerMassKg > 0.0f
        && !(Manifest.bBoarded && Manifest.bExited);
}

bool FPinkCabFareRuntimeSnapshotCodec::ValidateManifestRecord(
    const FPinkCabFarePassengerRecordSnapshot& Record,
    TSet<FString>& PassengerIds,
    TSet<uint8>& Seats,
    float& TotalMassKg)
{
    if (!IsValidId(Record.PassengerId))
    {
        return false;
    }
    if (PassengerIds.Contains(Record.PassengerId))
    {
        return false;
    }

    const uint8 Seat = static_cast<uint8>(Record.Seat);
    if (Seat > static_cast<uint8>(EPinkCabPassengerSeat::Front2)
        || Seats.Contains(Seat))
    {
        return false;
    }
    if (!FMath::IsFinite(Record.MassKg)
        || !FMath::IsFinite(Record.LongitudinalCm)
        || Record.MassKg <= 0.0f)
    {
        return false;
    }

    PassengerIds.Add(Record.PassengerId);
    Seats.Add(Seat);
    TotalMassKg += Record.MassKg;
    return true;
}

bool FPinkCabFareRuntimeSnapshotCodec::ValidateManifest(
    const FPinkCabFarePassengerManifestSnapshot& Manifest)
{
    if (!ValidateManifestHeader(Manifest))
    {
        return false;
    }

    TSet<FString> PassengerIds;
    TSet<uint8> Seats;
    float TotalMassKg = 0.0f;
    for (const FPinkCabFarePassengerRecordSnapshot& Record
        : Manifest.Records)
    {
        if (!ValidateManifestRecord(
                Record,
                PassengerIds,
                Seats,
                TotalMassKg))
        {
            return false;
        }
    }

    return FMath::IsNearlyEqual(
        TotalMassKg,
        Manifest.TotalPassengerMassKg,
        0.01f);
}

bool FPinkCabFareRuntimeSnapshotCodec::ValidateSessionBasics(
    const FPinkCabFareSessionSnapshot& Session)
{
    return IsValidId(Session.FareId)
        && static_cast<uint8>(Session.State)
            <= static_cast<uint8>(EPinkCabFareState::Failed)
        && !(Session.bReceiptTaken && !Session.bReceiptResolved)
        && !(Session.bReceiptResolved && !Session.bPaymentCommitted)
        && !(Session.bStopoverActive
            && Session.State != EPinkCabFareState::Active);
}

bool FPinkCabFareRuntimeSnapshotCodec::ValidateSessionRecovery(
    const FPinkCabFareSessionSnapshot& Session)
{
    if (Session.bRequiresWorkdayEnd
        != Session.bRequiresRepairRecovery)
    {
        return false;
    }
    if (Session.bRequiresWorkdayEnd
        && Session.State != EPinkCabFareState::Failed)
    {
        return false;
    }
    return true;
}

bool FPinkCabFareRuntimeSnapshotCodec::ValidateSession(
    const FPinkCabFareSessionSnapshot& Session)
{
    return ValidateSessionBasics(Session)
        && ValidateSessionRecovery(Session);
}

bool FPinkCabFareRuntimeSnapshotCodec::ValidateTaximeter(
    const FPinkCabTaximeterSnapshot& Taximeter)
{
    return ValidatePricing(Taximeter.Terms)
        && FMath::IsFinite(Taximeter.DistanceKm)
        && FMath::IsFinite(Taximeter.FareSeconds)
        && Taximeter.DistanceKm >= 0.0
        && Taximeter.FareSeconds >= 0.0
        && !(Taximeter.bRunning && !Taximeter.bStarted);
}

bool FPinkCabFareRuntimeSnapshotCodec::ValidateOfferedState(
    const FPinkCabFareRuntimeSnapshot& Snapshot)
{
    return Snapshot.Session.State == EPinkCabFareState::Boarding
        && !Snapshot.Taximeter.bStarted
        && !Snapshot.Taximeter.bRunning
        && !Snapshot.Manifest.bBoarded
        && !Snapshot.Manifest.bExited;
}

bool FPinkCabFareRuntimeSnapshotCodec::ValidateBoardedState(
    const FPinkCabFareRuntimeSnapshot& Snapshot)
{
    return Snapshot.Session.State == EPinkCabFareState::Boarding
        && !Snapshot.Taximeter.bStarted
        && !Snapshot.Taximeter.bRunning
        && Snapshot.Manifest.bBoarded
        && !Snapshot.Manifest.bExited;
}

bool FPinkCabFareRuntimeSnapshotCodec::ValidateActiveState(
    const FPinkCabFareRuntimeSnapshot& Snapshot)
{
    return Snapshot.Session.State == EPinkCabFareState::Active
        && Snapshot.Taximeter.bStarted
        && Snapshot.Taximeter.bRunning
        && Snapshot.Manifest.bBoarded
        && !Snapshot.Manifest.bExited;
}

bool FPinkCabFareRuntimeSnapshotCodec::ValidateAwaitingPaymentState(
    const FPinkCabFareRuntimeSnapshot& Snapshot)
{
    return Snapshot.Session.State
            == EPinkCabFareState::AwaitingPayment
        && Snapshot.Taximeter.bStarted
        && !Snapshot.Taximeter.bRunning
        && Snapshot.Manifest.bBoarded
        && !Snapshot.Manifest.bExited;
}

bool FPinkCabFareRuntimeSnapshotCodec::ValidatePaidState(
    const FPinkCabFareRuntimeSnapshot& Snapshot)
{
    return Snapshot.Session.State == EPinkCabFareState::Paid
        && Snapshot.Session.bPaymentCommitted
        && Snapshot.Taximeter.bStarted
        && !Snapshot.Taximeter.bRunning
        && Snapshot.Manifest.bBoarded
        && !Snapshot.Manifest.bExited;
}

bool FPinkCabFareRuntimeSnapshotCodec::ValidateEvadedState(
    const FPinkCabFareRuntimeSnapshot& Snapshot)
{
    return Snapshot.Session.State == EPinkCabFareState::Evaded
        && !Snapshot.Session.bPaymentCommitted
        && Snapshot.Taximeter.bStarted
        && !Snapshot.Taximeter.bRunning
        && Snapshot.Manifest.bBoarded
        && !Snapshot.Manifest.bExited;
}

bool FPinkCabFareRuntimeSnapshotCodec::ValidateCompleteState(
    const FPinkCabFareRuntimeSnapshot& Snapshot)
{
    const bool bResolvedSession =
        Snapshot.Session.State == EPinkCabFareState::Idle
        || Snapshot.Session.State == EPinkCabFareState::Evaded;
    return bResolvedSession
        && Snapshot.Taximeter.bStarted
        && !Snapshot.Taximeter.bRunning
        && !Snapshot.Manifest.bBoarded
        && Snapshot.Manifest.bExited;
}

bool FPinkCabFareRuntimeSnapshotCodec::ValidateFailedState(
    const FPinkCabFareRuntimeSnapshot& Snapshot)
{
    return Snapshot.Session.State == EPinkCabFareState::Failed
        && Snapshot.Session.bRequiresWorkdayEnd
        && Snapshot.Session.bRequiresRepairRecovery
        && Snapshot.Taximeter.bStarted
        && !Snapshot.Taximeter.bRunning
        && !Snapshot.Manifest.bBoarded
        && Snapshot.Manifest.bExited;
}

bool FPinkCabFareRuntimeSnapshotCodec::ValidateStateMachine(
    const FPinkCabFareRuntimeSnapshot& Snapshot)
{
    switch (Snapshot.LoopState)
    {
    case EPinkCabFareLoopState::Offered:
    case EPinkCabFareLoopState::Pickup:
    case EPinkCabFareLoopState::Declined:
        return ValidateOfferedState(Snapshot);
    case EPinkCabFareLoopState::Boarded:
        return ValidateBoardedState(Snapshot);
    case EPinkCabFareLoopState::Active:
        return ValidateActiveState(Snapshot);
    case EPinkCabFareLoopState::AwaitingPayment:
        return ValidateAwaitingPaymentState(Snapshot);
    case EPinkCabFareLoopState::Paid:
        return ValidatePaidState(Snapshot);
    case EPinkCabFareLoopState::Evaded:
        return ValidateEvadedState(Snapshot);
    case EPinkCabFareLoopState::Complete:
        return ValidateCompleteState(Snapshot);
    case EPinkCabFareLoopState::Failed:
        return ValidateFailedState(Snapshot);
    default:
        return false;
    }
}

bool FPinkCabFareRuntimeSnapshotCodec::ValidateIdentityCoherence(
    const FPinkCabFareRuntimeSnapshot& Snapshot)
{
    return Snapshot.SchemaVersion
            == FPinkCabFareRuntimeSnapshot::CurrentSchemaVersion
        && Snapshot.bInitialized
        && IsValidId(Snapshot.FareId)
        && static_cast<uint8>(Snapshot.LoopState)
            <= static_cast<uint8>(EPinkCabFareLoopState::Failed)
        && Snapshot.Session.FareId == Snapshot.FareId
        && Snapshot.Manifest.FareId == Snapshot.FareId;
}

bool FPinkCabFareRuntimeSnapshotCodec::Validate(
    const FPinkCabFareRuntimeSnapshot& Snapshot)
{
    if (!ValidateIdentityCoherence(Snapshot))
    {
        return false;
    }
    if (!ValidateSession(Snapshot.Session))
    {
        return false;
    }
    if (!ValidateTaximeter(Snapshot.Taximeter))
    {
        return false;
    }
    if (!ValidateManifest(Snapshot.Manifest))
    {
        return false;
    }
    return ValidateStateMachine(Snapshot);
}
