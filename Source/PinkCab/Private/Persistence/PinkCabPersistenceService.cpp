#include "Persistence/PinkCabPersistenceService.h"

bool FPinkCabPersistenceService::Serialize(
    const FPinkCabSaveHeader& Header,
    const FPinkCabGameSnapshot& State,
    TArray<uint8>& OutBytes)
{
    return FPinkCabPersistenceBinaryCodec::Serialize(
        Header,
        State,
        OutBytes);
}

EPinkCabLoadResult FPinkCabPersistenceService::Deserialize(
    const TArray<uint8>& Bytes,
    const FPinkCabSaveHeader& ExpectedHeader,
    const FPinkCabMigrationRegistry& Registry,
    FPinkCabGameSnapshot& OutState)
{
    return FPinkCabPersistenceBinaryCodec::Deserialize(
        Bytes,
        ExpectedHeader,
        Registry,
        OutState);
}

EPinkCabLoadResult FPinkCabPersistenceService::DeserializeInto(
    const TArray<uint8>& Bytes,
    const FPinkCabSaveHeader& ExpectedHeader,
    const FPinkCabMigrationRegistry& Registry,
    FPinkCabCityIdentity& OutCityIdentity,
    FPinkCabGamePersistenceOwners& Owners)
{
    return FPinkCabPersistenceBinaryCodec::DeserializeInto(
        Bytes,
        ExpectedHeader,
        Registry,
        OutCityIdentity,
        Owners);
}

bool FPinkCabPersistenceService::Serialize(
    const FPinkCabSaveHeader& Header,
    const FPinkCabLogicalSaveState& State,
    TArray<uint8>& OutBytes)
{
    return FPinkCabPersistenceBinaryCodec::Serialize(
        Header,
        State,
        OutBytes);
}

EPinkCabLoadResult FPinkCabPersistenceService::Deserialize(
    const TArray<uint8>& Bytes,
    const FPinkCabSaveHeader& ExpectedHeader,
    const FPinkCabMigrationRegistry& Registry,
    FPinkCabLogicalSaveState& OutState)
{
    return FPinkCabPersistenceBinaryCodec::Deserialize(
        Bytes,
        ExpectedHeader,
        Registry,
        OutState);
}

bool FPinkCabPersistenceService::CommitSnapshot(
    const FPinkCabSaveHeader& Header,
    const FPinkCabGameSnapshot& State,
    EPinkCabPersistenceCommitReason Reason)
{
    TArray<uint8> Bytes;
    if (!Serialize(Header, State, Bytes))
    {
        return false;
    }
    return CommitBytes(MoveTemp(Bytes), Reason);
}

bool FPinkCabPersistenceService::CommitSnapshot(
    const FPinkCabSaveHeader& Header,
    const FPinkCabLogicalSaveState& State,
    EPinkCabPersistenceCommitReason Reason)
{
    TArray<uint8> Bytes;
    if (!Serialize(Header, State, Bytes))
    {
        return false;
    }
    return CommitBytes(MoveTemp(Bytes), Reason);
}

bool FPinkCabPersistenceService::CommitBytes(
    TArray<uint8>&& Bytes,
    EPinkCabPersistenceCommitReason Reason)
{
    LastCommittedBytes = MoveTemp(Bytes);
    LastCommitReason = Reason;
    Checkpoints.Push(LastCommittedBytes);
    return true;
}

EPinkCabLoadResult
FPinkCabPersistenceService::RecoverLastCommittedInto(
    const FPinkCabSaveHeader& ExpectedHeader,
    const FPinkCabMigrationRegistry& Registry,
    FPinkCabCityIdentity& OutCityIdentity,
    FPinkCabGamePersistenceOwners& Owners) const
{
    if (LastCommittedBytes.Num() == 0)
    {
        return EPinkCabLoadResult::NoCommittedSnapshot;
    }
    return DeserializeInto(
        LastCommittedBytes,
        ExpectedHeader,
        Registry,
        OutCityIdentity,
        Owners);
}

EPinkCabLoadResult
FPinkCabPersistenceService::RecoverCheckpointInto(
    int32 NewestOffset,
    const FPinkCabSaveHeader& ExpectedHeader,
    const FPinkCabMigrationRegistry& Registry,
    FPinkCabCityIdentity& OutCityIdentity,
    FPinkCabGamePersistenceOwners& Owners) const
{
    const TArray<uint8>* Bytes =
        Checkpoints.GetFromNewestOffset(NewestOffset);
    if (!Bytes)
    {
        return EPinkCabLoadResult::NoCommittedSnapshot;
    }
    return DeserializeInto(
        *Bytes,
        ExpectedHeader,
        Registry,
        OutCityIdentity,
        Owners);
}

EPinkCabLoadResult
FPinkCabPersistenceService::RecoverLastCommitted(
    const FPinkCabSaveHeader& ExpectedHeader,
    const FPinkCabMigrationRegistry& Registry,
    FPinkCabGameSnapshot& OutState) const
{
    if (LastCommittedBytes.Num() == 0)
    {
        return EPinkCabLoadResult::NoCommittedSnapshot;
    }
    return Deserialize(
        LastCommittedBytes,
        ExpectedHeader,
        Registry,
        OutState);
}

EPinkCabLoadResult
FPinkCabPersistenceService::RecoverCheckpoint(
    int32 NewestOffset,
    const FPinkCabSaveHeader& ExpectedHeader,
    const FPinkCabMigrationRegistry& Registry,
    FPinkCabGameSnapshot& OutState) const
{
    const TArray<uint8>* Bytes =
        Checkpoints.GetFromNewestOffset(NewestOffset);
    if (!Bytes)
    {
        return EPinkCabLoadResult::NoCommittedSnapshot;
    }
    return Deserialize(
        *Bytes,
        ExpectedHeader,
        Registry,
        OutState);
}

EPinkCabLoadResult
FPinkCabPersistenceService::RecoverLastCommitted(
    const FPinkCabSaveHeader& ExpectedHeader,
    const FPinkCabMigrationRegistry& Registry,
    FPinkCabLogicalSaveState& OutState) const
{
    if (LastCommittedBytes.Num() == 0)
    {
        return EPinkCabLoadResult::NoCommittedSnapshot;
    }
    return Deserialize(
        LastCommittedBytes,
        ExpectedHeader,
        Registry,
        OutState);
}

EPinkCabPersistenceCommitReason
FPinkCabPersistenceService::GetLastCommitReason() const
{
    return LastCommitReason;
}

const FPinkCabCheckpointRing&
FPinkCabPersistenceService::GetCheckpointRing() const
{
    return Checkpoints;
}
