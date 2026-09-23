#pragma once

#include "CoreMinimal.h"
#include "Core/PinkCabStableId.h"

struct PINKCABECONOMY_API FPinkCabTransactionId
{
    FPinkCabTransactionId() = default;
    explicit FPinkCabTransactionId(const FString& InValue);

    bool IsValid() const;
    const FString& GetValue() const;

private:
    FPinkCabStableId StableId;
};

enum class EPinkCabTransactionType : uint8
{
    FareIncome,
    Tip,
    FuelPurchase,
    PartPurchase,
    Repair,
    Parking,
    Fine
};

enum class EPinkCabEconomyPurpose : uint8
{
    General,
    OrdinaryPurchase,
    EssentialRecovery,
    MinimumRoadworthyRepair,
    MandatoryDayObligation,
    DiscretionaryTuning
};

struct PINKCABECONOMY_API FPinkCabEconomyTransaction
{
    FPinkCabTransactionId Id;
    EPinkCabTransactionType Type = EPinkCabTransactionType::FareIncome;
    EPinkCabEconomyPurpose Purpose = EPinkCabEconomyPurpose::General;
    int64 AmountMinor = 0;
    bool bCredit = true;

    static FPinkCabEconomyTransaction Credit(
        const FPinkCabTransactionId& InId,
        EPinkCabTransactionType InType,
        int64 InAmountMinor,
        EPinkCabEconomyPurpose InPurpose = EPinkCabEconomyPurpose::General);

    static FPinkCabEconomyTransaction Debit(
        const FPinkCabTransactionId& InId,
        EPinkCabTransactionType InType,
        int64 InAmountMinor,
        EPinkCabEconomyPurpose InPurpose = EPinkCabEconomyPurpose::General);
};
