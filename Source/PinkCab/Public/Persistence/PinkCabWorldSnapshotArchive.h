#pragma once

#include "Persistence/PinkCabGameSnapshotArchivePrimitives.h"

class FPinkCabWorldSnapshotArchive : protected FPinkCabGameSnapshotArchivePrimitives
{
public:
    static void Serialize(FArchive& Ar, FPinkCabCityDeltaSnapshot& CityDeltas, FPinkCabStateKernelSnapshot& Kernel, FPinkCabWorkdaySessionSnapshot& Workday)
    {
        SerializeWorldSnapshots(Ar, CityDeltas, Kernel, Workday);
    }

private:
    static void SerializeCityDeltaRecord(FArchive& Ar, FPinkCabCityDeltaSnapshotRecord& Record)
    {
        Ar << Record.DeltaId;
        SerializeEnum(Ar, Record.Kind);
        Ar << Record.SubjectStableId;
        Ar << Record.OperationKey;
    }

    static void SerializeWorldSnapshots(
        FArchive& Ar,
        FPinkCabCityDeltaSnapshot& CityDeltas,
        FPinkCabStateKernelSnapshot& Kernel,
        FPinkCabWorkdaySessionSnapshot& Workday)
    {
        Ar << CityDeltas.SchemaVersion;
        Ar << CityDeltas.MaxDeltas;
        if (!IsCapacityReasonable(CityDeltas.MaxDeltas))
        {
            Ar.SetError();
            return;
        }
        SerializeArray(Ar, CityDeltas.Records, CityDeltas.MaxDeltas,
            [](FArchive& A, FPinkCabCityDeltaSnapshotRecord& Value)
            { SerializeCityDeltaRecord(A, Value); });
        Ar << Kernel.SchemaVersion;
        Ar << Kernel.RootSeed;
        Ar << Kernel.Sequence;
        Ar << Workday.SchemaVersion;
        Ar << Workday.WorkdayId;
        Ar << Workday.Ordinal;
        Ar << Workday.ElapsedGameSeconds;
        Ar << Workday.MaxReplayJournalEntries;
        Ar << Workday.bSummaryCommitted;
        Ar << Workday.HouseholdTransactionCount;
        Ar << Workday.bTerminalRecovery;
        if (!IsCapacityReasonable(Workday.MaxReplayJournalEntries))
        {
            Ar.SetError();
            return;
        }
        SerializeArray(Ar, Workday.AppliedHouseholdOperationIds, Workday.MaxReplayJournalEntries,
            [](FArchive& A, FString& Value) { A << Value; });
    }
};
