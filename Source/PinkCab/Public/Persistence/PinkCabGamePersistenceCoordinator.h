#pragma once

#include "CoreMinimal.h"
#include "Persistence/PinkCabGameSnapshot.h"
#include "Persistence/PinkCabPassengerSnapshotCodec.h"
#include "Persistence/PinkCabServiceSnapshotCodec.h"

struct PINKCAB_API FPinkCabGamePersistenceOwners
{
    FPinkCabGamePersistenceOwners(
        FPinkCabPassengerRegistry& InPassengers,
        FPinkCabEconomyLedger& InEconomy,
        FPinkCabFareSettlementService& InSettlement,
        FPinkCabFareLoopCoordinator& InFare,
        FPinkCabVehicleHealthState& InHealth,
        FPinkCabVehicleLoadState& InLoad,
        FPinkCabServiceContext& InService,
        FPinkCabServiceInventory& InInventory,
        FPinkCabVehicleBuild& InBuild,
        FPinkCabServiceOperationRuntime& InOperations,
        FPinkCabFuelTank& InFuel,
        FPinkCabMovingFuelSession& InMovingFuel,
        FPinkCabCityDeltaState& InCityDeltas,
        FPinkCabStateKernel& InKernel,
        FPinkCabWorkdaySessionState& InWorkday);

    FPinkCabPassengerRegistry& Passengers;
    FPinkCabEconomyLedger& Economy;
    FPinkCabFareSettlementService& Settlement;
    FPinkCabFareLoopCoordinator& Fare;
    FPinkCabVehicleHealthState& Health;
    FPinkCabVehicleLoadState& Load;
    FPinkCabServiceContext& Service;
    FPinkCabServiceInventory& Inventory;
    FPinkCabVehicleBuild& Build;
    FPinkCabServiceOperationRuntime& Operations;
    FPinkCabFuelTank& Fuel;
    FPinkCabMovingFuelSession& MovingFuel;
    FPinkCabCityDeltaState& CityDeltas;
    FPinkCabStateKernel& Kernel;
    FPinkCabWorkdaySessionState& Workday;
};

class PINKCAB_API FPinkCabGamePersistenceCoordinator
{
public:
    static bool Capture(
        const FPinkCabCityIdentity& CityIdentity,
        FPinkCabGamePersistenceOwners& Owners,
        FPinkCabGameSnapshot& OutSnapshot);

    static bool Restore(
        const FPinkCabGameSnapshot& Snapshot,
        FPinkCabCityIdentity& OutCityIdentity,
        FPinkCabGamePersistenceOwners& Owners);

private:
    static bool CaptureRuntime(
        FPinkCabGamePersistenceOwners& Owners,
        FPinkCabGameSnapshot& Snapshot);
    static bool CaptureWorld(
        FPinkCabGamePersistenceOwners& Owners,
        FPinkCabGameSnapshot& Snapshot);
    static bool RestoreRuntime(
        const FPinkCabGameSnapshot& Snapshot,
        FPinkCabPassengerRegistry& Passengers,
        FPinkCabEconomyLedger& Economy,
        FPinkCabFareSettlementService& Settlement,
        FPinkCabFareLoopCoordinator& Fare,
        FPinkCabVehicleHealthState& Health,
        FPinkCabVehicleLoadState& Load);
    static bool RestoreServiceAndWorld(
        const FPinkCabGameSnapshot& Snapshot,
        FPinkCabServiceContext& Service,
        FPinkCabServiceInventory& Inventory,
        FPinkCabVehicleBuild& Build,
        FPinkCabServiceOperationRuntime& Operations,
        FPinkCabFuelTank& Fuel,
        FPinkCabMovingFuelSession& MovingFuel,
        FPinkCabCityDeltaState& CityDeltas,
        FPinkCabStateKernel& Kernel,
        FPinkCabWorkdaySessionState& Workday);
    static void CommitRestore(
        FPinkCabGamePersistenceOwners& Owners,
        FPinkCabPassengerRegistry&& Passengers,
        FPinkCabEconomyLedger&& Economy,
        FPinkCabFareSettlementService&& Settlement,
        FPinkCabFareLoopCoordinator&& Fare,
        FPinkCabVehicleHealthState&& Health,
        FPinkCabVehicleLoadState&& Load,
        FPinkCabServiceContext&& Service,
        FPinkCabServiceInventory&& Inventory,
        FPinkCabVehicleBuild&& Build,
        FPinkCabServiceOperationRuntime&& Operations,
        FPinkCabFuelTank&& Fuel,
        FPinkCabMovingFuelSession&& MovingFuel,
        FPinkCabCityDeltaState&& CityDeltas,
        FPinkCabStateKernel&& Kernel,
        FPinkCabWorkdaySessionState&& Workday);
};
