#include "Persistence/PinkCabEconomySnapshotArchive.h"

void FPinkCabEconomySnapshotArchive::Serialize(
    FArchive& Ar,
    FPinkCabEconomySnapshot& Snapshot)
{
    SerializeEconomySnapshot(Ar, Snapshot);
}

void FPinkCabEconomySnapshotArchive::SerializeSettlementRecord(
    FArchive& Ar,
    FPinkCabFareSettlementSnapshotRecord& Record)
{
    Ar << Record.FareId;
    SerializeEnum(Ar, Record.Kind);
    SerializeEnum(Ar, Record.FareResult);
    SerializeEnum(Ar, Record.TipResult);
    Ar << Record.ConsequenceId;
}

void FPinkCabEconomySnapshotArchive::SerializeEconomySnapshot(
    FArchive& Ar,
    FPinkCabEconomySnapshot& Snapshot)
{
    Ar << Snapshot.SchemaVersion;
    Ar << Snapshot.BalanceMinor;
    Ar << Snapshot.DebtLimitMinor;
    Ar << Snapshot.MaxCommittedTransactionIds;
    Ar << Snapshot.MaxResolvedFares;
    if (!IsCapacityReasonable(Snapshot.MaxCommittedTransactionIds)
        || !IsCapacityReasonable(Snapshot.MaxResolvedFares))
    {
        Ar.SetError();
        return;
    }

    SerializeArray(
        Ar,
        Snapshot.CommittedTransactionIds,
        Snapshot.MaxCommittedTransactionIds,
        [](FArchive& A, FString& Value)
        {
            A << Value;
        });
    SerializeArray(
        Ar,
        Snapshot.FareSettlements,
        Snapshot.MaxResolvedFares,
        [](FArchive& A, FPinkCabFareSettlementSnapshotRecord& Value)
        {
            SerializeSettlementRecord(A, Value);
        });
}
