#pragma once

#include "CoreMinimal.h"
#include "Core/PinkCabStableId.h"
#include "Taxi/PinkCabPassengerRecord.h"

class PINKCABTAXI_API FPinkCabConductorServiceHooks
{
public:
    explicit FPinkCabConductorServiceHooks(const FPinkCabPassengerRegistry& InRegistry);

    int32 GetPassengerCount() const;
    bool HasDrivingAuthority() const;
    bool HasSteeringAuthority() const;
    bool CanContact(const FPinkCabStableId& PassengerId) const;

private:
    const FPinkCabPassengerRegistry& Registry;
};
