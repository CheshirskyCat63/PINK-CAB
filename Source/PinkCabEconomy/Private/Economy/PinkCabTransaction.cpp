#include "Economy/PinkCabTransaction.h"

FPinkCabTransactionId::FPinkCabTransactionId(const FString& InValue)
{
    FPinkCabStableId Parsed;
    if (FPinkCabStableId::TryParse(InValue, Parsed))
    {
        StableId = Parsed;
    }
}

bool FPinkCabTransactionId::IsValid() const
{
    return StableId.IsValid();
}

const FString& FPinkCabTransactionId::GetValue() const
{
    return StableId.Serialize();
}

FPinkCabEconomyTransaction FPinkCabEconomyTransaction::Credit(
    const FPinkCabTransactionId& InId,
    EPinkCabTransactionType InType,
    int64 InAmountMinor,
    EPinkCabEconomyPurpose InPurpose)
{
    return {
        InId,
        InType,
        InPurpose,
        InAmountMinor,
        true};
}
FPinkCabEconomyTransaction FPinkCabEconomyTransaction::Debit(
    const FPinkCabTransactionId& InId,
    EPinkCabTransactionType InType,
    int64 InAmountMinor,
    EPinkCabEconomyPurpose InPurpose)
{
    return {
        InId,
        InType,
        InPurpose,
        InAmountMinor,
        false};
}
