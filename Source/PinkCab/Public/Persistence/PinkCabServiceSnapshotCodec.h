#pragma once

#include "Service/PinkCabServiceSnapshotTypes.h"

class PINKCAB_API FPinkCabServiceSnapshotCodec
{
public:
    static bool Capture(
        const FPinkCabServiceContext& Context,
        const FPinkCabServiceInventory& Inventory,
        const FPinkCabVehicleBuild& Build,
        const FPinkCabServiceOperationRuntime& Operations,
        const FPinkCabFuelTank& Tank,
        const FPinkCabMovingFuelSession& MovingFuel,
        FPinkCabServiceSnapshot& OutSnapshot);
    static bool Restore(
        const FPinkCabServiceSnapshot& Snapshot,
        FPinkCabServiceContext& OutContext,
        FPinkCabServiceInventory& OutInventory,
        FPinkCabVehicleBuild& OutBuild,
        FPinkCabServiceOperationRuntime& OutOperations,
        FPinkCabFuelTank& OutTank,
        FPinkCabMovingFuelSession& OutMovingFuel);

private:
    static bool ValidateIds(const TArray<FString>& Ids, int32 MaxEntries);
    static bool ValidateInventory(const FPinkCabInventorySnapshot& Inventory);
    static bool ValidateBuild(const FPinkCabBuildSnapshot& Build);
    static bool ValidateFuelTank(const FPinkCabFuelTankSnapshot& FuelTank);
    static bool ValidateMovingFuel(const FPinkCabMovingFuelSnapshot& MovingFuel);
    static bool Validate(const FPinkCabServiceSnapshot& Snapshot);
};
