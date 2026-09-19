#pragma once

#include "Persistence/PinkCabGameSnapshotArchivePrimitives.h"

class PINKCAB_API FPinkCabVehicleSnapshotArchive
    : protected FPinkCabGameSnapshotArchivePrimitives
{
public:
    static void Serialize(
        FArchive& Ar,
        FPinkCabVehicleSnapshot& Snapshot);

private:
    static void SerializeLoadItem(
        FArchive& Ar,
        FPinkCabVehicleLoadItemSnapshot& Item);
    static void SerializeVehicleSnapshot(
        FArchive& Ar,
        FPinkCabVehicleSnapshot& Snapshot);
};
