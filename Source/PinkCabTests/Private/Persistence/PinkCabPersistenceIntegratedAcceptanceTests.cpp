#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Persistence/PinkCabPersistenceService.h"
#include "Persistence/PinkCabRecoveryOrchestrator.h"

namespace PinkCabPersistenceAcceptance
{
FPinkCabSaveHeader Header(const FString& Schema = TEXT("core-1"))
{
    return FPinkCabSaveHeader::Create(
        TEXT("PINK-CAB"), Schema, TEXT("cfg-1"), TEXT("gen-1"), TEXT("content-1"));
}

FPinkCabCityIdentity City()
{
    return FPinkCabCityIdentity::Create(TEXT("FIRST-EURO"), TEXT("gen-1"), TEXT("content-1"));
}

FPinkCabMovingFuelPolicyInputs FuelPolicy()
{
    FPinkCabMovingFuelPolicyInputs P;
    P.QueuePolicyId = TEXT("queue.acceptance");
    P.SettlementPolicyId = TEXT("settlement.acceptance");
    P.InsufficientFundsPolicyId = TEXT("funds.acceptance");
    P.TargetLongitudinalGapCm = 40.0f;
    P.GapToleranceCm = 10.0f;
    P.MaxConnectionSpeedKmh = 20.0f;
    return P;
}
FPinkCabServiceContext ServiceContext(const FPinkCabCityIdentity& InCity)
{
    FPinkCabServiceContext C;
    C.City = InCity;
    C.SemanticKey = TEXT("garage:acceptance");
    C.ServiceNodeId = FPinkCabServiceContext::MakeNodeId(InCity, 1, C.SemanticKey);
    C.Owners.VehicleId = FPinkCabStableId(TEXT("vehicle:acceptance"));
    C.Owners.BuildOwnerId = FPinkCabStableId(TEXT("build:acceptance"));
    C.Owners.HealthOwnerId = FPinkCabStableId(TEXT("health:acceptance"));
    C.Owners.InventoryOwnerId = FPinkCabStableId(TEXT("inventory:acceptance"));
    C.Owners.EconomyOwnerId = FPinkCabStableId(TEXT("economy:acceptance"));
    return C;
}

FPinkCabPartDefinition Part()
{
    return {TEXT("part.acceptance.brake"), TEXT("FrontBrake"), 1200, TEXT("tatra")};
}

FPinkCabRoadGraph Graph(FPinkCabLaneId& OutLane)
{
    FPinkCabRoadGraph G;
    OutLane = FPinkCabLaneId(TEXT("lane:acceptance"));
    FPinkCabLogicalLane L;
    L.LaneId = OutLane;
    L.FromNode = FPinkCabRoadNodeId(TEXT("node:acceptance:a"));
    L.ToNode = FPinkCabRoadNodeId(TEXT("node:acceptance:b"));
    L.LengthCm = 500.0;
    G.AddLane(L);
    return G;
}
struct FFixture
{
    FPinkCabPassengerRegistry Passengers{16, 8, 8, 32, 8, 32};
    FPinkCabEconomyLedger Economy{1000, 0, 64};
    FPinkCabFareSettlementService Settlement{64};
    FPinkCabFareLoopCoordinator Fare;
    FPinkCabVehicleHealthState Health;
    FPinkCabVehicleLoadState Load;
    FPinkCabServiceContext Service;
    FPinkCabServiceInventory Inventory{8, 64};
    FPinkCabVehicleBuild Build{64};
    FPinkCabServiceOperationRuntime Operations{64};
    FPinkCabFuelTank Fuel{60.0f, 10.0f, 64};
    FPinkCabMovingFuelSession MovingFuel{FuelPolicy()};
    FPinkCabCityDeltaState CityDeltas{64};
    FPinkCabStateKernel Kernel{7777};
    FPinkCabWorkdaySessionState Workday;

    FPinkCabGamePersistenceOwners Owners()
    {
        return FPinkCabGamePersistenceOwners{
            Passengers, Economy, Settlement, Fare, Health, Load, Service,
            Inventory, Build, Operations, Fuel, MovingFuel, CityDeltas, Kernel, Workday};
    }
};

bool BuildFare(FFixture& F, bool bAwaitingPayment)
{
    FPinkCabOrder Order;
    Order.OrderId = FPinkCabStableId(TEXT("fare:acceptance"));
    Order.PickupId = FPinkCabStableId(TEXT("pickup:acceptance"));
    Order.DestinationId = FPinkCabStableId(TEXT("destination:acceptance"));
    const FPinkCabStableId PassengerId(TEXT("passenger:acceptance"));
    Order.PassengerIdentityIds.Add(PassengerId);
    Order.FareMode = EPinkCabFareMode::Metered;
    TArray<FPinkCabFarePassengerInput> Inputs = {{PassengerId, 70.0f}};
    FPinkCabFarePassengerManifest Manifest;
    if (!FPinkCabFarePassengerManifest::TryCreate(Order.OrderId, Inputs, Manifest)) return false;
    if (!FPinkCabFareLoopCoordinator::TryCreate(
            Order.OrderId, Order, FPinkCabFarePricingTerms::Metered(100, 100, 10), Manifest, F.Fare))
        return false;
    if (!F.Fare.TryAcceptPickup(true, true)) return false;
    if (!F.Fare.TryBoard(true, true, F.Load)) return false;
    if (!F.Fare.TryStartRide(true)) return false;
    F.Fare.TickFare(1.0, 60.0, false);
    if (bAwaitingPayment && !F.Fare.TryArrive(true, true)) return false;
    return true;
}

bool PopulateRich(FFixture& F, bool bAwaitingPayment)
{
    const FPinkCabCityIdentity C = City();
    F.Service = ServiceContext(C);
    if (!FPinkCabWorkdaySessionState::TryCreate(
            FPinkCabStableId(TEXT("workday:acceptance")), 3, 120.0, 32, F.Workday)) return false;

    FPinkCabPassengerRecord* Record = nullptr;
    if (!F.Passengers.TryCreate(
            FPinkCabStableId(TEXT("passenger:acceptance")),
            FPinkCabPassengerTemplate(TEXT("worker"), 70.0f), TEXT("acceptance"),
            {TEXT("music"), TEXT("quiet")}, Record)) return false;
    FPinkCabPassengerRelationship Delta;
    Delta.Trust = 0.2f;
    Delta.Satisfaction = 0.1f;
    if (Record->RegisterPaidFareOnce(
            FPinkCabStableId(TEXT("passenger:event:acceptance")), Delta)
        != EPinkCabPassengerMutationResult::Applied) return false;
    Record->SetNeuralPermission(true);
    if (Record->AddMessageOnce(
            FPinkCabStableId(TEXT("message:acceptance")), TEXT("hello again"))
        != EPinkCabPassengerMutationResult::Applied) return false;

    if (F.Inventory.AddOwnedPartOnce(
            FPinkCabStableId(TEXT("inventory:acceptance")), Part().PartId)
        != EPinkCabInventoryMutationResult::Applied) return false;
    if (F.Build.TryInstallPartOnce(
            FPinkCabStableId(TEXT("build:acceptance")), Part(), TEXT("tatra"))
        != EPinkCabPartInstallResult::Applied) return false;
    if (F.Fuel.CreditFuelOnce(
            FPinkCabTransactionId(TEXT("fuel:acceptance")), 5.0f)
        != EPinkCabFuelCreditResult::Applied) return false;

    FPinkCabLaneId Lane;
    FPinkCabRoadGraph G = Graph(Lane);
    FPinkCabVehicleTelemetry Telemetry;
    Telemetry.SpeedKmh = 10.0f;
    if (!F.MovingFuel.TryConnect(G, Telemetry, 40.0f, Lane)) return false;
    if (!F.MovingFuel.BeginFueling()) return false;
    if (F.Economy.Commit(FPinkCabEconomyTransaction::Credit(
            FPinkCabTransactionId(TEXT("tx:acceptance:seed")),
            EPinkCabTransactionType::FareIncome, 300))
        != EPinkCabSettlementResult::Committed) return false;
    F.Health.ApplyFunctionalDamage(EPinkCabVehicleHealthChannel::Brake, 0.35f);
    F.Load.SetFuelMassKg(30.0f, -20.0f);
    F.Load.SetCrew(60.0f, 45.0f);
    F.Load.AddPassenger(FPinkCabVehicleLoadItem(65.0f, 15.0f));

    FString DeltaId;
    if (!F.CityDeltas.TryAddLaneClosure(C, Lane, TEXT("closure:acceptance"), DeltaId)) return false;
    F.Kernel.NextSequence();
    F.Kernel.NextSequence();
    return BuildFare(F, bAwaitingPayment);
}

bool Capture(FFixture& F, FPinkCabGameSnapshot& Out)
{
    auto Owners = F.Owners();
    return FPinkCabGamePersistenceCoordinator::Capture(City(), Owners, Out);
}

uint64 PassengerSignature(const FFixture& F)
{
    return F.Passengers.GetReconstructionSignature();
}
} // namespace PinkCabPersistenceAcceptance

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabPersistenceAcceptanceRichRoundTripTest,
    "PinkCab.Persistence.Acceptance.RichRoundTrip",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPinkCabPersistenceAcceptanceRichRoundTripTest::RunTest(const FString& Parameters)
{
    using namespace PinkCabPersistenceAcceptance;
    FFixture F;
    TestTrue(TEXT("rich state populated"), PopulateRich(F, false));
    const uint64 PassengerBefore = PassengerSignature(F);
    const uint32 DamageBefore = F.Health.GetFunctionalDamageSerial();
    const float FuelBefore = F.Fuel.GetLiters();
    const int64 BalanceBefore = F.Economy.GetBalanceMinor();

    FPinkCabGameSnapshot Snapshot;
    TestTrue(TEXT("aggregate capture"), Capture(F, Snapshot));
    FPinkCabPersistenceService Service;
    TestTrue(TEXT("aggregate commit"), Service.CommitSnapshot(
        Header(), Snapshot, EPinkCabPersistenceCommitReason::PeriodicCheckpoint));

    F.Economy.Commit(FPinkCabEconomyTransaction::Credit(
        FPinkCabTransactionId(TEXT("tx:acceptance:mutation")),
        EPinkCabTransactionType::FareIncome, 999));
    F.Health.ApplyFunctionalDamage(EPinkCabVehicleHealthChannel::Alignment, 0.5f);
    F.Kernel.NextSequence();
    FPinkCabMigrationRegistry Migrations;
    FPinkCabCityIdentity RestoredCity;
    auto Owners = F.Owners();
    TestEqual(TEXT("recover directly into owners"), Service.RecoverLastCommittedInto(
        Header(), Migrations, RestoredCity, Owners), EPinkCabLoadResult::Success);
    TestEqual(TEXT("city identity restored"), RestoredCity.GetStableKey(), City().GetStableKey());
    TestEqual(TEXT("active fare restored"), F.Fare.GetState(), EPinkCabFareLoopState::Active);
    TestTrue(TEXT("fare passenger load restored"), F.Load.HasFarePassengerGroup(
        FPinkCabStableId(TEXT("fare:acceptance"))));
    TestEqual(TEXT("passenger social/neural signature"), PassengerSignature(F), PassengerBefore);
    TestEqual(TEXT("economy restored"), F.Economy.GetBalanceMinor(), BalanceBefore);
    TestEqual(TEXT("damage restored"), F.Health.GetFunctionalDamageSerial(), DamageBefore);
    TestTrue(TEXT("fuel restored"), FMath::IsNearlyEqual(F.Fuel.GetLiters(), FuelBefore));
    TestEqual(TEXT("inventory restored"), F.Inventory.GetQuantity(Part().PartId), 1);
    TestEqual(TEXT("build restored"), F.Build.GetInstalledPartId(Part().SlotId), Part().PartId);
    TestEqual(TEXT("moving refuel restored"), F.MovingFuel.GetState(), EPinkCabMovingFuelState::Fueling);
    TestTrue(TEXT("world delta restored"), F.CityDeltas.IsLaneClosed(FPinkCabLaneId(TEXT("lane:acceptance"))));
    TestEqual(TEXT("kernel sequence restored"), F.Kernel.NextSequence(), static_cast<uint64>(3));

    TestEqual(TEXT("inventory replay blocked"), F.Inventory.AddOwnedPartOnce(
        FPinkCabStableId(TEXT("inventory:acceptance")), Part().PartId),
        EPinkCabInventoryMutationResult::Duplicate);
    TestEqual(TEXT("build replay blocked"), F.Build.TryInstallPartOnce(
        FPinkCabStableId(TEXT("build:acceptance")), Part(), TEXT("tatra")),
        EPinkCabPartInstallResult::Duplicate);
    TestEqual(TEXT("fuel replay blocked"), F.Fuel.CreditFuelOnce(
        FPinkCabTransactionId(TEXT("fuel:acceptance")), 5.0f), EPinkCabFuelCreditResult::Duplicate);
    FPinkCabPassengerRecord* Record = F.Passengers.Find(FPinkCabStableId(TEXT("passenger:acceptance")));
    TestNotNull(TEXT("passenger restored"), Record);
    if (Record)
    {
        FPinkCabPassengerRelationship Delta;
        Delta.Trust = 0.2f;
        Delta.Satisfaction = 0.1f;
        TestEqual(TEXT("passenger event replay blocked"), Record->RegisterPaidFareOnce(
            FPinkCabStableId(TEXT("passenger:event:acceptance")), Delta),
            EPinkCabPassengerMutationResult::Duplicate);
        TestEqual(TEXT("neural replay blocked"), Record->AddMessageOnce(
            FPinkCabStableId(TEXT("message:acceptance")), TEXT("hello again")),
            EPinkCabPassengerMutationResult::Duplicate);
    }
    TestEqual(TEXT("economy transaction replay blocked"), F.Economy.Commit(
        FPinkCabEconomyTransaction::Credit(
            FPinkCabTransactionId(TEXT("tx:acceptance:seed")),
            EPinkCabTransactionType::FareIncome, 300)), EPinkCabSettlementResult::Duplicate);
    TestEqual(TEXT("replays do not change balance"), F.Economy.GetBalanceMinor(), BalanceBefore);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabPersistenceAcceptanceAwaitingPaymentReplayTest,
    "PinkCab.Persistence.Acceptance.AwaitingPaymentReplay",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPinkCabPersistenceAcceptanceAwaitingPaymentReplayTest::RunTest(const FString& Parameters)
{
    using namespace PinkCabPersistenceAcceptance;
    FFixture F;
    TestTrue(TEXT("awaiting-payment state populated"), PopulateRich(F, true));
    TestEqual(TEXT("fare awaiting payment"), F.Fare.GetState(), EPinkCabFareLoopState::AwaitingPayment);
    const int64 FareMinor = F.Fare.GetFareMinor();
    const FPinkCabFareSettlementResult FirstSettlement = F.Settlement.CommitPaid(
        FPinkCabStableId(TEXT("fare:acceptance")), FareMinor, 50, F.Economy);
    TestEqual(TEXT("crash-window settlement committed"), FirstSettlement.Disposition,
        EPinkCabFareSettlementDisposition::Committed);
    const int64 SettledBalance = F.Economy.GetBalanceMinor();

    FPinkCabGameSnapshot Snapshot;
    TestTrue(TEXT("capture crash-window state"), Capture(F, Snapshot));
    FPinkCabPersistenceService Service;
    TestTrue(TEXT("commit crash-window state"), Service.CommitSnapshot(
        Header(), Snapshot, EPinkCabPersistenceCommitReason::FareSettlement));
    F.Economy.Commit(FPinkCabEconomyTransaction::Credit(
        FPinkCabTransactionId(TEXT("tx:post-crash-mutation")),
        EPinkCabTransactionType::FareIncome, 999));

    FPinkCabMigrationRegistry Migrations;
    FPinkCabCityIdentity RestoredCity;
    auto Owners = F.Owners();
    TestEqual(TEXT("recover crash-window state"), Service.RecoverLastCommittedInto(
        Header(), Migrations, RestoredCity, Owners), EPinkCabLoadResult::Success);
    TestEqual(TEXT("settled balance restored"), F.Economy.GetBalanceMinor(), SettledBalance);
    TestEqual(TEXT("fare still awaiting after reload"), F.Fare.GetState(), EPinkCabFareLoopState::AwaitingPayment);
    TestTrue(TEXT("ordinary payment resumes via replay"), F.Fare.TryPay(50, F.Economy, F.Settlement));
    TestEqual(TEXT("fare becomes paid"), F.Fare.GetState(), EPinkCabFareLoopState::Paid);
    TestEqual(TEXT("replayed settlement does not double credit"), F.Economy.GetBalanceMinor(), SettledBalance);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabPersistenceAcceptanceCorruptAtomicMigrationTest,
    "PinkCab.Persistence.Acceptance.CorruptAtomicMigration",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPinkCabPersistenceAcceptanceCorruptAtomicMigrationTest::RunTest(const FString& Parameters)
{
    using namespace PinkCabPersistenceAcceptance;
    FFixture F;
    TestTrue(TEXT("rich state populated"), PopulateRich(F, false));
    FPinkCabGameSnapshot Snapshot;
    TestTrue(TEXT("capture"), Capture(F, Snapshot));
    TArray<uint8> Bytes;
    TestTrue(TEXT("serialize"), FPinkCabPersistenceService::Serialize(Header(), Snapshot, Bytes));

    F.Economy.Commit(FPinkCabEconomyTransaction::Credit(
        FPinkCabTransactionId(TEXT("tx:atomic-sentinel")),
        EPinkCabTransactionType::FareIncome, 777));
    const int64 SentinelBalance = F.Economy.GetBalanceMinor();
    const uint64 SentinelPassenger = PassengerSignature(F);
    TArray<uint8> Truncated = Bytes;
    Truncated.SetNum(FMath::Max(1, Bytes.Num() / 2));
    FPinkCabMigrationRegistry Migrations;
    FPinkCabCityIdentity RestoredCity;
    auto Owners = F.Owners();
    TestEqual(TEXT("truncated bytes rejected atomically"), FPinkCabPersistenceService::DeserializeInto(
        Truncated, Header(), Migrations, RestoredCity, Owners), EPinkCabLoadResult::CorruptPayload);
    TestEqual(TEXT("economy untouched after corrupt"), F.Economy.GetBalanceMinor(), SentinelBalance);
    TestEqual(TEXT("passenger untouched after corrupt"), PassengerSignature(F), SentinelPassenger);

    FPinkCabSaveHeader NextSchema = Header(TEXT("core-2"));
    TestEqual(TEXT("unknown schema rejected atomically"), FPinkCabPersistenceService::DeserializeInto(
        Bytes, NextSchema, Migrations, RestoredCity, Owners), EPinkCabLoadResult::IncompatibleSchema);
    TestEqual(TEXT("economy untouched after unknown schema"), F.Economy.GetBalanceMinor(), SentinelBalance);
    Migrations.RegisterBoundary(TEXT("core-1"), TEXT("core-2"));
    TestEqual(TEXT("registered boundary explicit"), FPinkCabPersistenceService::DeserializeInto(
        Bytes, NextSchema, Migrations, RestoredCity, Owners), EPinkCabLoadResult::MigrationRequired);
    TestEqual(TEXT("economy untouched for migration required"), F.Economy.GetBalanceMinor(), SentinelBalance);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabPersistenceAcceptanceCheckpointIdempotenceTest,
    "PinkCab.Persistence.Acceptance.CheckpointIdempotence",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPinkCabPersistenceAcceptanceCheckpointIdempotenceTest::RunTest(const FString& Parameters)
{
    using namespace PinkCabPersistenceAcceptance;
    FFixture F;
    TestTrue(TEXT("rich state populated"), PopulateRich(F, false));
    FPinkCabGameSnapshot Snapshot;
    TestTrue(TEXT("capture"), Capture(F, Snapshot));
    FPinkCabPersistenceService Service;
    for (int32 Index = 0; Index < 4; ++Index)
    {
        Snapshot.Economy.BalanceMinor = 2000 + Index;
        TestTrue(TEXT("rolling checkpoint commit"), Service.CommitSnapshot(
            Header(), Snapshot, EPinkCabPersistenceCommitReason::PeriodicCheckpoint));
    }
    TestEqual(TEXT("rolling history bounded"), Service.GetCheckpointRing().Num(),
        FPinkCabPersistencePolicy::RollingCheckpointCount);

    F.Economy.Commit(FPinkCabEconomyTransaction::Credit(
        FPinkCabTransactionId(TEXT("tx:checkpoint:mutation")),
        EPinkCabTransactionType::FareIncome, 500));
    FPinkCabMigrationRegistry Migrations;
    FPinkCabCityIdentity RestoredCity;
    auto Owners = F.Owners();
    TestEqual(TEXT("first recovery"), Service.RecoverLastCommittedInto(
        Header(), Migrations, RestoredCity, Owners), EPinkCabLoadResult::Success);
    const int64 FirstBalance = F.Economy.GetBalanceMinor();
    const uint64 FirstPassenger = PassengerSignature(F);
    TestEqual(TEXT("latest checkpoint balance"), FirstBalance, static_cast<int64>(2003));
    TestEqual(TEXT("second recovery"), Service.RecoverLastCommittedInto(
        Header(), Migrations, RestoredCity, Owners), EPinkCabLoadResult::Success);
    TestEqual(TEXT("repeated recovery balance identical"), F.Economy.GetBalanceMinor(), FirstBalance);
    TestEqual(TEXT("repeated recovery passenger identical"), PassengerSignature(F), FirstPassenger);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabPersistenceAcceptanceTerminalWorkdayTest,
    "PinkCab.Persistence.Acceptance.TerminalWorkday",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPinkCabPersistenceAcceptanceTerminalWorkdayTest::RunTest(const FString& Parameters)
{
    using namespace PinkCabPersistenceAcceptance;
    FFixture F;
    TestTrue(TEXT("active rich state populated"), PopulateRich(F, false));
    const uint32 DamageBefore = F.Health.GetFunctionalDamageSerial();
    const int64 BalanceBefore = F.Economy.GetBalanceMinor();
    const uint64 PassengerBefore = PassengerSignature(F);
    const int32 DeltasBefore = F.CityDeltas.Num();
    const int32 InventoryBefore = F.Inventory.GetQuantity(Part().PartId);
    auto Owners = F.Owners();
    TestTrue(TEXT("terminal recovery applies"), FPinkCabRecoveryOrchestrator::ApplyTerminalRecovery(Owners));
    TestEqual(TEXT("active fare failed"), F.Fare.GetState(), EPinkCabFareLoopState::Failed);
    TestTrue(TEXT("workday terminal marker"), F.Workday.WasTerminalRecovery());
    TestFalse(TEXT("fare passenger load removed"), F.Load.HasFarePassengerGroup(
        FPinkCabStableId(TEXT("fare:acceptance"))));
    TestEqual(TEXT("damage preserved"), F.Health.GetFunctionalDamageSerial(), DamageBefore);
    TestTrue(TEXT("summary committed"), F.Workday.MarkSummaryCommitted());
    TestTrue(TEXT("household transaction recorded"), F.Workday.RecordHouseholdTransactionOnce(
        FPinkCabStableId(TEXT("household:acceptance"))));
    TestTrue(TEXT("advance next workday"), FPinkCabRecoveryOrchestrator::AdvanceToNextWorkday(
        FPinkCabStableId(TEXT("workday:acceptance:next")), 4, 987654, Owners));

    TestEqual(TEXT("next workday ordinal"), F.Workday.GetOrdinal(), 4);
    TestFalse(TEXT("terminal marker reset"), F.Workday.WasTerminalRecovery());
    TestEqual(TEXT("fare runtime reset"), F.Fare.GetState(), EPinkCabFareLoopState::Offered);
    TestFalse(TEXT("reset fare uninitialized"), F.Fare.TryAcceptPickup(true, true));
    TestEqual(TEXT("kernel reset/reseed"), F.Kernel.GetRootSeed(), static_cast<uint64>(987654));
    TestEqual(TEXT("kernel sequence reset"), F.Kernel.NextSequence(), static_cast<uint64>(1));
    TestEqual(TEXT("economy campaign state preserved"), F.Economy.GetBalanceMinor(), BalanceBefore);
    TestEqual(TEXT("passenger campaign state preserved"), PassengerSignature(F), PassengerBefore);
    TestEqual(TEXT("damage campaign state preserved"), F.Health.GetFunctionalDamageSerial(), DamageBefore);
    TestEqual(TEXT("world deltas preserved"), F.CityDeltas.Num(), DeltasBefore);
    TestEqual(TEXT("inventory preserved"), F.Inventory.GetQuantity(Part().PartId), InventoryBefore);
    return true;
}

#endif
