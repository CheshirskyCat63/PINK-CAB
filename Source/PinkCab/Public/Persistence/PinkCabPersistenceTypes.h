#pragma once

#include "CoreMinimal.h"

enum class EPinkCabLoadResult : uint8
{
    Success,
    CorruptPayload,
    IncompatibleProduct,
    IncompatibleSchema,
    IncompatibleConfig,
    IncompatibleWorldVersion,
    MigrationRequired,
    NoCommittedSnapshot
};

enum class EPinkCabPersistenceCommitReason : uint8
{
    FareSettlement,
    ServiceSettlement,
    RefuelSettlement,
    RepeatClientPromotion,
    WorkdayEnd,
    PersistentWorldChange,
    PeriodicCheckpoint,
    TerminalRecovery
};
