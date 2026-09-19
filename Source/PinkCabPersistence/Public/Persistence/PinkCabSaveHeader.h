#pragma once

#include "CoreMinimal.h"

struct FPinkCabSaveHeader
{
    FString ProductName;
    FString SchemaVersion;
    FString ConfigVersion;
    FString GeneratorVersion;
    FString ContentSetVersion;

    static FPinkCabSaveHeader Create(
        const FString& InProduct,
        const FString& InSchema,
        const FString& InConfig,
        const FString& InGenerator,
        const FString& InContent)
    {
        FPinkCabSaveHeader Header;
        Header.ProductName = InProduct;
        Header.SchemaVersion = InSchema;
        Header.ConfigVersion = InConfig;
        Header.GeneratorVersion = InGenerator;
        Header.ContentSetVersion = InContent;
        return Header;
    }
};

struct FPinkCabTerminalRecoveryPolicy
{
    bool bFailActiveFare = true;
    bool bEndWorkday = true;
    bool bPreserveDamagedVehicle = true;
    bool bFreeReset = false;

    static FPinkCabTerminalRecoveryPolicy Canonical()
    {
        return FPinkCabTerminalRecoveryPolicy();
    }
};

struct FPinkCabPersistencePolicy
{
    static constexpr int32 CampaignSlotCount = 3;
    static constexpr int32 RollingCheckpointCount = 3;
    static constexpr int32 PeriodicCheckpointSeconds = 300;
    static constexpr int32 WorkdayGameHours = 12;
    static constexpr int32 WorkdayRealMinutes = 120;
    static constexpr int32 WorkdayTimeScale = 6;

    static bool CanManualSaveExit(
        const bool bActiveFare,
        const bool bPassengerPresent,
        const bool bFullStop)
    {
        return !bActiveFare && !bPassengerPresent && bFullStop;
    }
    static bool CanSleepEndDay(
        const bool bInVehicle,
        const bool bFullStop,
        const bool bActiveFare)
    {
        return bInVehicle && bFullStop && !bActiveFare;
    }

    static bool ShouldHardPause(const bool bSystemMenu)
    {
        return bSystemMenu;
    }

    static bool CanAdvanceToNextWorkday(
        const bool bSummaryCommitted,
        const int32 HouseholdTransactionCount)
    {
        return bSummaryCommitted && HouseholdTransactionCount == 1;
    }
};
