#pragma once

#include "CoreMinimal.h"
#include "Persistence/PinkCabPersistenceTypes.h"
#include "Persistence/PinkCabSaveHeader.h"
#include "Persistence/PinkCabMigrationRegistry.h"
#include "Persistence/PinkCabPersistedLogicalState.h"
#include "Persistence/PinkCabGameSnapshotArchive.h"
#include "Persistence/PinkCabGamePersistenceCoordinator.h"

class PINKCAB_API FPinkCabPersistenceBinaryCodec
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

private:
    static EPinkCabLoadResult CheckCompatibility(
        const FPinkCabSaveHeader& SavedHeader,
        const FPinkCabSaveHeader& ExpectedHeader,
        const FPinkCabMigrationRegistry& Registry);

    static constexpr uint32 AggregatePayloadMagic = 0x50434147u;
    static constexpr uint32 PayloadMagic = 0x50434C47u;
};
