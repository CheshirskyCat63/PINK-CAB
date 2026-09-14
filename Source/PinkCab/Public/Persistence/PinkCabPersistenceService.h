#pragma once

#include "CoreMinimal.h"
#include "Serialization/MemoryReader.h"
#include "Serialization/MemoryWriter.h"
#include "Persistence/PinkCabSaveHeader.h"
#include "Persistence/PinkCabMigrationRegistry.h"
#include "Persistence/PinkCabPersistedLogicalState.h"
#include "Persistence/PinkCabGameSnapshotArchive.h"
#include "Persistence/PinkCabGamePersistenceCoordinator.h"

enum class EPinkCabLoadResult : uint8
{
    Success,
    CorruptPayload,
    IncompatibleProduct,
    IncompatibleSchema,
    IncompatibleConfig,
    IncompatibleWorldVersion,
    MigrationRequired,
    NoCommittedSnapshot
};

enum class EPinkCabPersistenceCommitReason : uint8
{
    FareSettlement,
    ServiceSettlement,
    RefuelSettlement,
    RepeatClientPromotion,
    WorkdayEnd,
    PersistentWorldChange,
    PeriodicCheckpoint,
    TerminalRecovery
};

class FPinkCabCheckpointRing
{
public:
    void Push(const TArray<uint8>& Bytes)
    {
        Checkpoints.Add(Bytes);
        while (Checkpoints.Num() > FPinkCabPersistencePolicy::RollingCheckpointCount)
        {
            Checkpoints.RemoveAt(0);
        }
    }

    int32 Num() const { return Checkpoints.Num(); }

    const TArray<uint8>& GetOldest() const
    {
        return Checkpoints[0];
    }

    const TArray<uint8>& GetNewest() const
    {
        return Checkpoints.Last();
    }

    const TArray<uint8>* GetFromNewestOffset(int32 Offset) const
    {
        if (Offset < 0 || Offset >= Checkpoints.Num())
        {
            return nullptr;
        }
        return &Checkpoints[Checkpoints.Num() - 1 - Offset];
    }
private:
    TArray<TArray<uint8>> Checkpoints;
};

class FPinkCabPersistenceService
{
public:
    static bool Serialize(
        const FPinkCabSaveHeader& Header,
        const FPinkCabGameSnapshot& State,
        TArray<uint8>& OutBytes)
    {
        OutBytes.Reset();
        FMemoryWriter Writer(OutBytes, true);
        uint32 Magic = AggregatePayloadMagic;
        Writer << Magic;
        FPinkCabSaveHeader MutableHeader = Header;
        Writer << MutableHeader.ProductName;
        Writer << MutableHeader.SchemaVersion;
        Writer << MutableHeader.ConfigVersion;
        Writer << MutableHeader.GeneratorVersion;
        Writer << MutableHeader.ContentSetVersion;
        FPinkCabGameSnapshot MutableState = State;
        if (!FPinkCabGameSnapshotArchive::Serialize(Writer, MutableState))
        {
            return false;
        }
        Writer.Close();
        return !Writer.IsError() && OutBytes.Num() > 0;
    }

    static EPinkCabLoadResult Deserialize(
        const TArray<uint8>& Bytes,
        const FPinkCabSaveHeader& ExpectedHeader,
        const FPinkCabMigrationRegistry& Registry,
        FPinkCabGameSnapshot& OutState)
    {
        if (Bytes.Num() == 0)
        {
            return EPinkCabLoadResult::CorruptPayload;
        }
        FMemoryReader Reader(Bytes, true);
        uint32 Magic = 0;
        Reader << Magic;
        if (Magic != AggregatePayloadMagic)
        {
            return EPinkCabLoadResult::CorruptPayload;
        }
        FPinkCabSaveHeader SavedHeader;
        Reader << SavedHeader.ProductName;
        Reader << SavedHeader.SchemaVersion;
        Reader << SavedHeader.ConfigVersion;
        Reader << SavedHeader.GeneratorVersion;
        Reader << SavedHeader.ContentSetVersion;
        const EPinkCabLoadResult Compatibility = CheckCompatibility(
            SavedHeader, ExpectedHeader, Registry);
        if (Compatibility != EPinkCabLoadResult::Success)
        {
            return Compatibility;
        }
        FPinkCabGameSnapshot Restored;
        if (!FPinkCabGameSnapshotArchive::Serialize(Reader, Restored)
            || Reader.IsError() || Reader.Tell() != Reader.TotalSize())
        {
            return EPinkCabLoadResult::CorruptPayload;
        }
        OutState = MoveTemp(Restored);
        return EPinkCabLoadResult::Success;
    }
    static EPinkCabLoadResult DeserializeInto(
        const TArray<uint8>& Bytes,
        const FPinkCabSaveHeader& ExpectedHeader,
        const FPinkCabMigrationRegistry& Registry,
        FPinkCabCityIdentity& OutCityIdentity,
        FPinkCabGamePersistenceOwners& Owners)
    {
        FPinkCabGameSnapshot Snapshot;
        const EPinkCabLoadResult Result = Deserialize(Bytes, ExpectedHeader, Registry, Snapshot);
        if (Result != EPinkCabLoadResult::Success)
        {
            return Result;
        }
        FPinkCabCityIdentity RestoredCity;
        if (!FPinkCabGamePersistenceCoordinator::Restore(Snapshot, RestoredCity, Owners))
        {
            return EPinkCabLoadResult::CorruptPayload;
        }
        OutCityIdentity = MoveTemp(RestoredCity);
        return EPinkCabLoadResult::Success;
    }
    static bool Serialize(
        const FPinkCabSaveHeader& Header,
        const FPinkCabLogicalSaveState& State,
        TArray<uint8>& OutBytes)
    {
        OutBytes.Reset();
        FMemoryWriter Writer(OutBytes, true);
        uint32 Magic = PayloadMagic;
        Writer << Magic;

        FPinkCabSaveHeader MutableHeader = Header;
        Writer << MutableHeader.ProductName;
        Writer << MutableHeader.SchemaVersion;
        Writer << MutableHeader.ConfigVersion;
        Writer << MutableHeader.GeneratorVersion;
        Writer << MutableHeader.ContentSetVersion;

        FPinkCabLogicalSaveState MutableState = State;
        Writer << MutableState.VehicleHealth.ChannelHealth;
        Writer << MutableState.VehicleHealth.FunctionalDamageSerial;
        Writer << MutableState.Passenger.IdentityId;
        Writer << MutableState.Passenger.TemplateId;
        Writer << MutableState.Passenger.IdentitySeed;
        Writer << MutableState.Passenger.AppearanceSeed;
        Writer << MutableState.Passenger.AppearanceProfileId;
        Writer << MutableState.Passenger.PaidFareCount;
        Writer << MutableState.Passenger.AuthoredEventCount;
        Writer << MutableState.Passenger.bRepeatEligible;
        Writer << MutableState.Passenger.bNeuralPermissionGranted;
        Writer << MutableState.Passenger.bNeuralBlocked;
        Writer << MutableState.Passenger.Trust;
        Writer << MutableState.Passenger.Satisfaction;
        Writer << MutableState.Passenger.RiskTolerance;
        Writer << MutableState.Passenger.ReviewStars;
        Writer << MutableState.Passenger.ReviewTexts;
        Writer << MutableState.Economy.BalanceMinor;
        Writer << MutableState.Economy.DebtLimitMinor;
        Writer << MutableState.Economy.CommittedTransactionIds;
        Writer.Close();
        return !Writer.IsError() && OutBytes.Num() > 0;
    }

    static EPinkCabLoadResult Deserialize(
        const TArray<uint8>& Bytes,
        const FPinkCabSaveHeader& ExpectedHeader,
        const FPinkCabMigrationRegistry& Registry,
        FPinkCabLogicalSaveState& OutState)
    {
        if (Bytes.Num() == 0)
        {
            return EPinkCabLoadResult::CorruptPayload;
        }

        FMemoryReader Reader(Bytes, true);
        uint32 Magic = 0;
        Reader << Magic;
        if (Magic != PayloadMagic)
        {
            return EPinkCabLoadResult::CorruptPayload;
        }

        FPinkCabSaveHeader SavedHeader;
        Reader << SavedHeader.ProductName;
        Reader << SavedHeader.SchemaVersion;
        Reader << SavedHeader.ConfigVersion;
        Reader << SavedHeader.GeneratorVersion;
        Reader << SavedHeader.ContentSetVersion;

        if (SavedHeader.ProductName != ExpectedHeader.ProductName)
        {
            return EPinkCabLoadResult::IncompatibleProduct;
        }
        if (SavedHeader.SchemaVersion != ExpectedHeader.SchemaVersion)
        {
            return Registry.HasBoundary(SavedHeader.SchemaVersion, ExpectedHeader.SchemaVersion)
                ? EPinkCabLoadResult::MigrationRequired
                : EPinkCabLoadResult::IncompatibleSchema;
        }
        if (SavedHeader.ConfigVersion != ExpectedHeader.ConfigVersion)
        {
            return EPinkCabLoadResult::IncompatibleConfig;
        }
        if (SavedHeader.GeneratorVersion != ExpectedHeader.GeneratorVersion ||
            SavedHeader.ContentSetVersion != ExpectedHeader.ContentSetVersion)
        {
            return EPinkCabLoadResult::IncompatibleWorldVersion;
        }

        FPinkCabLogicalSaveState Restored;
        Reader << Restored.VehicleHealth.ChannelHealth;
        Reader << Restored.VehicleHealth.FunctionalDamageSerial;
        Reader << Restored.Passenger.IdentityId;
        Reader << Restored.Passenger.TemplateId;
        Reader << Restored.Passenger.IdentitySeed;
        Reader << Restored.Passenger.AppearanceSeed;
        Reader << Restored.Passenger.AppearanceProfileId;
        Reader << Restored.Passenger.PaidFareCount;
        Reader << Restored.Passenger.AuthoredEventCount;
        Reader << Restored.Passenger.bRepeatEligible;
        Reader << Restored.Passenger.bNeuralPermissionGranted;
        Reader << Restored.Passenger.bNeuralBlocked;
        Reader << Restored.Passenger.Trust;
        Reader << Restored.Passenger.Satisfaction;
        Reader << Restored.Passenger.RiskTolerance;
        Reader << Restored.Passenger.ReviewStars;
        Reader << Restored.Passenger.ReviewTexts;
        Reader << Restored.Economy.BalanceMinor;
        Reader << Restored.Economy.DebtLimitMinor;
        Reader << Restored.Economy.CommittedTransactionIds;

        if (Reader.IsError())
        {
            return EPinkCabLoadResult::CorruptPayload;
        }
        OutState = MoveTemp(Restored);
        return EPinkCabLoadResult::Success;
    }

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
    static EPinkCabLoadResult CheckCompatibility(
        const FPinkCabSaveHeader& SavedHeader,
        const FPinkCabSaveHeader& ExpectedHeader,
        const FPinkCabMigrationRegistry& Registry)
    {
        if (SavedHeader.ProductName != ExpectedHeader.ProductName)
            return EPinkCabLoadResult::IncompatibleProduct;
        if (SavedHeader.SchemaVersion != ExpectedHeader.SchemaVersion)
        {
            return Registry.HasBoundary(SavedHeader.SchemaVersion, ExpectedHeader.SchemaVersion)
                ? EPinkCabLoadResult::MigrationRequired
                : EPinkCabLoadResult::IncompatibleSchema;
        }
        if (SavedHeader.ConfigVersion != ExpectedHeader.ConfigVersion)
            return EPinkCabLoadResult::IncompatibleConfig;
        if (SavedHeader.GeneratorVersion != ExpectedHeader.GeneratorVersion
            || SavedHeader.ContentSetVersion != ExpectedHeader.ContentSetVersion)
            return EPinkCabLoadResult::IncompatibleWorldVersion;
        return EPinkCabLoadResult::Success;
    }

    static constexpr uint32 AggregatePayloadMagic = 0x50434147u;
    static constexpr uint32 PayloadMagic = 0x50434C47u;
    TArray<uint8> LastCommittedBytes;
    FPinkCabCheckpointRing Checkpoints;
    EPinkCabPersistenceCommitReason LastCommitReason = EPinkCabPersistenceCommitReason::PeriodicCheckpoint;
};
