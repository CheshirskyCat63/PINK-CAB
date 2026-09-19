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

class PINKCABECONOMY_API FPinkCabEconomyLedger
{
public:
    FPinkCabEconomyLedger(
        int64 InInitialBalanceMinor,
        int64 InDebtLimitMinor,
        int32 InMaxReplayJournalEntries = 512);

    int64 GetBalanceMinor() const;
    int64 GetAffordableFuelSpendMinor(int64 RequestedMinor) const;
    EPinkCabSettlementResult Commit(
        const FPinkCabEconomyTransaction& Transaction);

private:
    static bool IsCreditType(EPinkCabTransactionType Type);
    static bool CanUseDebt(const FPinkCabEconomyTransaction& Transaction);
    bool CanRecordTransactionIds(TConstArrayView<FString> Ids) const;

    friend class FPinkCabEconomySnapshotCodec;
    friend class FPinkCabFareSettlementService;

    int64 BalanceMinor = 0;
    int64 DebtLimitMinor = 0;
    int32 MaxReplayJournalEntries = 512;
    TSet<FString> CommittedIds;
};
