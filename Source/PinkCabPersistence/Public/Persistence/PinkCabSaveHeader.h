#pragma once

#include "CoreMinimal.h"

struct PINKCABPERSISTENCE_API FPinkCabSaveHeader
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
        const FString& InContent);
};

struct PINKCABPERSISTENCE_API FPinkCabTerminalRecoveryPolicy
{
    bool bFailActiveFare = true;
    bool bEndWorkday = true;
    bool bPreserveDamagedVehicle = true;
    bool bFreeReset = false;

    static FPinkCabTerminalRecoveryPolicy Canonical();
};

struct PINKCABPERSISTENCE_API FPinkCabPersistencePolicy
{
    static constexpr int32 CampaignSlotCount = 3;
    static constexpr int32 RollingCheckpointCount = 3;
    static constexpr int32 PeriodicCheckpointSeconds = 300;
    static constexpr int32 WorkdayGameHours = 12;
    static constexpr int32 WorkdayRealMinutes = 120;
    static constexpr int32 WorkdayTimeScale = 6;

    static bool CanManualSaveExit(
        bool bActiveFare,
        bool bPassengerPresent,
        bool bFullStop);
    static bool CanSleepEndDay(
        bool bInVehicle,
        bool bFullStop,
        bool bActiveFare);
    static bool ShouldHardPause(bool bSystemMenu);
    static bool CanAdvanceToNextWorkday(
        bool bSummaryCommitted,
        int32 HouseholdTransactionCount);
};
