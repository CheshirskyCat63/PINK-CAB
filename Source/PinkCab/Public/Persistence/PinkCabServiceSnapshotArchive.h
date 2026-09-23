#pragma once

#include "Persistence/PinkCabGameSnapshotArchivePrimitives.h"

class PINKCAB_API FPinkCabServiceSnapshotArchive
    : protected FPinkCabGameSnapshotArchivePrimitives
{
public:
    static void Serialize(
        FArchive& Ar,
        FPinkCabServiceSnapshot& Snapshot);

private:
    static void SerializeOwnerIdentity(
        FArchive& Ar,
        FPinkCabServiceOwnerIdentity& Owners);
    static void SerializeServiceContext(
        FArchive& Ar,
        FPinkCabServiceContext& Context);
    static void SerializeOwnedPart(
        FArchive& Ar,
        FPinkCabOwnedPartSnapshot& Part);
    static void SerializeInstalledPart(
        FArchive& Ar,
        FPinkCabInstalledPartSnapshot& Part);
    static void SerializeMovingFuelPolicy(
        FArchive& Ar,
        FPinkCabMovingFuelPolicyInputs& Policy);

    static void SerializeInventory(
        FArchive& Ar,
        FPinkCabServiceSnapshot& Snapshot);
    static void SerializeBuild(
        FArchive& Ar,
        FPinkCabServiceSnapshot& Snapshot);
    static void SerializeOperations(
        FArchive& Ar,
        FPinkCabServiceSnapshot& Snapshot);
    static void SerializeFuel(
        FArchive& Ar,
        FPinkCabServiceSnapshot& Snapshot);
    static void SerializeMovingFuel(
        FArchive& Ar,
        FPinkCabServiceSnapshot& Snapshot);
    static void ValidateCapacities(
        FArchive& Ar,
        const FPinkCabServiceSnapshot& Snapshot);
    static void SerializeServiceSnapshot(
        FArchive& Ar,
        FPinkCabServiceSnapshot& Snapshot);
};
