#pragma once

#include "CoreMinimal.h"
#include "Economy/PinkCabTransaction.h"

enum class EPinkCabSettlementResult : uint8
{
    Committed,
    Duplicate,
    InvalidTransaction,
    InsufficientFunds,
    PolicyRequired,
    CapacityExceeded
};

class FPinkCabEconomyLedger
{
public:
    FPinkCabEconomyLedger(int64 InInitialBalanceMinor, int64 InDebtLimitMinor, int32 InMaxReplayJournalEntries = 512)
        : BalanceMinor(InInitialBalanceMinor)
        , DebtLimitMinor(FMath::Max<int64>(0, InDebtLimitMinor))
        , MaxReplayJournalEntries(FMath::Max(1, InMaxReplayJournalEntries))
    {
    }

    int64 GetBalanceMinor() const { return BalanceMinor; }

    int64 GetAffordableFuelSpendMinor(int64 RequestedMinor) const
    {
        if (RequestedMinor <= 0 || BalanceMinor <= 0)
        {
            return 0;
        }
        return FMath::Min(RequestedMinor, BalanceMinor);
    }

    EPinkCabSettlementResult Commit(const FPinkCabEconomyTransaction& Transaction)
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

        const int64 CandidateBalance = BalanceMinor - Transaction.AmountMinor;
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

private:
    static bool IsCreditType(EPinkCabTransactionType Type)
    {
        return Type == EPinkCabTransactionType::FareIncome || Type == EPinkCabTransactionType::Tip;
    }

    static bool CanUseDebt(const FPinkCabEconomyTransaction& Transaction)
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

    bool CanRecordTransactionIds(TConstArrayView<FString> Ids) const
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

    friend class FPinkCabEconomySnapshotCodec;
    friend class FPinkCabFareSettlementService;

    int64 BalanceMinor = 0;
    int64 DebtLimitMinor = 0;
    int32 MaxReplayJournalEntries = 512;
    TSet<FString> CommittedIds;
};
