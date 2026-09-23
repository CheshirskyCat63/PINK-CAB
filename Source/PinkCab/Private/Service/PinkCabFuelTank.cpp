#include "Service/PinkCabFuelTank.h"

FPinkCabFuelTank::FPinkCabFuelTank(
    float InCapacityLiters,
    float InCurrentLiters,
    int32 InMaxReplayJournalEntries)
    : CapacityLiters(FMath::Max(0.0f, InCapacityLiters))
    , CurrentLiters(
        FMath::Clamp(
            InCurrentLiters,
            0.0f,
            CapacityLiters))
    , MaxReplayJournalEntries(
        FMath::Max(1, InMaxReplayJournalEntries))
{
}

float FPinkCabFuelTank::GetCapacityLiters() const
{
    return CapacityLiters;
}

float FPinkCabFuelTank::GetLiters() const
{
    return CurrentLiters;
}

float FPinkCabFuelTank::GetFreeCapacityLiters() const
{
    return FMath::Max(0.0f, CapacityLiters - CurrentLiters);
}

EPinkCabFuelCreditResult FPinkCabFuelTank::CreditFuelOnce(
    const FPinkCabTransactionId& TransactionId,
    float RequestedLiters)
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

    const float Credited =
        FMath::Min(RequestedLiters, GetFreeCapacityLiters());
    if (Credited <= 0.0f)
    {
        return EPinkCabFuelCreditResult::InvalidCredit;
    }

    CurrentLiters += Credited;
    CreditedTransactionIds.Add(TransactionId.GetValue());
    return EPinkCabFuelCreditResult::Applied;
}
