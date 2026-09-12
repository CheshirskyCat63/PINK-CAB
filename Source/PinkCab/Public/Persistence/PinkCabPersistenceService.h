#pragma once

#include "CoreMinimal.h"
#include "Serialization/MemoryReader.h"
#include "Serialization/MemoryWriter.h"
#include "Persistence/PinkCabSaveHeader.h"
#include "Persistence/PinkCabMigrationRegistry.h"

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

struct FPinkCabPersistedVehicleHealth
{
    TArray<float> ChannelHealth;
    uint32 FunctionalDamageSerial = 0;
};

struct FPinkCabPersistedPassengerIdentity
{
    FString IdentityId;
    FString TemplateId;
    float Trust = 0.0f;
    float Satisfaction = 0.0f;
    float RiskTolerance = 0.0f;
    TArray<int32> ReviewStars;
    TArray<FString> ReviewTexts;
};

struct FPinkCabPersistedEconomyState
{
    int64 BalanceMinor = 0;
    int64 DebtLimitMinor = 0;
    TArray<FString> CommittedTransactionIds;
};

struct FPinkCabLogicalSaveState
{
    FPinkCabPersistedVehicleHealth VehicleHealth;
    FPinkCabPersistedPassengerIdentity Passenger;
    FPinkCabPersistedEconomyState Economy;
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

private:
    TArray<TArray<uint8>> Checkpoints;
};

class FPinkCabPersistenceService
{
public:
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
    static constexpr uint32 PayloadMagic = 0x50434C47u;
    TArray<uint8> LastCommittedBytes;
    FPinkCabCheckpointRing Checkpoints;
    EPinkCabPersistenceCommitReason LastCommitReason = EPinkCabPersistenceCommitReason::PeriodicCheckpoint;
};
