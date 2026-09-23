#pragma once

#include "Taxi/PinkCabFareRuntimeStateSnapshot.h"

struct FPinkCabFareRuntimeSnapshot : FPinkCabFareRuntimeStateSnapshot
{
    static constexpr int32 CurrentSchemaVersion = 1;
    int32 SchemaVersion = CurrentSchemaVersion;
};
