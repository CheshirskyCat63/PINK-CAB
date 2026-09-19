#include "Taxi/PinkCabConductorServiceHooks.h"

FPinkCabConductorServiceHooks::FPinkCabConductorServiceHooks(
    const FPinkCabPassengerRegistry& InRegistry)
    : Registry(InRegistry)
{
}

int32 FPinkCabConductorServiceHooks::GetPassengerCount() const
{
    return Registry.Num();
}

bool FPinkCabConductorServiceHooks::HasDrivingAuthority() const
{
    return false;
}

bool FPinkCabConductorServiceHooks::HasSteeringAuthority() const
{
    return false;
}

bool FPinkCabConductorServiceHooks::CanContact(
    const FPinkCabStableId& PassengerId) const
{
    const FPinkCabPassengerRecord* Record = Registry.Find(PassengerId);
    return Record != nullptr
        && Record->HasNeuralPermission()
        && !Record->IsNeuralBlocked();
}
