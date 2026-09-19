#pragma once

#include "CoreMinimal.h"
#include "Economy/PinkCabTransaction.h"

enum class EPinkCabFuelCreditResult : uint8
{
    Applied,
    Duplicate,
    InvalidState,
    SettlementNotCommitted,
    InvalidCredit,
    JournalFull
};

class PINKCAB_API FPinkCabFuelTank
{
public:
    FPinkCabFuelTank(
        float InCapacityLiters,
        float InCurrentLiters,
        int32 InMaxReplayJournalEntries = 4096);

    float GetCapacityLiters() const;
    float GetLiters() const;
    float GetFreeCapacityLiters() const;
    EPinkCabFuelCreditResult CreditFuelOnce(
        const FPinkCabTransactionId& TransactionId,
        float RequestedLiters);

private:
    friend class FPinkCabServiceSnapshotCodec;

    float CapacityLiters = 0.0f;
    float CurrentLiters = 0.0f;
    int32 MaxReplayJournalEntries = 4096;
    TSet<FString> CreditedTransactionIds;
};
