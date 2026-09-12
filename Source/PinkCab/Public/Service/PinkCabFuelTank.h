#pragma once

#include "CoreMinimal.h"
#include "Economy/PinkCabTransaction.h"

enum class EPinkCabFuelCreditResult : uint8
{
    Applied,
    Duplicate,
    InvalidState,
    SettlementNotCommitted,
    InvalidCredit
};

class FPinkCabFuelTank
{
public:
    FPinkCabFuelTank(float InCapacityLiters, float InCurrentLiters)
        : CapacityLiters(FMath::Max(0.0f, InCapacityLiters))
        , CurrentLiters(FMath::Clamp(InCurrentLiters, 0.0f, CapacityLiters))
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
    float CapacityLiters = 0.0f;
    float CurrentLiters = 0.0f;
    TSet<FString> CreditedTransactionIds;
};
