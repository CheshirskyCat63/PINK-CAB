#include "Economy/PinkCabEconomyLedger.h"

FPinkCabEconomyLedger::FPinkCabEconomyLedger(
    int64 InInitialBalanceMinor,
    int64 InDebtLimitMinor,
    int32 InMaxReplayJournalEntries)
    : BalanceMinor(InInitialBalanceMinor),
      DebtLimitMinor(FMath::Max<int64>(0, InDebtLimitMinor)),
      MaxReplayJournalEntries(FMath::Max(1, InMaxReplayJournalEntries))
{
}

int64 FPinkCabEconomyLedger::GetBalanceMinor() const
{
    return BalanceMinor;
}

int64 FPinkCabEconomyLedger::GetAffordableFuelSpendMinor(
    int64 RequestedMinor) const
{
    if (RequestedMinor <= 0 || BalanceMinor <= 0)
    {
        return 0;
    }
    return FMath::Min(RequestedMinor, BalanceMinor);
}

EPinkCabSettlementResult FPinkCabEconomyLedger::Commit(
    const FPinkCabEconomyTransaction& Transaction)
{
    if (!Transaction.Id.IsValid() || Transaction.AmountMinor <= 0)
    {
        return EPinkCabSettlementResult::InvalidTransaction;
    }
    if (CommittedIds.Contains(Transaction.Id.GetValue()))
    {
        return EPinkCabSettlementResult::Duplicate;
    }
    if (CommittedIds.Num() >= MaxReplayJournalEntries)
    {
        return EPinkCabSettlementResult::CapacityExceeded;
    }

    const bool bTypeIsCredit = IsCreditType(Transaction.Type);
    if (bTypeIsCredit != Transaction.bCredit)
    {
        return EPinkCabSettlementResult::InvalidTransaction;
    }

    if (Transaction.bCredit)
    {
        BalanceMinor += Transaction.AmountMinor;
        CommittedIds.Add(Transaction.Id.GetValue());
        return EPinkCabSettlementResult::Committed;
    }

    const int64 CandidateBalance =
        BalanceMinor - Transaction.AmountMinor;
    if (CandidateBalance < 0 && !CanUseDebt(Transaction))
    {
        return EPinkCabSettlementResult::InsufficientFunds;
    }
    if (CandidateBalance < -DebtLimitMinor)
    {
        return EPinkCabSettlementResult::InsufficientFunds;
    }
    BalanceMinor = CandidateBalance;
    CommittedIds.Add(Transaction.Id.GetValue());
    return EPinkCabSettlementResult::Committed;
}

bool FPinkCabEconomyLedger::IsCreditType(
    EPinkCabTransactionType Type)
{
    return Type == EPinkCabTransactionType::FareIncome
        || Type == EPinkCabTransactionType::Tip;
}

bool FPinkCabEconomyLedger::CanUseDebt(
    const FPinkCabEconomyTransaction& Transaction)
{
    switch (Transaction.Purpose)
    {
    case EPinkCabEconomyPurpose::EssentialRecovery:
    case EPinkCabEconomyPurpose::MinimumRoadworthyRepair:
    case EPinkCabEconomyPurpose::MandatoryDayObligation:
        return true;
    case EPinkCabEconomyPurpose::General:
    case EPinkCabEconomyPurpose::OrdinaryPurchase:
    case EPinkCabEconomyPurpose::DiscretionaryTuning:
    default:
        return false;
    }
}

bool FPinkCabEconomyLedger::CanRecordTransactionIds(
    TConstArrayView<FString> Ids) const
{
    int32 Needed = 0;
    TSet<FString> NewIds;
    for (const FString& Id : Ids)
    {
        if (!CommittedIds.Contains(Id) && !NewIds.Contains(Id))
        {
            NewIds.Add(Id);
            ++Needed;
        }
    }
    return CommittedIds.Num() + Needed <= MaxReplayJournalEntries;
}
