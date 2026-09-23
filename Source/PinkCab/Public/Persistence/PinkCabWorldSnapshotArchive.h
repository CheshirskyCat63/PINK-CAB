#pragma once

#include "Persistence/PinkCabGameSnapshotArchivePrimitives.h"

class PINKCAB_API FPinkCabWorldSnapshotArchive
    : protected FPinkCabGameSnapshotArchivePrimitives
{
public:
    static void Serialize(
        FArchive& Ar,
        FPinkCabCityDeltaSnapshot& CityDeltas,
        FPinkCabStateKernelSnapshot& Kernel,
        FPinkCabWorkdaySessionSnapshot& Workday);

private:
    static void SerializeCityDeltaRecord(
        FArchive& Ar,
        FPinkCabCityDeltaSnapshotRecord& Record);
    static void SerializeWorldSnapshots(
        FArchive& Ar,
        FPinkCabCityDeltaSnapshot& CityDeltas,
        FPinkCabStateKernelSnapshot& Kernel,
        FPinkCabWorkdaySessionSnapshot& Workday);
};
