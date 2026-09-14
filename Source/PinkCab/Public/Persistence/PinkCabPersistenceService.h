#pragma once

#include "CoreMinimal.h"
#include "Persistence/PinkCabPersistenceTypes.h"
#include "Persistence/PinkCabCheckpointRing.h"
#include "Persistence/PinkCabPersistenceBinaryCodec.h"

class FPinkCabPersistenceService
{
public:
    static bool Serialize(const FPinkCabSaveHeader& Header, const FPinkCabGameSnapshot& State, TArray<uint8>& OutBytes)
    { return FPinkCabPersistenceBinaryCodec::Serialize(Header, State, OutBytes); }

    static EPinkCabLoadResult Deserialize(const TArray<uint8>& Bytes, const FPinkCabSaveHeader& ExpectedHeader, const FPinkCabMigrationRegistry& Registry, FPinkCabGameSnapshot& OutState)
    { return FPinkCabPersistenceBinaryCodec::Deserialize(Bytes, ExpectedHeader, Registry, OutState); }

    static EPinkCabLoadResult DeserializeInto(const TArray<uint8>& Bytes, const FPinkCabSaveHeader& ExpectedHeader, const FPinkCabMigrationRegistry& Registry, FPinkCabCityIdentity& OutCityIdentity, FPinkCabGamePersistenceOwners& Owners)
    { return FPinkCabPersistenceBinaryCodec::DeserializeInto(Bytes, ExpectedHeader, Registry, OutCityIdentity, Owners); }

    static bool Serialize(const FPinkCabSaveHeader& Header, const FPinkCabLogicalSaveState& State, TArray<uint8>& OutBytes)
    { return FPinkCabPersistenceBinaryCodec::Serialize(Header, State, OutBytes); }

    static EPinkCabLoadResult Deserialize(const TArray<uint8>& Bytes, const FPinkCabSaveHeader& ExpectedHeader, const FPinkCabMigrationRegistry& Registry, FPinkCabLogicalSaveState& OutState)
    { return FPinkCabPersistenceBinaryCodec::Deserialize(Bytes, ExpectedHeader, Registry, OutState); }

    bool CommitSnapshot(
        const FPinkCabSaveHeader& Header,
        const FPinkCabGameSnapshot& State,
        EPinkCabPersistenceCommitReason Reason)
    {
        TArray<uint8> Bytes;
        if (!Serialize(Header, State, Bytes))
        {
            return false;
        }
        LastCommittedBytes = MoveTemp(Bytes);
        LastCommitReason = Reason;
        Checkpoints.Push(LastCommittedBytes);
        return true;
    }
    bool CommitSnapshot(
        const FPinkCabSaveHeader& Header,
        const FPinkCabLogicalSaveState& State,
        EPinkCabPersistenceCommitReason Reason)
    {
        TArray<uint8> Bytes;
        if (!Serialize(Header, State, Bytes))
        {
            return false;
        }
        LastCommittedBytes = MoveTemp(Bytes);
        LastCommitReason = Reason;
        Checkpoints.Push(LastCommittedBytes);
        return true;
    }

    EPinkCabLoadResult RecoverLastCommittedInto(
        const FPinkCabSaveHeader& ExpectedHeader,
        const FPinkCabMigrationRegistry& Registry,
        FPinkCabCityIdentity& OutCityIdentity,
        FPinkCabGamePersistenceOwners& Owners) const
    {
        if (LastCommittedBytes.Num() == 0)
        {
            return EPinkCabLoadResult::NoCommittedSnapshot;
        }
        return DeserializeInto(LastCommittedBytes, ExpectedHeader, Registry, OutCityIdentity, Owners);
    }

    EPinkCabLoadResult RecoverCheckpointInto(
        int32 NewestOffset,
        const FPinkCabSaveHeader& ExpectedHeader,
        const FPinkCabMigrationRegistry& Registry,
        FPinkCabCityIdentity& OutCityIdentity,
        FPinkCabGamePersistenceOwners& Owners) const
    {
        const TArray<uint8>* Bytes = Checkpoints.GetFromNewestOffset(NewestOffset);
        if (!Bytes)
        {
            return EPinkCabLoadResult::NoCommittedSnapshot;
        }
        return DeserializeInto(*Bytes, ExpectedHeader, Registry, OutCityIdentity, Owners);
    }
    EPinkCabLoadResult RecoverLastCommitted(
        const FPinkCabSaveHeader& ExpectedHeader,
        const FPinkCabMigrationRegistry& Registry,
        FPinkCabGameSnapshot& OutState) const
    {
        if (LastCommittedBytes.Num() == 0)
        {
            return EPinkCabLoadResult::NoCommittedSnapshot;
        }
        return Deserialize(LastCommittedBytes, ExpectedHeader, Registry, OutState);
    }

    EPinkCabLoadResult RecoverCheckpoint(
        int32 NewestOffset,
        const FPinkCabSaveHeader& ExpectedHeader,
        const FPinkCabMigrationRegistry& Registry,
        FPinkCabGameSnapshot& OutState) const
    {
        const TArray<uint8>* Bytes = Checkpoints.GetFromNewestOffset(NewestOffset);
        if (!Bytes)
        {
            return EPinkCabLoadResult::NoCommittedSnapshot;
        }
        return Deserialize(*Bytes, ExpectedHeader, Registry, OutState);
    }
    EPinkCabLoadResult RecoverLastCommitted(
        const FPinkCabSaveHeader& ExpectedHeader,
        const FPinkCabMigrationRegistry& Registry,
        FPinkCabLogicalSaveState& OutState) const
    {
        if (LastCommittedBytes.Num() == 0)
        {
            return EPinkCabLoadResult::NoCommittedSnapshot;
        }
        return Deserialize(LastCommittedBytes, ExpectedHeader, Registry, OutState);
    }

    EPinkCabPersistenceCommitReason GetLastCommitReason() const { return LastCommitReason; }
    const FPinkCabCheckpointRing& GetCheckpointRing() const { return Checkpoints; }

private:
    TArray<uint8> LastCommittedBytes;
    FPinkCabCheckpointRing Checkpoints;
    EPinkCabPersistenceCommitReason LastCommitReason = EPinkCabPersistenceCommitReason::PeriodicCheckpoint;
};
