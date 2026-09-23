#include "Persistence/PinkCabPersistenceBinaryCodec.h"
#include "Serialization/MemoryReader.h"
#include "Serialization/MemoryWriter.h"

bool FPinkCabPersistenceBinaryCodec::Serialize(
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
    if (!FPinkCabGameSnapshotArchive::Serialize(
            Writer,
            MutableState))
    {
        return false;
    }

    Writer.Close();
    return !Writer.IsError() && OutBytes.Num() > 0;
}

EPinkCabLoadResult FPinkCabPersistenceBinaryCodec::Deserialize(
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

    const EPinkCabLoadResult Compatibility =
        CheckCompatibility(
            SavedHeader,
            ExpectedHeader,
            Registry);
    if (Compatibility != EPinkCabLoadResult::Success)
    {
        return Compatibility;
    }

    FPinkCabGameSnapshot Restored;
    if (!FPinkCabGameSnapshotArchive::Serialize(
            Reader,
            Restored)
        || Reader.IsError()
        || Reader.Tell() != Reader.TotalSize())
    {
        return EPinkCabLoadResult::CorruptPayload;
    }

    OutState = MoveTemp(Restored);
    return EPinkCabLoadResult::Success;
}

EPinkCabLoadResult FPinkCabPersistenceBinaryCodec::DeserializeInto(
    const TArray<uint8>& Bytes,
    const FPinkCabSaveHeader& ExpectedHeader,
    const FPinkCabMigrationRegistry& Registry,
    FPinkCabCityIdentity& OutCityIdentity,
    FPinkCabGamePersistenceOwners& Owners)
{
    FPinkCabGameSnapshot Snapshot;
    const EPinkCabLoadResult Result = Deserialize(
        Bytes,
        ExpectedHeader,
        Registry,
        Snapshot);
    if (Result != EPinkCabLoadResult::Success)
    {
        return Result;
    }

    FPinkCabCityIdentity RestoredCity;
    if (!FPinkCabGamePersistenceCoordinator::Restore(
            Snapshot,
            RestoredCity,
            Owners))
    {
        return EPinkCabLoadResult::CorruptPayload;
    }

    OutCityIdentity = MoveTemp(RestoredCity);
    return EPinkCabLoadResult::Success;
}

EPinkCabLoadResult FPinkCabPersistenceBinaryCodec::CheckCompatibility(
    const FPinkCabSaveHeader& SavedHeader,
    const FPinkCabSaveHeader& ExpectedHeader,
    const FPinkCabMigrationRegistry& Registry)
{
    if (SavedHeader.ProductName != ExpectedHeader.ProductName)
    {
        return EPinkCabLoadResult::IncompatibleProduct;
    }
    if (SavedHeader.SchemaVersion != ExpectedHeader.SchemaVersion)
    {
        return Registry.HasBoundary(
            SavedHeader.SchemaVersion,
            ExpectedHeader.SchemaVersion)
            ? EPinkCabLoadResult::MigrationRequired
            : EPinkCabLoadResult::IncompatibleSchema;
    }
    if (SavedHeader.ConfigVersion != ExpectedHeader.ConfigVersion)
    {
        return EPinkCabLoadResult::IncompatibleConfig;
    }
    if (SavedHeader.GeneratorVersion != ExpectedHeader.GeneratorVersion
        || SavedHeader.ContentSetVersion
            != ExpectedHeader.ContentSetVersion)
    {
        return EPinkCabLoadResult::IncompatibleWorldVersion;
    }
    return EPinkCabLoadResult::Success;
}
