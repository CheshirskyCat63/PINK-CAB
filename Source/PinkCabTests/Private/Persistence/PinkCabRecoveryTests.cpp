#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Persistence/PinkCabPersistenceService.h"
#include "Persistence/PinkCabRecoveryOrchestrator.h"

namespace PinkCabRecoveryTests
{
FPinkCabMovingFuelPolicyInputs FuelPolicy()
{
    FPinkCabMovingFuelPolicyInputs Policy;
    Policy.QueuePolicyId = TEXT("queue.recovery");
    Policy.SettlementPolicyId = TEXT("settlement.recovery");
    Policy.InsufficientFundsPolicyId = TEXT("funds.recovery");
    Policy.TargetLongitudinalGapCm = 40.0f;
    Policy.GapToleranceCm = 10.0f;
    Policy.MaxConnectionSpeedKmh = 20.0f;
    return Policy;
}

FPinkCabServiceContext ServiceContext(const FPinkCabCityIdentity& City)
{
    FPinkCabServiceContext Context;
    Context.City = City;
    Context.SemanticKey = TEXT("garage:recovery");
    Context.ServiceNodeId = FPinkCabServiceContext::MakeNodeId(City, 1, Context.SemanticKey);
    Context.Owners.VehicleId = FPinkCabStableId(TEXT("vehicle:recovery"));
    Context.Owners.BuildOwnerId = FPinkCabStableId(TEXT("build:recovery"));
    Context.Owners.HealthOwnerId = FPinkCabStableId(TEXT("health:recovery"));
    Context.Owners.InventoryOwnerId = FPinkCabStableId(TEXT("inventory:recovery"));
    Context.Owners.EconomyOwnerId = FPinkCabStableId(TEXT("economy:recovery"));
    return Context;
}

bool BuildFare(FPinkCabFareLoopCoordinator& OutFare)
{
    FPinkCabOrder Order;
    Order.OrderId = FPinkCabStableId(TEXT("fare:recovery"));
    Order.PickupId = FPinkCabStableId(TEXT("pickup:recovery"));
    Order.DestinationId = FPinkCabStableId(TEXT("destination:recovery"));
    Order.PassengerIdentityIds.Add(FPinkCabStableId(TEXT("passenger:recovery")));
    Order.FareMode = EPinkCabFareMode::Metered;
    TArray<FPinkCabFarePassengerInput> Inputs = {
        {FPinkCabStableId(TEXT("passenger:recovery")), 70.0f}};
    FPinkCabFarePassengerManifest Manifest;
    if (!FPinkCabFarePassengerManifest::TryCreate(Order.OrderId, Inputs, Manifest)) return false;
    return FPinkCabFareLoopCoordinator::TryCreate(
        Order.OrderId, Order, FPinkCabFarePricingTerms::Metered(100, 100, 10), Manifest, OutFare);
}
struct FFixture
{
    FPinkCabPassengerRegistry Passengers;
    FPinkCabEconomyLedger Economy{1000, 0, 64};
    FPinkCabFareSettlementService Settlement{64};
    FPinkCabFareLoopCoordinator Fare;
    FPinkCabVehicleHealthState Health;
    FPinkCabVehicleLoadState Load;
    FPinkCabServiceContext Service;
    FPinkCabServiceInventory Inventory{8, 64};
    FPinkCabVehicleBuild Build{64};
    FPinkCabServiceOperationRuntime Operations{64};
    FPinkCabFuelTank Fuel{60.0f, 15.0f, 64};
    FPinkCabMovingFuelSession MovingFuel{FuelPolicy()};
    FPinkCabCityDeltaState CityDeltas{64};
    FPinkCabStateKernel Kernel{9001};
    FPinkCabWorkdaySessionState Workday;

    FPinkCabGamePersistenceOwners Owners()
    {
        return FPinkCabGamePersistenceOwners{
            Passengers, Economy, Settlement, Fare, Health, Load, Service,
            Inventory, Build, Operations, Fuel, MovingFuel, CityDeltas, Kernel, Workday};
    }
};
bool Populate(FFixture& Fixture, const FPinkCabCityIdentity& City, bool bActiveFare)
{
    Fixture.Service = ServiceContext(City);
    if (!BuildFare(Fixture.Fare)) return false;
    if (!FPinkCabWorkdaySessionState::TryCreate(
        FPinkCabStableId(TEXT("workday:recovery")), 7, 60.0, 16, Fixture.Workday)) return false;
    if (Fixture.Economy.Commit(FPinkCabEconomyTransaction::Credit(
        FPinkCabTransactionId(TEXT("tx:recovery:fare")), EPinkCabTransactionType::FareIncome, 300))
        != EPinkCabSettlementResult::Committed) return false;
    Fixture.Health.ApplyFunctionalDamage(EPinkCabVehicleHealthChannel::Brake, 0.4f);
    Fixture.Kernel.NextSequence();
    if (bActiveFare)
    {
        if (!Fixture.Fare.TryAcceptPickup(true, true)) return false;
        if (!Fixture.Fare.TryBoard(true, true, Fixture.Load)) return false;
        if (!Fixture.Fare.TryStartRide(true)) return false;
        Fixture.Fare.TickFare(1.0, 30.0, false);
    }
    return true;
}

bool Capture(const FPinkCabCityIdentity& City, FFixture& Fixture, FPinkCabGameSnapshot& Out)
{
    auto Owners = Fixture.Owners();
    return FPinkCabGamePersistenceCoordinator::Capture(City, Owners, Out);
}
FPinkCabSaveHeader Header(const FString& Schema = TEXT("core-1"))
{
    return FPinkCabSaveHeader::Create(
        TEXT("PINK-CAB"), Schema, TEXT("cfg-1"), TEXT("gen-1"), TEXT("content-1"));
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabRecoveryEnvelopeTest,
    "PinkCab.Persistence.Recovery.EnvelopeCompatibility",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPinkCabRecoveryEnvelopeTest::RunTest(const FString& Parameters)
{
    const FPinkCabCityIdentity City = FPinkCabCityIdentity::Create(
        TEXT("FIRST-EURO"), TEXT("gen-1"), TEXT("content-1"));
    PinkCabRecoveryTests::FFixture Fixture;
    TestTrue(TEXT("populate"), PinkCabRecoveryTests::Populate(Fixture, City, false));
    FPinkCabGameSnapshot Snapshot;
    TestTrue(TEXT("capture"), PinkCabRecoveryTests::Capture(City, Fixture, Snapshot));
    TArray<uint8> Bytes;
    TestTrue(TEXT("serialize aggregate"), FPinkCabPersistenceService::Serialize(
        PinkCabRecoveryTests::Header(), Snapshot, Bytes));
    FPinkCabGameSnapshot Restored;
    FPinkCabMigrationRegistry Migrations;
    TestEqual(TEXT("round trip success"), FPinkCabPersistenceService::Deserialize(
        Bytes, PinkCabRecoveryTests::Header(), Migrations, Restored), EPinkCabLoadResult::Success);
    TestEqual(TEXT("city round trip"), Restored.CityIdentity.GetStableKey(), Snapshot.CityIdentity.GetStableKey());
    TestEqual(TEXT("economy round trip"), Restored.Economy.BalanceMinor, Snapshot.Economy.BalanceMinor);

    TArray<uint8> Truncated = Bytes;
    Truncated.SetNum(FMath::Max(1, Bytes.Num() / 2));
    FPinkCabGameSnapshot Untouched;
    TestEqual(TEXT("truncated payload rejected"), FPinkCabPersistenceService::Deserialize(
        Truncated, PinkCabRecoveryTests::Header(), Migrations, Untouched), EPinkCabLoadResult::CorruptPayload);

    FPinkCabSaveHeader WrongProduct = PinkCabRecoveryTests::Header();
    WrongProduct.ProductName = TEXT("OTHER");
    TestEqual(TEXT("product mismatch"), FPinkCabPersistenceService::Deserialize(
        Bytes, WrongProduct, Migrations, Untouched), EPinkCabLoadResult::IncompatibleProduct);
    FPinkCabSaveHeader WrongConfig = PinkCabRecoveryTests::Header();
    WrongConfig.ConfigVersion = TEXT("cfg-2");
    TestEqual(TEXT("config mismatch"), FPinkCabPersistenceService::Deserialize(
        Bytes, WrongConfig, Migrations, Untouched), EPinkCabLoadResult::IncompatibleConfig);
    FPinkCabSaveHeader WrongWorld = PinkCabRecoveryTests::Header();
    WrongWorld.GeneratorVersion = TEXT("gen-2");
    TestEqual(TEXT("world mismatch"), FPinkCabPersistenceService::Deserialize(
        Bytes, WrongWorld, Migrations, Untouched), EPinkCabLoadResult::IncompatibleWorldVersion);

    FPinkCabSaveHeader NextSchema = PinkCabRecoveryTests::Header(TEXT("core-2"));
    TestEqual(TEXT("unknown schema incompatible"), FPinkCabPersistenceService::Deserialize(
        Bytes, NextSchema, Migrations, Untouched), EPinkCabLoadResult::IncompatibleSchema);
    Migrations.RegisterBoundary(TEXT("core-1"), TEXT("core-2"));
    TestEqual(TEXT("registered boundary explicit"), FPinkCabPersistenceService::Deserialize(
        Bytes, NextSchema, Migrations, Untouched), EPinkCabLoadResult::MigrationRequired);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabRecoveryCheckpointTest,
    "PinkCab.Persistence.Recovery.Checkpoints",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPinkCabRecoveryCheckpointTest::RunTest(const FString& Parameters)
{
    const FPinkCabCityIdentity City = FPinkCabCityIdentity::Create(
        TEXT("FIRST-EURO"), TEXT("gen-1"), TEXT("content-1"));
    PinkCabRecoveryTests::FFixture Fixture;
    TestTrue(TEXT("populate"), PinkCabRecoveryTests::Populate(Fixture, City, false));
    FPinkCabPersistenceService Service;
    FPinkCabGameSnapshot Snapshot;
    TestTrue(TEXT("capture"), PinkCabRecoveryTests::Capture(City, Fixture, Snapshot));
    for (int32 Index = 0; Index < 4; ++Index)
    {
        Snapshot.Economy.BalanceMinor = 1300 + Index;
        TestTrue(TEXT("checkpoint commit"), Service.CommitSnapshot(
            PinkCabRecoveryTests::Header(), Snapshot, EPinkCabPersistenceCommitReason::PeriodicCheckpoint));
    }
    TestEqual(TEXT("checkpoint ring bounded"), Service.GetCheckpointRing().Num(),
        FPinkCabPersistencePolicy::RollingCheckpointCount);

    FPinkCabMigrationRegistry Migrations;
    FPinkCabGameSnapshot Latest;
    TestEqual(TEXT("last committed recovers"), Service.RecoverLastCommitted(
        PinkCabRecoveryTests::Header(), Migrations, Latest), EPinkCabLoadResult::Success);
    TestEqual(TEXT("latest balance"), Latest.Economy.BalanceMinor, static_cast<int64>(1303));
    FPinkCabGameSnapshot Previous;
    TestEqual(TEXT("selected previous checkpoint"), Service.RecoverCheckpoint(
        1, PinkCabRecoveryTests::Header(), Migrations, Previous), EPinkCabLoadResult::Success);
    TestEqual(TEXT("previous balance"), Previous.Economy.BalanceMinor, static_cast<int64>(1302));

    FPinkCabGameSnapshot Again;
    TestEqual(TEXT("repeated recovery succeeds"), Service.RecoverLastCommitted(
        PinkCabRecoveryTests::Header(), Migrations, Again), EPinkCabLoadResult::Success);
    TestEqual(TEXT("repeated recovery identical"), Again.Economy.BalanceMinor, Latest.Economy.BalanceMinor);
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabRecoveryTerminalWorkdayTest,
    "PinkCab.Persistence.Recovery.TerminalWorkday",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPinkCabRecoveryTerminalWorkdayTest::RunTest(const FString& Parameters)
{
    const FPinkCabCityIdentity City = FPinkCabCityIdentity::Create(
        TEXT("FIRST-EURO"), TEXT("gen-1"), TEXT("content-1"));
    PinkCabRecoveryTests::FFixture Fixture;
    TestTrue(TEXT("active state populated"), PinkCabRecoveryTests::Populate(Fixture, City, true));
    const uint32 DamageSerialBefore = Fixture.Health.GetFunctionalDamageSerial();
    TestTrue(TEXT("fare passenger group active"), Fixture.Load.HasFarePassengerGroup(
        FPinkCabStableId(TEXT("fare:recovery"))));

    auto Owners = Fixture.Owners();
    TestTrue(TEXT("terminal recovery applies"), FPinkCabRecoveryOrchestrator::ApplyTerminalRecovery(Owners));
    TestEqual(TEXT("fare explicitly failed"), Fixture.Fare.GetState(), EPinkCabFareLoopState::Failed);
    TestTrue(TEXT("workday marked terminal"), Fixture.Workday.WasTerminalRecovery());
    TestFalse(TEXT("fare passengers removed"), Fixture.Load.HasFarePassengerGroup(
        FPinkCabStableId(TEXT("fare:recovery"))));
    TestEqual(TEXT("damage preserved"), Fixture.Health.GetFunctionalDamageSerial(), DamageSerialBefore);
    TestTrue(TEXT("summary committed"), Fixture.Workday.MarkSummaryCommitted());
    TestTrue(TEXT("household transaction recorded"), Fixture.Workday.RecordHouseholdTransactionOnce(
        FPinkCabStableId(TEXT("household:recovery:1"))));
    TestTrue(TEXT("workday advances"), FPinkCabRecoveryOrchestrator::AdvanceToNextWorkday(
        FPinkCabStableId(TEXT("workday:recovery:next")), 8, 123456, Owners));
    TestEqual(TEXT("next workday id"), Fixture.Workday.GetWorkdayId().Serialize(),
        FString(TEXT("workday:recovery:next")));
    TestEqual(TEXT("next workday ordinal"), Fixture.Workday.GetOrdinal(), 8);
    TestFalse(TEXT("terminal marker reset"), Fixture.Workday.WasTerminalRecovery());
    TestEqual(TEXT("kernel reseeded"), Fixture.Kernel.GetRootSeed(), static_cast<uint64>(123456));
    TestEqual(TEXT("kernel sequence reset"), Fixture.Kernel.NextSequence(), static_cast<uint64>(1));
    TestEqual(TEXT("fare runtime reset"), Fixture.Fare.GetState(), EPinkCabFareLoopState::Offered);
    TestFalse(TEXT("uninitialized fare cannot accept pickup"), Fixture.Fare.TryAcceptPickup(true, true));
    return true;
}

#endif
