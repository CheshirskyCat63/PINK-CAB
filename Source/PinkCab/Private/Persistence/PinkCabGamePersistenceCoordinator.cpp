#include "Persistence/PinkCabGamePersistenceCoordinator.h"

FPinkCabGamePersistenceOwners::FPinkCabGamePersistenceOwners(
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
    FPinkCabWorkdaySessionState& InWorkday)
    : Passengers(InPassengers)
    , Economy(InEconomy)
    , Settlement(InSettlement)
    , Fare(InFare)
    , Health(InHealth)
    , Load(InLoad)
    , Service(InService)
    , Inventory(InInventory)
    , Build(InBuild)
    , Operations(InOperations)
    , Fuel(InFuel)
    , MovingFuel(InMovingFuel)
    , CityDeltas(InCityDeltas)
    , Kernel(InKernel)
    , Workday(InWorkday)
{
}

bool FPinkCabGamePersistenceCoordinator::Capture(
    const FPinkCabCityIdentity& CityIdentity,
    FPinkCabGamePersistenceOwners& Owners,
    FPinkCabGameSnapshot& OutSnapshot)
{
    if (!CityIdentity.IsValid()
        || !Owners.Service.City.IsValid()
        || Owners.Service.City.GetStableKey()
            != CityIdentity.GetStableKey())
    {
        return false;
    }

    FPinkCabGameSnapshot Snapshot;
    Snapshot.CityIdentity = CityIdentity;
    if (!CaptureRuntime(Owners, Snapshot)
        || !CaptureWorld(Owners, Snapshot))
    {
        return false;
    }

    OutSnapshot = MoveTemp(Snapshot);
    return true;
}

bool FPinkCabGamePersistenceCoordinator::CaptureRuntime(
    FPinkCabGamePersistenceOwners& Owners,
    FPinkCabGameSnapshot& Snapshot)
{
    return FPinkCabPassengerSnapshotCodec::Capture(
            Owners.Passengers,
            Snapshot.Passenger)
        && FPinkCabEconomySnapshotCodec::Capture(
            Owners.Economy,
            Owners.Settlement,
            Snapshot.Economy)
        && FPinkCabFareRuntimeSnapshotCodec::Capture(
            Owners.Fare,
            Snapshot.FareRuntime)
        && FPinkCabVehicleSnapshotCodec::Capture(
            Owners.Health,
            Owners.Load,
            Snapshot.Vehicle)
        && FPinkCabServiceSnapshotCodec::Capture(
            Owners.Service,
            Owners.Inventory,
            Owners.Build,
            Owners.Operations,
            Owners.Fuel,
            Owners.MovingFuel,
            Snapshot.Service);
}

bool FPinkCabGamePersistenceCoordinator::CaptureWorld(
    FPinkCabGamePersistenceOwners& Owners,
    FPinkCabGameSnapshot& Snapshot)
{
    return FPinkCabWorldSessionSnapshotCodec::CaptureCityDeltas(
            Owners.CityDeltas,
            Snapshot.CityDeltas)
        && FPinkCabWorldSessionSnapshotCodec::CaptureKernel(
            Owners.Kernel,
            Snapshot.Kernel)
        && FPinkCabWorldSessionSnapshotCodec::CaptureWorkday(
            Owners.Workday,
            Snapshot.Workday);
}

bool FPinkCabGamePersistenceCoordinator::RestoreRuntime(
    const FPinkCabGameSnapshot& Snapshot,
    FPinkCabPassengerRegistry& Passengers,
    FPinkCabEconomyLedger& Economy,
    FPinkCabFareSettlementService& Settlement,
    FPinkCabFareLoopCoordinator& Fare,
    FPinkCabVehicleHealthState& Health,
    FPinkCabVehicleLoadState& Load)
{
    return FPinkCabPassengerSnapshotCodec::Restore(
            Snapshot.Passenger,
            Passengers)
        && FPinkCabEconomySnapshotCodec::Restore(
            Snapshot.Economy,
            Economy,
            Settlement)
        && FPinkCabFareRuntimeSnapshotCodec::Restore(
            Snapshot.FareRuntime,
            Fare)
        && FPinkCabVehicleSnapshotCodec::Restore(
            Snapshot.Vehicle,
            Health,
            Load);
}

bool FPinkCabGamePersistenceCoordinator::Restore(
    const FPinkCabGameSnapshot& Snapshot,
    FPinkCabCityIdentity& OutCityIdentity,
    FPinkCabGamePersistenceOwners& Owners)
{
    if (Snapshot.SchemaVersion
            != FPinkCabGameSnapshot::CurrentSchemaVersion
        || !Snapshot.CityIdentity.IsValid())
    {
        return false;
    }

    FPinkCabPassengerRegistry TempPassengers;
    FPinkCabEconomyLedger TempEconomy(0, 0, 1);
    FPinkCabFareSettlementService TempSettlement(1);
    FPinkCabFareLoopCoordinator TempFare;
    FPinkCabVehicleHealthState TempHealth;
    FPinkCabVehicleLoadState TempLoad;
    FPinkCabServiceContext TempService;
    FPinkCabServiceInventory TempInventory(1, 1);
    FPinkCabVehicleBuild TempBuild(1);
    FPinkCabServiceOperationRuntime TempOperations(1);
    FPinkCabFuelTank TempFuel(1.0f, 0.0f, 1);
    FPinkCabMovingFuelSession TempMovingFuel(
        Snapshot.Service.MovingFuel.Policy);
    FPinkCabCityDeltaState TempCityDeltas(1);
    FPinkCabStateKernel TempKernel(Snapshot.Kernel.RootSeed);
    FPinkCabWorkdaySessionState TempWorkday;

    if (!RestoreRuntime(
            Snapshot,
            TempPassengers,
            TempEconomy,
            TempSettlement,
            TempFare,
            TempHealth,
            TempLoad))
    {
        return false;
    }
    if (!RestoreServiceAndWorld(
            Snapshot,
            TempService,
            TempInventory,
            TempBuild,
            TempOperations,
            TempFuel,
            TempMovingFuel,
            TempCityDeltas,
            TempKernel,
            TempWorkday))
    {
        return false;
    }

    CommitRestore(
        Owners,
        MoveTemp(TempPassengers),
        MoveTemp(TempEconomy),
        MoveTemp(TempSettlement),
        MoveTemp(TempFare),
        MoveTemp(TempHealth),
        MoveTemp(TempLoad),
        MoveTemp(TempService),
        MoveTemp(TempInventory),
        MoveTemp(TempBuild),
        MoveTemp(TempOperations),
        MoveTemp(TempFuel),
        MoveTemp(TempMovingFuel),
        MoveTemp(TempCityDeltas),
        MoveTemp(TempKernel),
        MoveTemp(TempWorkday));
    OutCityIdentity = Snapshot.CityIdentity;
    return true;
}

bool FPinkCabGamePersistenceCoordinator::RestoreServiceAndWorld(
    const FPinkCabGameSnapshot& Snapshot,
    FPinkCabServiceContext& Service,
    FPinkCabServiceInventory& Inventory,
    FPinkCabVehicleBuild& Build,
    FPinkCabServiceOperationRuntime& Operations,
    FPinkCabFuelTank& Fuel,
    FPinkCabMovingFuelSession& MovingFuel,
    FPinkCabCityDeltaState& CityDeltas,
    FPinkCabStateKernel& Kernel,
    FPinkCabWorkdaySessionState& Workday)
{
    if (!FPinkCabServiceSnapshotCodec::Restore(
            Snapshot.Service,
            Service,
            Inventory,
            Build,
            Operations,
            Fuel,
            MovingFuel))
    {
        return false;
    }
    if (!FPinkCabWorldSessionSnapshotCodec::RestoreCityDeltas(
            Snapshot.CityDeltas,
            CityDeltas)
        || !FPinkCabWorldSessionSnapshotCodec::RestoreKernel(
            Snapshot.Kernel,
            Kernel)
        || !FPinkCabWorldSessionSnapshotCodec::RestoreWorkday(
            Snapshot.Workday,
            Workday))
    {
        return false;
    }
    return Service.City.IsValid()
        && Service.City.GetStableKey()
            == Snapshot.CityIdentity.GetStableKey();
}

void FPinkCabGamePersistenceCoordinator::CommitRestore(
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
    FPinkCabWorkdaySessionState&& Workday)
{
    Owners.Passengers = MoveTemp(Passengers);
    Owners.Economy = MoveTemp(Economy);
    Owners.Settlement = MoveTemp(Settlement);
    Owners.Fare = MoveTemp(Fare);
    Owners.Health = MoveTemp(Health);
    Owners.Load = MoveTemp(Load);
    Owners.Service = MoveTemp(Service);
    Owners.Inventory = MoveTemp(Inventory);
    Owners.Build = MoveTemp(Build);
    Owners.Operations = MoveTemp(Operations);
    Owners.Fuel = MoveTemp(Fuel);
    Owners.MovingFuel = MoveTemp(MovingFuel);
    Owners.CityDeltas = MoveTemp(CityDeltas);
    Owners.Kernel = MoveTemp(Kernel);
    Owners.Workday = MoveTemp(Workday);
}
