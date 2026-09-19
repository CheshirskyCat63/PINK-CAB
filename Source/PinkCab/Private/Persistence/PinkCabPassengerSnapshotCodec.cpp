#include "Persistence/PinkCabPassengerSnapshotCodec.h"

bool FPinkCabPassengerSnapshotCodec::Capture(
    const FPinkCabPassengerRegistry& Registry,
    FPinkCabPassengerSnapshot& OutSnapshot)
{
    FPinkCabPassengerSnapshot Snapshot;
    Snapshot.MaxRecords = Registry.MaxRecords;
    Snapshot.MaxPreferences = Registry.MaxPreferences;
    Snapshot.MaxRideMemories = Registry.MaxRideMemories;
    Snapshot.MaxReplayJournalEntries = Registry.MaxReplayJournalEntries;
    Snapshot.MaxNeuralMessages = Registry.MaxNeuralMessages;
    Snapshot.MaxNeuralReplayJournalEntries = Registry.MaxNeuralReplayJournalEntries;
    for (const TPair<FString, FPinkCabPassengerRecord>& Pair : Registry.Records)
    {
        Snapshot.Records.Add(Pair.Value);
    }
    Snapshot.Records.Sort([](const FPinkCabPassengerRecord& A, const FPinkCabPassengerRecord& B)
    {
        return A.IdentityId.Serialize() < B.IdentityId.Serialize();
    });
    if (!Validate(Snapshot)) return false;
    OutSnapshot = MoveTemp(Snapshot);
    return true;
}

bool FPinkCabPassengerSnapshotCodec::Restore(
    const FPinkCabPassengerSnapshot& Snapshot,
    FPinkCabPassengerRegistry& OutRegistry)
{
    if (!Validate(Snapshot)) return false;
    FPinkCabPassengerRegistry Restored(
        Snapshot.MaxRecords,
        Snapshot.MaxPreferences,
        Snapshot.MaxRideMemories,
        Snapshot.MaxReplayJournalEntries,
        Snapshot.MaxNeuralMessages,
        Snapshot.MaxNeuralReplayJournalEntries);
    for (const FPinkCabPassengerRecord& Record : Snapshot.Records)
    {
        Restored.Records.Add(Record.IdentityId.Serialize(), Record);
    }
    OutRegistry = MoveTemp(Restored);
    return true;
}

bool FPinkCabPassengerSnapshotCodec::ToPersistedIdentity(
    const FPinkCabPassengerRecord& Record,
    FPinkCabPersistedPassengerIdentity& OutPersisted)
{
    if (!PinkCabPassengerRecordIsValid(Record)) return false;
    FPinkCabPersistedPassengerIdentity Persisted;
    Persisted.IdentityId = Record.IdentityId.Serialize();
    Persisted.TemplateId = Record.TemplateId.ToString();
    Persisted.IdentitySeed = Record.IdentitySeed;
    Persisted.AppearanceSeed = Record.AppearanceSeed;
    Persisted.AppearanceProfileId = Record.AppearanceProfileId.ToString();
    Persisted.Trust = Record.Relationship.Trust;
    Persisted.Satisfaction = Record.Relationship.Satisfaction;
    Persisted.RiskTolerance = Record.Relationship.RiskTolerance;
    Persisted.PaidFareCount = Record.PaidFareCount;
    Persisted.AuthoredEventCount = Record.AuthoredEventCount;
    Persisted.bRepeatEligible = Record.bRepeatEligible;
    Persisted.bNeuralPermissionGranted = Record.bNeuralPermissionGranted;
    Persisted.bNeuralBlocked = Record.bNeuralBlocked;
    OutPersisted = MoveTemp(Persisted);
    return true;
}

bool FPinkCabPassengerSnapshotCodec::ValidateStableIdStrings(const TArray<FString>& Ids, int32 MaxEntries)
{
    if (MaxEntries <= 0 || Ids.Num() > MaxEntries) return false;
    TSet<FString> Unique;
    for (const FString& Id : Ids)
    {
        FPinkCabStableId Parsed;
        if (!FPinkCabStableId::TryParse(Id, Parsed) || Unique.Contains(Id)) return false;
        Unique.Add(Id);
    }
    return true;
}

bool FPinkCabPassengerSnapshotCodec::ValidateRelationship(const FPinkCabPassengerRelationship& Relationship)
{
    return FMath::IsFinite(Relationship.Trust)
        && FMath::IsFinite(Relationship.Satisfaction)
        && FMath::IsFinite(Relationship.RiskTolerance)
        && FMath::Abs(Relationship.Trust) <= 1.0f
        && FMath::Abs(Relationship.Satisfaction) <= 1.0f
        && FMath::Abs(Relationship.RiskTolerance) <= 1.0f;
}

bool FPinkCabPassengerSnapshotCodec::ValidateCanonicalAppearance(const FPinkCabPassengerRecord& Record)
{
    const uint64 Root = FPinkCabDeterministicSeed::FromText(
        Record.ContextKey + TEXT(":") + Record.TemplateId.ToString());
    const uint64 ExpectedIdentitySeed = FPinkCabDeterministicSeed::Derive(
        Root, Record.IdentityId, TEXT("passenger-record"));
    const uint64 ExpectedAppearanceSeed = FPinkCabDeterministicSeed::Derive(
        ExpectedIdentitySeed, Record.IdentityId, TEXT("passenger-appearance"));
    if (Record.IdentitySeed != ExpectedIdentitySeed
        || Record.AppearanceSeed != ExpectedAppearanceSeed)
        return false;
    const FName ExpectedProfile(*FString::Printf(
        TEXT("profile.%s.%02llu"), *Record.TemplateId.ToString(),
        ExpectedAppearanceSeed % 16ull));
    if (Record.AppearanceProfileId != ExpectedProfile) return false;
    const TArray<FName> ExpectedTraits = {
        FName(*FString::Printf(TEXT("body.%02llu"), (ExpectedAppearanceSeed >> 8) % 16ull)),
        FName(*FString::Printf(TEXT("face.%02llu"), (ExpectedAppearanceSeed >> 16) % 32ull)),
        FName(*FString::Printf(TEXT("hair.%02llu"), (ExpectedAppearanceSeed >> 24) % 32ull)),
        FName(*FString::Printf(TEXT("palette.%02llu"), (ExpectedAppearanceSeed >> 32) % 16ull))};
    return Record.AppearanceTraitIds == ExpectedTraits;
}

bool FPinkCabPassengerSnapshotCodec::ValidatePreferences(
    const FPinkCabPassengerRecord& Record,
    const FPinkCabPassengerSnapshot& Snapshot)
{
    if (Record.PreferenceTags.Num() > Snapshot.MaxPreferences) return false;
    TSet<FName> Preferences;
    for (const FName Tag : Record.PreferenceTags)
    {
        if (Tag.IsNone() || Preferences.Contains(Tag)) return false;
        Preferences.Add(Tag);
    }
    return true;
}

bool FPinkCabPassengerSnapshotCodec::ValidateCounters(
    const FPinkCabPassengerRecord& Record,
    const FPinkCabPassengerSnapshot& Snapshot)
{
    if (Record.MaxRideMemories != Snapshot.MaxRideMemories
        || Record.MaxReplayJournalEntries != Snapshot.MaxReplayJournalEntries
        || Record.MaxNeuralMessages != Snapshot.MaxNeuralMessages
        || Record.MaxNeuralReplayJournalEntries != Snapshot.MaxNeuralReplayJournalEntries)
    {
        return false;
    }
    if (Record.PaidFareCount < 0 || Record.AuthoredEventCount < 0) return false;
    const bool bExpectedRepeat = Record.PaidFareCount >= 2 || Record.AuthoredEventCount >= 1;
    return Record.bRepeatEligible == bExpectedRepeat;
}

bool FPinkCabPassengerSnapshotCodec::ValidateRideMemoryState(
    const FPinkCabPassengerRecord& Record,
    const FPinkCabPassengerSnapshot& Snapshot)
{
    if (Record.RideMemories.Num() > Snapshot.MaxRideMemories
        || !ValidateStableIdStrings(Record.AppliedSocialEventIds, Snapshot.MaxReplayJournalEntries))
    {
        return false;
    }
    TSet<FString> MemoryIds;
    for (const FPinkCabPassengerRideMemory& Memory : Record.RideMemories)
    {
        if (!Memory.MemoryId.IsValid() || !Memory.FareId.IsValid() || Memory.OutcomeTag.IsNone())
            return false;
        const FString MemoryKey = Memory.MemoryId.Serialize();
        if (MemoryIds.Contains(MemoryKey) || !Record.AppliedSocialEventIds.Contains(MemoryKey))
            return false;
        MemoryIds.Add(MemoryKey);
    }
    return true;
}

bool FPinkCabPassengerSnapshotCodec::ValidateNeuralState(
    const FPinkCabPassengerRecord& Record,
    const FPinkCabPassengerSnapshot& Snapshot)
{
    if (Record.NeuralMessages.Num() > Snapshot.MaxNeuralMessages
        || !ValidateStableIdStrings(
            Record.AppliedNeuralMessageIds, Snapshot.MaxNeuralReplayJournalEntries))
    {
        return false;
    }
    TSet<FString> MessageIds;
    for (const FPinkCabPassengerNeuralMessage& Message : Record.NeuralMessages)
    {
        const FString MessageKey = Message.MessageId.Serialize();
        if (!Message.MessageId.IsValid()
            || Message.Text.TrimStartAndEnd().IsEmpty()
            || MessageIds.Contains(MessageKey)
            || !Record.AppliedNeuralMessageIds.Contains(MessageKey))
        {
            return false;
        }
        MessageIds.Add(MessageKey);
    }
    return true;
}

bool FPinkCabPassengerSnapshotCodec::ValidateRecord(
    const FPinkCabPassengerRecord& Record,
    const FPinkCabPassengerSnapshot& Snapshot)
{
    return PinkCabPassengerRecordIsValid(Record)
        && Record.ContextKey == Record.ContextKey.TrimStartAndEnd()
        && ValidateRelationship(Record.Relationship)
        && ValidateCanonicalAppearance(Record)
        && ValidatePreferences(Record, Snapshot)
        && ValidateCounters(Record, Snapshot)
        && ValidateRideMemoryState(Record, Snapshot)
        && ValidateNeuralState(Record, Snapshot);
}

bool FPinkCabPassengerSnapshotCodec::Validate(const FPinkCabPassengerSnapshot& Snapshot)
{
    if (Snapshot.SchemaVersion != FPinkCabPassengerSnapshot::CurrentSchemaVersion)
        return false;
    if (Snapshot.MaxRecords <= 0 || Snapshot.MaxPreferences < 0
        || Snapshot.MaxRideMemories <= 0 || Snapshot.MaxReplayJournalEntries <= 0
        || Snapshot.MaxNeuralMessages <= 0 || Snapshot.MaxNeuralReplayJournalEntries <= 0
        || Snapshot.Records.Num() > Snapshot.MaxRecords)
        return false;
    TSet<FString> IdentityIds;
    for (const FPinkCabPassengerRecord& Record : Snapshot.Records)
    {
        const FString Key = Record.IdentityId.Serialize();
        if (IdentityIds.Contains(Key) || !ValidateRecord(Record, Snapshot)) return false;
        IdentityIds.Add(Key);
    }
    return true;
}
