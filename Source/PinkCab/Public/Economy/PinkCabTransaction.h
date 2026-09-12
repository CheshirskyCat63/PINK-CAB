#pragma once

#include "CoreMinimal.h"
#include "Core/PinkCabStableId.h"

struct FPinkCabTransactionId
{
    FPinkCabTransactionId() = default;
    explicit FPinkCabTransactionId(const FString& InValue)
    {
        FPinkCabStableId Parsed;
        if (FPinkCabStableId::TryParse(InValue, Parsed))
        {
            StableId = Parsed;
        }
    }

    bool IsValid() const { return StableId.IsValid(); }
    const FString& GetValue() const { return StableId.Serialize(); }

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

struct FPinkCabEconomyTransaction
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
        EPinkCabEconomyPurpose InPurpose = EPinkCabEconomyPurpose::General)
    {
        return FPinkCabEconomyTransaction{InId, InType, InPurpose, InAmountMinor, true};
    }

    static FPinkCabEconomyTransaction Debit(
        const FPinkCabTransactionId& InId,
        EPinkCabTransactionType InType,
        int64 InAmountMinor,
        EPinkCabEconomyPurpose InPurpose = EPinkCabEconomyPurpose::General)
    {
        return FPinkCabEconomyTransaction{InId, InType, InPurpose, InAmountMinor, false};
    }
};
