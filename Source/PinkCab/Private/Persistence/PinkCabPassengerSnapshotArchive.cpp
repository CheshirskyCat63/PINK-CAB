#include "Persistence/PinkCabPassengerSnapshotArchive.h"

void FPinkCabPassengerSnapshotArchive::Serialize(
    FArchive& Ar,
    FPinkCabPassengerSnapshot& Snapshot)
{
    SerializePassengerSnapshot(Ar, Snapshot);
}

void FPinkCabPassengerSnapshotArchive::SerializeRelationship(
    FArchive& Ar,
    FPinkCabPassengerRelationship& Relationship)
{
    Ar << Relationship.Trust;
    Ar << Relationship.Satisfaction;
    Ar << Relationship.RiskTolerance;
}

void FPinkCabPassengerSnapshotArchive::SerializeRideMemory(
    FArchive& Ar,
    FPinkCabPassengerRideMemory& Memory)
{
    SerializeStableId(Ar, Memory.MemoryId);
    SerializeStableId(Ar, Memory.FareId);
    Ar << Memory.OutcomeTag;
}

void FPinkCabPassengerSnapshotArchive::SerializeNeuralMessage(
    FArchive& Ar,
    FPinkCabPassengerNeuralMessage& Message)
{
    SerializeStableId(Ar, Message.MessageId);
    Ar << Message.Text;
}

void FPinkCabPassengerSnapshotArchive::SerializePassengerRecord(
    FArchive& Ar,
    FPinkCabPassengerRecord& Record)
{
    SerializeStableId(Ar, Record.IdentityId);
    Ar << Record.TemplateId;
    Ar << Record.ContextKey;
    Ar << Record.IdentitySeed;
    Ar << Record.AppearanceSeed;
    Ar << Record.AppearanceProfileId;
    SerializeArray(
        Ar,
        Record.AppearanceTraitIds,
        64,
        [](FArchive& A, FName& Value)
        {
            A << Value;
        });
    Ar << Record.ResolvedMassKg;
    SerializeArray(
        Ar,
        Record.PreferenceTags,
        256,
        [](FArchive& A, FName& Value)
        {
            A << Value;
        });
    SerializeRelationship(Ar, Record.Relationship);
    Ar << Record.MaxRideMemories;
    Ar << Record.MaxReplayJournalEntries;
    Ar << Record.PaidFareCount;
    Ar << Record.AuthoredEventCount;
    Ar << Record.bRepeatEligible;
    SerializeArray(
        Ar,
        Record.RideMemories,
        MaxArrayElements,
        [](FArchive& A, FPinkCabPassengerRideMemory& Value)
        {
            SerializeRideMemory(A, Value);
        });
    SerializeArray(
        Ar,
        Record.AppliedSocialEventIds,
        MaxArrayElements,
        [](FArchive& A, FString& Value)
        {
            A << Value;
        });
    Ar << Record.MaxNeuralMessages;
    Ar << Record.MaxNeuralReplayJournalEntries;
    Ar << Record.bNeuralPermissionGranted;
    Ar << Record.bNeuralBlocked;
    SerializeArray(
        Ar,
        Record.NeuralMessages,
        MaxArrayElements,
        [](FArchive& A, FPinkCabPassengerNeuralMessage& Value)
        {
            SerializeNeuralMessage(A, Value);
        });
    SerializeArray(
        Ar,
        Record.AppliedNeuralMessageIds,
        MaxArrayElements,
        [](FArchive& A, FString& Value)
        {
            A << Value;
        });

    if (!IsCapacityReasonable(Record.MaxRideMemories)
        || !IsCapacityReasonable(Record.MaxReplayJournalEntries)
        || !IsCapacityReasonable(Record.MaxNeuralMessages)
        || !IsCapacityReasonable(
            Record.MaxNeuralReplayJournalEntries))
    {
        Ar.SetError();
    }
}

void FPinkCabPassengerSnapshotArchive::SerializePassengerSnapshot(
    FArchive& Ar,
    FPinkCabPassengerSnapshot& Snapshot)
{
    Ar << Snapshot.SchemaVersion;
    Ar << Snapshot.MaxRecords;
    Ar << Snapshot.MaxPreferences;
    Ar << Snapshot.MaxRideMemories;
    Ar << Snapshot.MaxReplayJournalEntries;
    Ar << Snapshot.MaxNeuralMessages;
    Ar << Snapshot.MaxNeuralReplayJournalEntries;

    if (!IsCapacityReasonable(Snapshot.MaxRecords)
        || !IsCapacityReasonable(Snapshot.MaxPreferences)
        || !IsCapacityReasonable(Snapshot.MaxRideMemories)
        || !IsCapacityReasonable(Snapshot.MaxReplayJournalEntries)
        || !IsCapacityReasonable(Snapshot.MaxNeuralMessages)
        || !IsCapacityReasonable(
            Snapshot.MaxNeuralReplayJournalEntries))
    {
        Ar.SetError();
        return;
    }

    SerializeArray(
        Ar,
        Snapshot.Records,
        Snapshot.MaxRecords,
        [](FArchive& A, FPinkCabPassengerRecord& Value)
        {
            SerializePassengerRecord(A, Value);
        });
}
