#include "Persistence/PinkCabSaveHeader.h"

FPinkCabSaveHeader FPinkCabSaveHeader::Create(
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

FPinkCabTerminalRecoveryPolicy FPinkCabTerminalRecoveryPolicy::Canonical()
{
    return FPinkCabTerminalRecoveryPolicy();
}

bool FPinkCabPersistencePolicy::CanManualSaveExit(
    bool bActiveFare,
    bool bPassengerPresent,
    bool bFullStop)
{
    return !bActiveFare && !bPassengerPresent && bFullStop;
}

bool FPinkCabPersistencePolicy::CanSleepEndDay(
    bool bInVehicle,
    bool bFullStop,
    bool bActiveFare)
{
    return bInVehicle && bFullStop && !bActiveFare;
}

bool FPinkCabPersistencePolicy::ShouldHardPause(bool bSystemMenu)
{
    return bSystemMenu;
}

bool FPinkCabPersistencePolicy::CanAdvanceToNextWorkday(
    bool bSummaryCommitted,
    int32 HouseholdTransactionCount)
{
    return bSummaryCommitted && HouseholdTransactionCount == 1;
}
