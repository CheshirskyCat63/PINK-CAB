#pragma once

#include "CoreMinimal.h"
#include "World/PinkCabCityIdentity.h"
#include "Taxi/PinkCabPassengerSnapshot.h"
#include "Persistence/PinkCabEconomySnapshot.h"
#include "Persistence/PinkCabFareRuntimeSnapshot.h"
#include "Persistence/PinkCabVehicleSnapshot.h"
#include "Service/PinkCabServiceSnapshot.h"
#include "Persistence/PinkCabWorldSessionSnapshot.h"

struct FPinkCabGameSnapshot
{
    static constexpr int32 CurrentSchemaVersion = 1;

    int32 SchemaVersion = CurrentSchemaVersion;
    FPinkCabCityIdentity CityIdentity;
    FPinkCabPassengerSnapshot Passenger;
    FPinkCabEconomySnapshot Economy;
    FPinkCabFareRuntimeSnapshot FareRuntime;
    FPinkCabVehicleSnapshot Vehicle;
    FPinkCabServiceSnapshot Service;
    FPinkCabCityDeltaSnapshot CityDeltas;
    FPinkCabStateKernelSnapshot Kernel;
    FPinkCabWorkdaySessionSnapshot Workday;
};
