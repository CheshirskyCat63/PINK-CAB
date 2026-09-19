#pragma once

#include "CoreMinimal.h"
#include "Persistence/PinkCabGameSnapshot.h"
#include "Persistence/PinkCabPassengerSnapshotCodec.h"
#include "Persistence/PinkCabServiceSnapshotCodec.h"

struct FPinkCabGamePersistenceOwners
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
        FPinkCabWorkdaySessionState& InWorkday)
        : Passengers(InPassengers), Economy(InEconomy), Settlement(InSettlement), Fare(InFare)
        , Health(InHealth), Load(InLoad), Service(InService), Inventory(InInventory)
        , Build(InBuild), Operations(InOperations), Fuel(InFuel), MovingFuel(InMovingFuel)
        , CityDeltas(InCityDeltas), Kernel(InKernel), Workday(InWorkday)
    {
    }

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

class FPinkCabGamePersistenceCoordinator
{
public:
    static bool Capture(
        const FPinkCabCityIdentity& CityIdentity,
        FPinkCabGamePersistenceOwners& Owners,
        FPinkCabGameSnapshot& OutSnapshot)
    {
        if (!CityIdentity.IsValid()
            || !Owners.Service.City.IsValid()
            || Owners.Service.City.GetStableKey() != CityIdentity.GetStableKey())
        {
            return false;
        }

        FPinkCabGameSnapshot Snapshot;
        Snapshot.CityIdentity = CityIdentity;
        if (!FPinkCabPassengerSnapshotCodec::Capture(Owners.Passengers, Snapshot.Passenger)
            || !FPinkCabEconomySnapshotCodec::Capture(Owners.Economy, Owners.Settlement, Snapshot.Economy)
            || !FPinkCabFareRuntimeSnapshotCodec::Capture(Owners.Fare, Snapshot.FareRuntime)
            || !FPinkCabVehicleSnapshotCodec::Capture(Owners.Health, Owners.Load, Snapshot.Vehicle)
            || !FPinkCabServiceSnapshotCodec::Capture(
                Owners.Service, Owners.Inventory, Owners.Build, Owners.Operations,
                Owners.Fuel, Owners.MovingFuel, Snapshot.Service))
        {
            return false;
        }
        if (!FPinkCabWorldSessionSnapshotCodec::CaptureCityDeltas(
                Owners.CityDeltas, Snapshot.CityDeltas)
            || !FPinkCabWorldSessionSnapshotCodec::CaptureKernel(
                Owners.Kernel, Snapshot.Kernel)
            || !FPinkCabWorldSessionSnapshotCodec::CaptureWorkday(
                Owners.Workday, Snapshot.Workday))
        {
            return false;
        }

        OutSnapshot = MoveTemp(Snapshot);
        return true;
    }

    static bool Restore(
        const FPinkCabGameSnapshot& Snapshot,
        FPinkCabCityIdentity& OutCityIdentity,
        FPinkCabGamePersistenceOwners& Owners)
    {
        if (Snapshot.SchemaVersion != FPinkCabGameSnapshot::CurrentSchemaVersion
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
        FPinkCabMovingFuelSession TempMovingFuel(Snapshot.Service.MovingFuel.Policy);
        FPinkCabCityDeltaState TempCityDeltas(1);
        FPinkCabStateKernel TempKernel(Snapshot.Kernel.RootSeed);
        FPinkCabWorkdaySessionState TempWorkday;

        if (!FPinkCabPassengerSnapshotCodec::Restore(Snapshot.Passenger, TempPassengers)
            || !FPinkCabEconomySnapshotCodec::Restore(
                Snapshot.Economy, TempEconomy, TempSettlement)
            || !FPinkCabFareRuntimeSnapshotCodec::Restore(Snapshot.FareRuntime, TempFare)
            || !FPinkCabVehicleSnapshotCodec::Restore(Snapshot.Vehicle, TempHealth, TempLoad))
        {
            return false;
        }
        if (!FPinkCabServiceSnapshotCodec::Restore(
                Snapshot.Service, TempService, TempInventory, TempBuild,
                TempOperations, TempFuel, TempMovingFuel)
            || !FPinkCabWorldSessionSnapshotCodec::RestoreCityDeltas(
                Snapshot.CityDeltas, TempCityDeltas)
            || !FPinkCabWorldSessionSnapshotCodec::RestoreKernel(
                Snapshot.Kernel, TempKernel)
            || !FPinkCabWorldSessionSnapshotCodec::RestoreWorkday(
                Snapshot.Workday, TempWorkday))
        {
            return false;
        }
        if (!TempService.City.IsValid()
            || TempService.City.GetStableKey() != Snapshot.CityIdentity.GetStableKey())
        {
            return false;
        }

        Owners.Passengers = MoveTemp(TempPassengers);
        Owners.Economy = MoveTemp(TempEconomy);
        Owners.Settlement = MoveTemp(TempSettlement);
        Owners.Fare = MoveTemp(TempFare);
        Owners.Health = MoveTemp(TempHealth);
        Owners.Load = MoveTemp(TempLoad);
        Owners.Service = MoveTemp(TempService);
        Owners.Inventory = MoveTemp(TempInventory);
        Owners.Build = MoveTemp(TempBuild);
        Owners.Operations = MoveTemp(TempOperations);
        Owners.Fuel = MoveTemp(TempFuel);
        Owners.MovingFuel = MoveTemp(TempMovingFuel);
        Owners.CityDeltas = MoveTemp(TempCityDeltas);
        Owners.Kernel = MoveTemp(TempKernel);
        Owners.Workday = MoveTemp(TempWorkday);
        OutCityIdentity = Snapshot.CityIdentity;
        return true;
    }
};
