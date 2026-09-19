#pragma once

#include "CoreMinimal.h"
#include "Persistence/PinkCabPersistenceTypes.h"
#include "Persistence/PinkCabCheckpointRing.h"
#include "Persistence/PinkCabPersistenceBinaryCodec.h"

class PINKCAB_API FPinkCabPersistenceService
{
public:
    static bool Serialize(
        const FPinkCabSaveHeader& Header,
        const FPinkCabGameSnapshot& State,
        TArray<uint8>& OutBytes);
    static EPinkCabLoadResult Deserialize(
        const TArray<uint8>& Bytes,
        const FPinkCabSaveHeader& ExpectedHeader,
        const FPinkCabMigrationRegistry& Registry,
        FPinkCabGameSnapshot& OutState);
    static EPinkCabLoadResult DeserializeInto(
        const TArray<uint8>& Bytes,
        const FPinkCabSaveHeader& ExpectedHeader,
        const FPinkCabMigrationRegistry& Registry,
        FPinkCabCityIdentity& OutCityIdentity,
        FPinkCabGamePersistenceOwners& Owners);

    static bool Serialize(
        const FPinkCabSaveHeader& Header,
        const FPinkCabLogicalSaveState& State,
        TArray<uint8>& OutBytes);
    static EPinkCabLoadResult Deserialize(
        const TArray<uint8>& Bytes,
        const FPinkCabSaveHeader& ExpectedHeader,
        const FPinkCabMigrationRegistry& Registry,
        FPinkCabLogicalSaveState& OutState);

    bool CommitSnapshot(
        const FPinkCabSaveHeader& Header,
        const FPinkCabGameSnapshot& State,
        EPinkCabPersistenceCommitReason Reason);
    bool CommitSnapshot(
        const FPinkCabSaveHeader& Header,
        const FPinkCabLogicalSaveState& State,
        EPinkCabPersistenceCommitReason Reason);

    EPinkCabLoadResult RecoverLastCommittedInto(
        const FPinkCabSaveHeader& ExpectedHeader,
        const FPinkCabMigrationRegistry& Registry,
        FPinkCabCityIdentity& OutCityIdentity,
        FPinkCabGamePersistenceOwners& Owners) const;
    EPinkCabLoadResult RecoverCheckpointInto(
        int32 NewestOffset,
        const FPinkCabSaveHeader& ExpectedHeader,
        const FPinkCabMigrationRegistry& Registry,
        FPinkCabCityIdentity& OutCityIdentity,
        FPinkCabGamePersistenceOwners& Owners) const;

    EPinkCabLoadResult RecoverLastCommitted(
        const FPinkCabSaveHeader& ExpectedHeader,
        const FPinkCabMigrationRegistry& Registry,
        FPinkCabGameSnapshot& OutState) const;
    EPinkCabLoadResult RecoverCheckpoint(
        int32 NewestOffset,
        const FPinkCabSaveHeader& ExpectedHeader,
        const FPinkCabMigrationRegistry& Registry,
        FPinkCabGameSnapshot& OutState) const;
    EPinkCabLoadResult RecoverLastCommitted(
        const FPinkCabSaveHeader& ExpectedHeader,
        const FPinkCabMigrationRegistry& Registry,
        FPinkCabLogicalSaveState& OutState) const;

    EPinkCabPersistenceCommitReason GetLastCommitReason() const;
    const FPinkCabCheckpointRing& GetCheckpointRing() const;

private:
    bool CommitBytes(
        TArray<uint8>&& Bytes,
        EPinkCabPersistenceCommitReason Reason);

    TArray<uint8> LastCommittedBytes;
    FPinkCabCheckpointRing Checkpoints;
    EPinkCabPersistenceCommitReason LastCommitReason =
        EPinkCabPersistenceCommitReason::PeriodicCheckpoint;
};
