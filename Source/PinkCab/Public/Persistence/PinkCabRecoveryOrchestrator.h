#pragma once

#include "CoreMinimal.h"
#include "Persistence/PinkCabGamePersistenceCoordinator.h"
#include "Persistence/PinkCabSaveHeader.h"

class FPinkCabRecoveryOrchestrator
{
public:
    static bool ApplyTerminalRecovery(FPinkCabGamePersistenceOwners& Owners)
    {
        const FPinkCabTerminalRecoveryPolicy Policy =
            FPinkCabTerminalRecoveryPolicy::Canonical();
        if (Policy.bFailActiveFare)
        {
            const EPinkCabFareLoopState State = Owners.Fare.GetState();
            if ((State == EPinkCabFareLoopState::Active
                    || State == EPinkCabFareLoopState::AwaitingPayment)
                && !Owners.Fare.FailForTerminalRecovery(Owners.Load))
            {
                return false;
            }
        }

        Owners.Workday.MarkTerminalRecovery();
        return true;
    }

    static bool AdvanceToNextWorkday(
        const FPinkCabStableId& NextWorkdayId,
        int32 NextOrdinal,
        uint64 NextRootSeed,
        FPinkCabGamePersistenceOwners& Owners)
    {
        if (!Owners.Workday.CanAdvanceToNextWorkday())
        {
            return false;
        }

        FPinkCabWorkdaySessionState NextWorkday = Owners.Workday;
        if (!NextWorkday.ResetForNextWorkday(NextWorkdayId, NextOrdinal))
        {
            return false;
        }

        Owners.Workday = MoveTemp(NextWorkday);
        Owners.Kernel.ResetWorkday(NextRootSeed);
        Owners.Fare = FPinkCabFareLoopCoordinator();
        return true;
    }
};
