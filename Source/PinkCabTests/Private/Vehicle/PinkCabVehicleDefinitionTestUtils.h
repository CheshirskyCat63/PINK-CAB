#pragma once

#include "Vehicle/PinkCabVehicleDefinition.h"

namespace PinkCabVehicleDefinitionTestUtils
{
inline const UPinkCabVehicleDefinition* LoadTatra()
{
    return LoadObject<UPinkCabVehicleDefinition>(
        nullptr,
        TEXT("/Game/Dev/Vehicles/Definitions/DA_PC_Vehicle_tatra613.DA_PC_Vehicle_tatra613"));
}

inline const UPinkCabVehicleDefinition* LoadFixture()
{
    return LoadObject<UPinkCabVehicleDefinition>(
        nullptr,
        TEXT("/Game/Dev/Vehicles/Definitions/DA_PC_Vehicle_fixture.DA_PC_Vehicle_fixture"));
}
}
