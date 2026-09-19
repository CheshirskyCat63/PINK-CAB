#pragma once

#include "Persistence/PinkCabGameSnapshotArchivePrimitives.h"

class PINKCAB_API FPinkCabPassengerSnapshotArchive
    : protected FPinkCabGameSnapshotArchivePrimitives
{
public:
    static void Serialize(
        FArchive& Ar,
        FPinkCabPassengerSnapshot& Snapshot);

private:
    static void SerializeRelationship(
        FArchive& Ar,
        FPinkCabPassengerRelationship& Relationship);
    static void SerializeRideMemory(
        FArchive& Ar,
        FPinkCabPassengerRideMemory& Memory);
    static void SerializeNeuralMessage(
        FArchive& Ar,
        FPinkCabPassengerNeuralMessage& Message);
    static void SerializePassengerRecord(
        FArchive& Ar,
        FPinkCabPassengerRecord& Record);
    static void SerializePassengerSnapshot(
        FArchive& Ar,
        FPinkCabPassengerSnapshot& Snapshot);
};
