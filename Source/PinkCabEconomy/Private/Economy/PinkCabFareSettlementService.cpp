#include "Economy/PinkCabFareSettlementService.h"

FPinkCabFareSettlementService::FPinkCabFareSettlementService(
    int32 InMaxResolvedFares)
    : MaxResolvedFares(FMath::Max(1, InMaxResolvedFares))
{
}

FPinkCabFareSettlementResult
FPinkCabFareSettlementService::CommitPaid(
    const FPinkCabStableId& FareId,
    int64 FareMinor,
    int64 TipMinor,
    FPinkCabEconomyLedger& Ledger)
{
    if (!FareId.IsValid() || FareMinor <= 0 || TipMinor < 0)
    {
        return {};
    }

    FPinkCabFareSettlementResult ExistingResult;
    if (TryResolveExisting(
        FareId,
        EPinkCabFareSettlementKind::Paid,
        ExistingResult))
    {
        return ExistingResult;
    }
    if (!HasRecordCapacity())
    {
        return {
            EPinkCabFareSettlementDisposition::CapacityExceeded};
    }
    const TArray<FString> RequiredReplayIds =
        BuildRequiredReplayIds(FareId, TipMinor);
    if (!Ledger.CanRecordTransactionIds(RequiredReplayIds))
    {
        return {
            EPinkCabFareSettlementDisposition::CapacityExceeded,
            EPinkCabSettlementResult::CapacityExceeded,
            EPinkCabSettlementResult::CapacityExceeded};
    }

    const FString FareTxKey =
        FareId.Serialize() + TEXT(".fare");
    const FString TipTxKey =
        FareId.Serialize() + TEXT(".tip");
    const FPinkCabTransactionId FareTxId(FareTxKey);
    const EPinkCabSettlementResult FareResult = Ledger.Commit(
        FPinkCabEconomyTransaction::Credit(
            FareTxId,
            EPinkCabTransactionType::FareIncome,
            FareMinor));
    if (!IsAcceptedCommitResult(FareResult))
    {
        return {
            EPinkCabFareSettlementDisposition::Invalid,
            FareResult};
    }

    const EPinkCabSettlementResult TipResult =
        CommitTip(TipTxKey, TipMinor, Ledger);
    if (!IsAcceptedCommitResult(TipResult))
    {
        return {
            EPinkCabFareSettlementDisposition::Invalid,
            FareResult,
            TipResult};
    }
    FPinkCabFareSettlementRecord Record;
    Record.Kind = EPinkCabFareSettlementKind::Paid;
    Record.FareResult = FareResult;
    Record.TipResult = TipResult;
    Records.Add(FareId.Serialize(), Record);

    const bool bRecoveredReplay =
        FareResult == EPinkCabSettlementResult::Duplicate
        || TipResult == EPinkCabSettlementResult::Duplicate;
    const EPinkCabFareSettlementDisposition Disposition =
        bRecoveredReplay
            ? EPinkCabFareSettlementDisposition::Replayed
            : EPinkCabFareSettlementDisposition::Committed;
    return Record.ToResult(Disposition);
}

FPinkCabFareSettlementResult
FPinkCabFareSettlementService::RecordEvasion(
    const FPinkCabStableId& FareId)
{
    if (!FareId.IsValid())
    {
        return {};
    }

    FPinkCabFareSettlementResult ExistingResult;
    if (TryResolveExisting(
        FareId,
        EPinkCabFareSettlementKind::Evaded,
        ExistingResult))
    {
        return ExistingResult;
    }
    if (!HasRecordCapacity())
    {
        return {
            EPinkCabFareSettlementDisposition::CapacityExceeded};
    }
    FPinkCabFareSettlementRecord Record;
    Record.Kind = EPinkCabFareSettlementKind::Evaded;
    Record.ConsequenceId = FPinkCabStableId(
        FareId.Serialize() + TEXT(".evasion"));
    Records.Add(FareId.Serialize(), Record);
    return Record.ToResult(
        EPinkCabFareSettlementDisposition::Committed);
}

int32 FPinkCabFareSettlementService::NumResolvedFares() const
{
    return Records.Num();
}

bool FPinkCabFareSettlementService::TryResolveExisting(
    const FPinkCabStableId& FareId,
    EPinkCabFareSettlementKind ExpectedKind,
    FPinkCabFareSettlementResult& OutResult) const
{
    const FPinkCabFareSettlementRecord* Existing =
        Records.Find(FareId.Serialize());
    if (!Existing)
    {
        return false;
    }

    if (Existing->Kind != ExpectedKind)
    {
        OutResult = {
            EPinkCabFareSettlementDisposition::Conflict};
        return true;
    }
    OutResult = Existing->ToReplayResult();
    return true;
}
bool FPinkCabFareSettlementService::HasRecordCapacity() const
{
    return Records.Num() < MaxResolvedFares;
}

bool FPinkCabFareSettlementService::IsAcceptedCommitResult(
    EPinkCabSettlementResult Result)
{
    return Result == EPinkCabSettlementResult::Committed
        || Result == EPinkCabSettlementResult::Duplicate;
}

TArray<FString>
FPinkCabFareSettlementService::BuildRequiredReplayIds(
    const FPinkCabStableId& FareId,
    int64 TipMinor)
{
    TArray<FString> RequiredIds;
    RequiredIds.Reserve(TipMinor > 0 ? 2 : 1);
    RequiredIds.Add(FareId.Serialize() + TEXT(".fare"));
    if (TipMinor > 0)
    {
        RequiredIds.Add(FareId.Serialize() + TEXT(".tip"));
    }
    return RequiredIds;
}

EPinkCabSettlementResult
FPinkCabFareSettlementService::CommitTip(
    const FString& TipTxKey,
    int64 TipMinor,
    FPinkCabEconomyLedger& Ledger)
{
    if (TipMinor <= 0)
    {
        return EPinkCabSettlementResult::Committed;
    }
    const FPinkCabTransactionId TipTxId(TipTxKey);
    return Ledger.Commit(
        FPinkCabEconomyTransaction::Credit(
            TipTxId,
            EPinkCabTransactionType::Tip,
            TipMinor));
}
