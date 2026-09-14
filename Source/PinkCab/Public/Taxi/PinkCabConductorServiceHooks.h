#pragma once

#include "CoreMinimal.h"
#include "Core/PinkCabStableId.h"
#include "Taxi/PinkCabPassengerRecord.h"

class FPinkCabConductorServiceHooks
{
public:
    explicit FPinkCabConductorServiceHooks(const FPinkCabPassengerRegistry& InRegistry)
        : Registry(InRegistry)
    {
    }

    int32 GetPassengerCount() const { return Registry.Num(); }
    bool HasDrivingAuthority() const { return false; }
    bool HasSteeringAuthority() const { return false; }

    bool CanContact(const FPinkCabStableId& PassengerId) const
    {
        const FPinkCabPassengerRecord* Record = Registry.Find(PassengerId);
        return Record != nullptr
            && Record->HasNeuralPermission()
            && !Record->IsNeuralBlocked();
    }

private:
    const FPinkCabPassengerRegistry& Registry;
};
