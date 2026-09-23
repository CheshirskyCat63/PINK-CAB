#include "Persistence/PinkCabWorldSessionSnapshot.h"

bool FPinkCabWorldSessionSnapshotCodec::CaptureCityDeltas(
    const FPinkCabCityDeltaState& Source,
    FPinkCabCityDeltaSnapshot& OutSnapshot)
{
    FPinkCabCityDeltaSnapshot Snapshot;
    Snapshot.MaxDeltas = Source.MaxDeltas;
    TArray<FString> Keys;
    Source.Deltas.GetKeys(Keys);
    Keys.Sort();
    Snapshot.Records.Reserve(Keys.Num());

    for (const FString& Key : Keys)
    {
        const FPinkCabCityDeltaRecord& Record = Source.Deltas[Key];
        Snapshot.Records.Add({
            Record.DeltaId,
            Record.Kind,
            Record.SubjectStableId,
            Record.OperationKey});
    }

    if (!ValidateCityDeltas(Snapshot))
    {
        return false;
    }
    OutSnapshot = MoveTemp(Snapshot);
    return true;
}

bool FPinkCabWorldSessionSnapshotCodec::RestoreCityDeltas(
    const FPinkCabCityDeltaSnapshot& Snapshot,
    FPinkCabCityDeltaState& OutState)
{
    if (!ValidateCityDeltas(Snapshot))
    {
        return false;
    }

    FPinkCabCityDeltaState Restored(Snapshot.MaxDeltas);
    for (const FPinkCabCityDeltaSnapshotRecord& Serialized : Snapshot.Records)
    {
        FPinkCabCityDeltaRecord Record;
        Record.DeltaId = Serialized.DeltaId;
        Record.Kind = Serialized.Kind;
        Record.SubjectStableId = Serialized.SubjectStableId;
        Record.OperationKey = Serialized.OperationKey;
        Restored.Deltas.Add(Record.DeltaId, Record);

        if (Record.Kind == EPinkCabCityDeltaKind::LaneClosure)
        {
            Restored.ClosedLaneIds.Add(Record.SubjectStableId);
        }
    }

    OutState = MoveTemp(Restored);
    return true;
}

bool FPinkCabWorldSessionSnapshotCodec::CaptureKernel(
    const FPinkCabStateKernel& Source,
    FPinkCabStateKernelSnapshot& OutSnapshot)
{
    FPinkCabStateKernelSnapshot Snapshot;
    Snapshot.RootSeed = Source.RootSeed;
    Snapshot.Sequence = Source.Sequence;
    OutSnapshot = Snapshot;
    return true;
}

bool FPinkCabWorldSessionSnapshotCodec::RestoreKernel(
    const FPinkCabStateKernelSnapshot& Snapshot,
    FPinkCabStateKernel& OutKernel)
{
    if (Snapshot.SchemaVersion
        != FPinkCabStateKernelSnapshot::CurrentSchemaVersion)
    {
        return false;
    }

    FPinkCabStateKernel Restored(Snapshot.RootSeed);
    Restored.Sequence = Snapshot.Sequence;
    OutKernel = Restored;
    return true;
}

bool FPinkCabWorldSessionSnapshotCodec::CaptureWorkday(
    const FPinkCabWorkdaySessionState& Source,
    FPinkCabWorkdaySessionSnapshot& OutSnapshot)
{
    FPinkCabWorkdaySessionSnapshot Snapshot;
    Snapshot.WorkdayId = Source.WorkdayId.Serialize();
    Snapshot.Ordinal = Source.Ordinal;
    Snapshot.ElapsedGameSeconds = Source.ElapsedGameSeconds;
    Snapshot.MaxReplayJournalEntries = Source.MaxReplayJournalEntries;
    Snapshot.bSummaryCommitted = Source.bSummaryCommitted;
    Snapshot.HouseholdTransactionCount = Source.HouseholdTransactionCount;
    Snapshot.bTerminalRecovery = Source.bTerminalRecovery;

    for (const FString& Id : Source.AppliedHouseholdOperationIds)
    {
        Snapshot.AppliedHouseholdOperationIds.Add(Id);
    }
    Snapshot.AppliedHouseholdOperationIds.Sort();

    if (!ValidateWorkday(Snapshot))
    {
        return false;
    }
    OutSnapshot = MoveTemp(Snapshot);
    return true;
}

bool FPinkCabWorldSessionSnapshotCodec::RestoreWorkday(
    const FPinkCabWorkdaySessionSnapshot& Snapshot,
    FPinkCabWorkdaySessionState& OutState)
{
    if (!ValidateWorkday(Snapshot))
    {
        return false;
    }

    FPinkCabStableId WorkdayId;
    if (!FPinkCabStableId::TryParse(Snapshot.WorkdayId, WorkdayId))
    {
        return false;
    }

    FPinkCabWorkdaySessionState Restored;
    if (!FPinkCabWorkdaySessionState::TryCreate(
            WorkdayId,
            Snapshot.Ordinal,
            Snapshot.ElapsedGameSeconds,
            Snapshot.MaxReplayJournalEntries,
            Restored))
    {
        return false;
    }

    Restored.bSummaryCommitted = Snapshot.bSummaryCommitted;
    Restored.HouseholdTransactionCount = Snapshot.HouseholdTransactionCount;
    Restored.bTerminalRecovery = Snapshot.bTerminalRecovery;
    for (const FString& Id : Snapshot.AppliedHouseholdOperationIds)
    {
        Restored.AppliedHouseholdOperationIds.Add(Id);
    }

    OutState = MoveTemp(Restored);
    return true;
}

bool FPinkCabWorldSessionSnapshotCodec::ValidateCityDeltas(
    const FPinkCabCityDeltaSnapshot& Snapshot)
{
    if (Snapshot.SchemaVersion
            != FPinkCabCityDeltaSnapshot::CurrentSchemaVersion
        || Snapshot.MaxDeltas <= 0
        || Snapshot.Records.Num() > Snapshot.MaxDeltas)
    {
        return false;
    }

    TSet<FString> DeltaIds;
    for (const FPinkCabCityDeltaSnapshotRecord& Record : Snapshot.Records)
    {
        const uint8 KindValue = static_cast<uint8>(Record.Kind);
        if (Record.DeltaId.TrimStartAndEnd().IsEmpty()
            || Record.SubjectStableId.TrimStartAndEnd().IsEmpty()
            || Record.OperationKey.TrimStartAndEnd().IsEmpty()
            || KindValue > static_cast<uint8>(EPinkCabCityDeltaKind::Incident)
            || DeltaIds.Contains(Record.DeltaId))
        {
            return false;
        }
        DeltaIds.Add(Record.DeltaId);
    }
    return true;
}

bool FPinkCabWorldSessionSnapshotCodec::ValidateWorkday(
    const FPinkCabWorkdaySessionSnapshot& Snapshot)
{
    if (Snapshot.SchemaVersion
            != FPinkCabWorkdaySessionSnapshot::CurrentSchemaVersion
        || Snapshot.Ordinal < 0
        || !FMath::IsFinite(Snapshot.ElapsedGameSeconds)
        || Snapshot.ElapsedGameSeconds < 0.0
        || Snapshot.MaxReplayJournalEntries <= 0
        || Snapshot.HouseholdTransactionCount < 0
        || Snapshot.AppliedHouseholdOperationIds.Num()
            > Snapshot.MaxReplayJournalEntries
        || Snapshot.HouseholdTransactionCount
            != Snapshot.AppliedHouseholdOperationIds.Num())
    {
        return false;
    }

    FPinkCabStableId ParsedWorkday;
    if (!FPinkCabStableId::TryParse(
            Snapshot.WorkdayId,
            ParsedWorkday))
    {
        return false;
    }

    TSet<FString> UniqueIds;
    for (const FString& Id : Snapshot.AppliedHouseholdOperationIds)
    {
        FPinkCabStableId ParsedId;
        if (!FPinkCabStableId::TryParse(Id, ParsedId)
            || UniqueIds.Contains(ParsedId.Serialize()))
        {
            return false;
        }
        UniqueIds.Add(ParsedId.Serialize());
    }
    return true;
}
