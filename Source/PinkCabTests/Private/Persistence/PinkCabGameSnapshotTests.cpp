#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Persistence/PinkCabGamePersistenceCoordinator.h"

namespace PinkCabGameSnapshotTests
{
FPinkCabMovingFuelPolicyInputs FuelPolicy()
{
    FPinkCabMovingFuelPolicyInputs Policy;
    Policy.QueuePolicyId = TEXT("queue.game-snapshot");
    Policy.SettlementPolicyId = TEXT("settlement.game-snapshot");
    Policy.InsufficientFundsPolicyId = TEXT("funds.game-snapshot");
    Policy.TargetLongitudinalGapCm = 40.0f;
    Policy.GapToleranceCm = 10.0f;
    Policy.MaxConnectionSpeedKmh = 20.0f;
    return Policy;
}

FPinkCabServiceContext ServiceContext(const FPinkCabCityIdentity& City)
{
    FPinkCabServiceContext Context;
    Context.City = City;
    Context.SemanticKey = TEXT("garage:game-snapshot");
    Context.ServiceNodeId = FPinkCabServiceContext::MakeNodeId(City, 1, Context.SemanticKey);
    Context.Owners.VehicleId = FPinkCabStableId(TEXT("vehicle:game-snapshot"));
    Context.Owners.BuildOwnerId = FPinkCabStableId(TEXT("build:game-snapshot"));
    Context.Owners.HealthOwnerId = FPinkCabStableId(TEXT("health:game-snapshot"));
    Context.Owners.InventoryOwnerId = FPinkCabStableId(TEXT("inventory:game-snapshot"));
    Context.Owners.EconomyOwnerId = FPinkCabStableId(TEXT("economy:game-snapshot"));
    return Context;
}

bool BuildFare(FPinkCabFareLoopCoordinator& OutFare)
{
    FPinkCabOrder Order;
    Order.OrderId = FPinkCabStableId(TEXT("fare:game-snapshot"));
    Order.PickupId = FPinkCabStableId(TEXT("pickup:game-snapshot"));
    Order.DestinationId = FPinkCabStableId(TEXT("destination:game-snapshot"));
    Order.PassengerIdentityIds.Add(FPinkCabStableId(TEXT("passenger:game-snapshot")));
    Order.FareMode = EPinkCabFareMode::Metered;
    TArray<FPinkCabFarePassengerInput> Inputs = {
        {FPinkCabStableId(TEXT("passenger:game-snapshot")), 72.0f}};
    FPinkCabFarePassengerManifest Manifest;
    if (!FPinkCabFarePassengerManifest::TryCreate(Order.OrderId, Inputs, Manifest)) return false;
    return FPinkCabFareLoopCoordinator::TryCreate(
        Order.OrderId, Order, FPinkCabFarePricingTerms::Metered(100, 200, 50), Manifest, OutFare);
}
struct FFixture
{
    FPinkCabPassengerRegistry Passengers;
    FPinkCabEconomyLedger Economy{1000, 0, 32};
    FPinkCabFareSettlementService Settlement{32};
    FPinkCabFareLoopCoordinator Fare;
    FPinkCabVehicleHealthState Health;
    FPinkCabVehicleLoadState Load;
    FPinkCabServiceContext Service;
    FPinkCabServiceInventory Inventory{8, 32};
    FPinkCabVehicleBuild Build{32};
    FPinkCabServiceOperationRuntime Operations{32};
    FPinkCabFuelTank Fuel{60.0f, 12.0f, 32};
    FPinkCabMovingFuelSession MovingFuel{FuelPolicy()};
    FPinkCabCityDeltaState CityDeltas{32};
    FPinkCabStateKernel Kernel{12345};
    FPinkCabWorkdaySessionState Workday;

    FPinkCabGamePersistenceOwners Owners()
    {
        return FPinkCabGamePersistenceOwners{
            Passengers, Economy, Settlement, Fare, Health, Load, Service,
            Inventory, Build, Operations, Fuel, MovingFuel, CityDeltas, Kernel, Workday};
    }
};
bool Populate(FFixture& Fixture, const FPinkCabCityIdentity& City)
{
    Fixture.Service = ServiceContext(City);
    if (!BuildFare(Fixture.Fare)) return false;
    if (Fixture.Economy.Commit(FPinkCabEconomyTransaction::Credit(
        FPinkCabTransactionId(TEXT("tx:game-snapshot")),
        EPinkCabTransactionType::FareIncome, 250)) != EPinkCabSettlementResult::Committed) return false;
    Fixture.Health.ApplyFunctionalDamage(EPinkCabVehicleHealthChannel::Brake, 0.25f);
    Fixture.Load.SetFuelMassKg(50.0f, -20.0f);
    Fixture.Kernel.NextSequence();
    FString DeltaId;
    if (!Fixture.CityDeltas.TryAddLaneClosure(
        City, FPinkCabLaneId(TEXT("lane:game-snapshot")), TEXT("closure:1"), DeltaId)) return false;
    if (!FPinkCabWorkdaySessionState::TryCreate(
        FPinkCabStableId(TEXT("workday:game-snapshot")), 4, 125.0, 16, Fixture.Workday)) return false;
    return true;
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabGameSnapshotRoundTripTest,
    "PinkCab.Persistence.GameSnapshot.RoundTrip",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPinkCabGameSnapshotRoundTripTest::RunTest(const FString& Parameters)
{
    const FPinkCabCityIdentity City = FPinkCabCityIdentity::Create(
        TEXT("FIRST-EURO"), TEXT("gen-1"), TEXT("content-1"));
    PinkCabGameSnapshotTests::FFixture Source;
    TestTrue(TEXT("source populated"), PinkCabGameSnapshotTests::Populate(Source, City));
    FPinkCabGameSnapshot Snapshot;
    auto SourceOwners = Source.Owners();
    TestTrue(TEXT("aggregate capture"), FPinkCabGamePersistenceCoordinator::Capture(
        City, SourceOwners, Snapshot));

    PinkCabGameSnapshotTests::FFixture Target;
    Target.Economy = FPinkCabEconomyLedger(7, 0, 4);
    Target.Health.ApplyFunctionalDamage(EPinkCabVehicleHealthChannel::Wheel, 0.5f);
    FPinkCabCityIdentity RestoredCity;
    auto TargetOwners = Target.Owners();
    TestTrue(TEXT("aggregate restore"), FPinkCabGamePersistenceCoordinator::Restore(
        Snapshot, RestoredCity, TargetOwners));
    TestEqual(TEXT("city restored"), RestoredCity.GetStableKey(), City.GetStableKey());
    TestEqual(TEXT("economy restored"), Target.Economy.GetBalanceMinor(), static_cast<int64>(1250));
    TestEqual(TEXT("fare restored"), Target.Fare.GetState(), EPinkCabFareLoopState::Offered);
    TestTrue(TEXT("brake damage restored"), FMath::IsNearlyEqual(
        Target.Health.GetHealth(EPinkCabVehicleHealthChannel::Brake), 0.75f));
    TestEqual(TEXT("service context restored"), Target.Service.ServiceNodeId, Source.Service.ServiceNodeId);
    TestEqual(TEXT("city deltas restored"), Target.CityDeltas.GetReconstructionSignature(),
        Source.CityDeltas.GetReconstructionSignature());
    TestEqual(TEXT("kernel root restored"), Target.Kernel.GetRootSeed(), Source.Kernel.GetRootSeed());
    TestEqual(TEXT("kernel sequence continues"), Target.Kernel.NextSequence(), static_cast<uint64>(2));
    TestEqual(TEXT("workday restored"), Target.Workday.GetWorkdayId().Serialize(),
        Source.Workday.GetWorkdayId().Serialize());
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabGameSnapshotAtomicRejectTest,
    "PinkCab.Persistence.GameSnapshot.AtomicReject",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPinkCabGameSnapshotAtomicRejectTest::RunTest(const FString& Parameters)
{
    const FPinkCabCityIdentity City = FPinkCabCityIdentity::Create(
        TEXT("FIRST-EURO"), TEXT("gen-1"), TEXT("content-1"));
    PinkCabGameSnapshotTests::FFixture Source;
    TestTrue(TEXT("source populated"), PinkCabGameSnapshotTests::Populate(Source, City));
    FPinkCabGameSnapshot Snapshot;
    auto SourceOwners = Source.Owners();
    TestTrue(TEXT("capture"), FPinkCabGamePersistenceCoordinator::Capture(City, SourceOwners, Snapshot));
    const FString DuplicateTxId = Snapshot.Economy.CommittedTransactionIds[0];
    Snapshot.Economy.CommittedTransactionIds.Add(DuplicateTxId);

    PinkCabGameSnapshotTests::FFixture Target;
    Target.Economy = FPinkCabEconomyLedger(777, 0, 4);
    const float WheelBefore = Target.Health.GetHealth(EPinkCabVehicleHealthChannel::Wheel);
    FPinkCabCityIdentity RestoredCity = FPinkCabCityIdentity::Create(
        TEXT("KEEP"), TEXT("keep-gen"), TEXT("keep-content"));
    auto TargetOwners = Target.Owners();
    TestFalse(TEXT("invalid child rejected"), FPinkCabGamePersistenceCoordinator::Restore(
        Snapshot, RestoredCity, TargetOwners));
    TestEqual(TEXT("economy unchanged"), Target.Economy.GetBalanceMinor(), static_cast<int64>(777));
    TestTrue(TEXT("vehicle unchanged"), FMath::IsNearlyEqual(
        Target.Health.GetHealth(EPinkCabVehicleHealthChannel::Wheel), WheelBefore));
    TestEqual(TEXT("city unchanged"), RestoredCity.GetCityCode(), FString(TEXT("KEEP")));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabGameSnapshotCityAuthorityTest,
    "PinkCab.Persistence.GameSnapshot.CityAuthority",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPinkCabGameSnapshotCityAuthorityTest::RunTest(const FString& Parameters)
{
    const FPinkCabCityIdentity City = FPinkCabCityIdentity::Create(
        TEXT("FIRST-EURO"), TEXT("gen-1"), TEXT("content-1"));
    PinkCabGameSnapshotTests::FFixture Source;
    TestTrue(TEXT("source populated"), PinkCabGameSnapshotTests::Populate(Source, City));
    FPinkCabGameSnapshot Snapshot;
    auto SourceOwners = Source.Owners();
    TestTrue(TEXT("capture"), FPinkCabGamePersistenceCoordinator::Capture(City, SourceOwners, Snapshot));
    Snapshot.CityIdentity = FPinkCabCityIdentity::Create(
        TEXT("OTHER-CITY"), TEXT("gen-1"), TEXT("content-1"));
    PinkCabGameSnapshotTests::FFixture Target;
    FPinkCabCityIdentity RestoredCity;
    auto TargetOwners = Target.Owners();
    TestFalse(TEXT("city/service mismatch rejected"), FPinkCabGamePersistenceCoordinator::Restore(
        Snapshot, RestoredCity, TargetOwners));
    TestFalse(TEXT("city remains invalid"), RestoredCity.IsValid());
    return true;
}

#endif
