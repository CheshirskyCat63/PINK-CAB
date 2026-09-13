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

class FPinkCabFuelTank
{
public:
    FPinkCabFuelTank(float InCapacityLiters, float InCurrentLiters, int32 InMaxReplayJournalEntries = 4096)
        : CapacityLiters(FMath::Max(0.0f, InCapacityLiters))
        , CurrentLiters(FMath::Clamp(InCurrentLiters, 0.0f, CapacityLiters))
        , MaxReplayJournalEntries(FMath::Max(1, InMaxReplayJournalEntries))
    {
    }

    float GetCapacityLiters() const { return CapacityLiters; }
    float GetLiters() const { return CurrentLiters; }
    float GetFreeCapacityLiters() const { return FMath::Max(0.0f, CapacityLiters - CurrentLiters); }

    EPinkCabFuelCreditResult CreditFuelOnce(const FPinkCabTransactionId& TransactionId, float RequestedLiters)
    {
        if (!TransactionId.IsValid() || RequestedLiters <= 0.0f)
        {
            return EPinkCabFuelCreditResult::InvalidCredit;
        }
        if (CreditedTransactionIds.Contains(TransactionId.GetValue()))
        {
            return EPinkCabFuelCreditResult::Duplicate;
        }

        if (CreditedTransactionIds.Num() >= MaxReplayJournalEntries)
        {
            return EPinkCabFuelCreditResult::JournalFull;
        }

        const float Credited = FMath::Min(RequestedLiters, GetFreeCapacityLiters());
        if (Credited <= 0.0f)
        {
            return EPinkCabFuelCreditResult::InvalidCredit;
        }
        CurrentLiters += Credited;
        CreditedTransactionIds.Add(TransactionId.GetValue());
        return EPinkCabFuelCreditResult::Applied;
    }

private:
    friend class FPinkCabServiceSnapshotCodec;

    float CapacityLiters = 0.0f;
    float CurrentLiters = 0.0f;
    int32 MaxReplayJournalEntries = 4096;
    TSet<FString> CreditedTransactionIds;
};
