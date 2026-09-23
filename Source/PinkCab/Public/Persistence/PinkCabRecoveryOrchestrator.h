#pragma once

#include "CoreMinimal.h"
#include "Persistence/PinkCabGamePersistenceCoordinator.h"
#include "Persistence/PinkCabSaveHeader.h"

class PINKCAB_API FPinkCabRecoveryOrchestrator
{
public:
    static bool ApplyTerminalRecovery(
        FPinkCabGamePersistenceOwners& Owners);
    static bool AdvanceToNextWorkday(
        const FPinkCabStableId& NextWorkdayId,
        int32 NextOrdinal,
        uint64 NextRootSeed,
        FPinkCabGamePersistenceOwners& Owners);
};
