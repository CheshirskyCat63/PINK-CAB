#pragma once

#include "Persistence/PinkCabGameSnapshotArchivePrimitives.h"

class PINKCAB_API FPinkCabEconomySnapshotArchive
    : protected FPinkCabGameSnapshotArchivePrimitives
{
public:
    static void Serialize(
        FArchive& Ar,
        FPinkCabEconomySnapshot& Snapshot);

private:
    static void SerializeSettlementRecord(
        FArchive& Ar,
        FPinkCabFareSettlementSnapshotRecord& Record);
    static void SerializeEconomySnapshot(
        FArchive& Ar,
        FPinkCabEconomySnapshot& Snapshot);
};
