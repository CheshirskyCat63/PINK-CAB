#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Persistence/PinkCabServiceSnapshotCodec.h"
#include "Service/PinkCabServiceNode.h"
#include "Service/PinkCabServiceOperationRuntime.h"
#include "Service/PinkCabServiceSnapshot.h"

namespace PinkCabServiceIntegratedAcceptance
{
FPinkCabServiceOwnerIdentity Owners()
{
    FPinkCabServiceOwnerIdentity O;
    O.VehicleId = FPinkCabStableId(TEXT("vehicle:acceptance"));
    O.BuildOwnerId = FPinkCabStableId(TEXT("build:acceptance"));
    O.HealthOwnerId = FPinkCabStableId(TEXT("health:acceptance"));
    O.InventoryOwnerId = FPinkCabStableId(TEXT("inventory:acceptance"));
    O.EconomyOwnerId = FPinkCabStableId(TEXT("economy:acceptance"));
    return O;
}

FPinkCabCityIdentity City()
{
    return FPinkCabCityIdentity::Create(TEXT("FIRST-EURO"), TEXT("gen-1"), TEXT("content-1"));
}

FPinkCabPartDefinition Part()
{
    return {TEXT("part.acceptance.brake"), TEXT("FrontBrake"), 1200, TEXT("tatra")};
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

FPinkCabRoadGraph Graph(FPinkCabLaneId& OutLane)
{
    FPinkCabRoadGraph G;
    OutLane = FPinkCabLaneId(TEXT("lane:acceptance:fuel"));
    FPinkCabLogicalLane L;
    L.LaneId = OutLane;
    L.FromNode = FPinkCabRoadNodeId(TEXT("node:acceptance:a"));
    L.ToNode = FPinkCabRoadNodeId(TEXT("node:acceptance:b"));
    L.LengthCm = 1000.0;
    G.AddLane(L);
    return G;
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabServiceIntegratedLoopTest,
    "PinkCab.Service.Acceptance.FullLoop",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPinkCabServiceIntegratedLoopTest::RunTest(const FString& Parameters)
{
    FPinkCabServiceNode Node(EPinkCabServiceNodeKind::GarageTuning);
    const FPinkCabServiceOwnerIdentity Owners = PinkCabServiceIntegratedAcceptance::Owners();
    TestTrue(TEXT("bind context"), Node.BindContext(
        PinkCabServiceIntegratedAcceptance::City(), TEXT("garage:acceptance"), Owners));
    TestTrue(TEXT("eligible"), Node.MarkEligible());
    TestTrue(TEXT("enter"), Node.Enter());
    TestTrue(TEXT("active"), Node.Activate());

    FPinkCabEconomyLedger Ledger(20000, 5000);
    FPinkCabServiceInventory Inventory(8, 32);
    FPinkCabVehicleBuild Build(32);
    FPinkCabServiceOperationRuntime Operations(32);
    FPinkCabVehicleHealthState Health;
    FPinkCabFuelTank Tank(60.0f, 10.0f, 32);
    const FPinkCabPartDefinition Part = PinkCabServiceIntegratedAcceptance::Part();

    const FPinkCabStableId ParkingOp(TEXT("op:acceptance:parking"));
    const FPinkCabStableId PurchaseOp(TEXT("op:acceptance:purchase"));
    const FPinkCabStableId InstallOp(TEXT("op:acceptance:install"));
    const FPinkCabStableId RepairOp(TEXT("op:acceptance:repair"));

    TestEqual(TEXT("parking applied"), Operations.ChargeParking(Ledger, ParkingOp, 500),
        EPinkCabServiceOperationResult::Applied);
    TestEqual(TEXT("part purchased"), Operations.PurchasePart(Ledger, Inventory, PurchaseOp, Part),
        EPinkCabServiceOperationResult::Applied);
    TestEqual(TEXT("part installed"), Operations.InstallOwnedPart(
        Inventory, Build, InstallOp, Part, TEXT("tatra")), EPinkCabServiceOperationResult::Applied);
    TestTrue(TEXT("damage applied"), Health.ApplyFunctionalDamage(
        EPinkCabVehicleHealthChannel::Brake, 0.6f));
    TArray<FPinkCabRepairLine> RepairLines;
    RepairLines.Add({EPinkCabVehicleHealthChannel::Brake, 0.8f});
    TestEqual(TEXT("repair applied"), Operations.Repair(
        Ledger, Health, RepairOp, 800,
        EPinkCabEconomyPurpose::MinimumRoadworthyRepair, RepairLines),
        EPinkCabServiceOperationResult::Applied);

    TestTrue(TEXT("begin commit"), Node.BeginCommit());
    TestTrue(TEXT("exit"), Node.Exit());
    TestTrue(TEXT("resume world"), Node.ResumeWorld());
    TestTrue(TEXT("owner continuity"), Node.HasSameOwners(Owners));

    FPinkCabLaneId Lane;
    FPinkCabRoadGraph Graph = PinkCabServiceIntegratedAcceptance::Graph(Lane);
    FPinkCabMovingFuelSession Fuel(PinkCabServiceIntegratedAcceptance::FuelPolicy());
    FPinkCabVehicleTelemetry Telemetry;
    Telemetry.SpeedKmh = 10.0f;
    TestTrue(TEXT("moving fuel connects"), Fuel.TryConnect(Graph, Telemetry, 40.0f, Lane));
    TestTrue(TEXT("moving fuel begins"), Fuel.BeginFueling());

    const FPinkCabTransactionId FuelTx(TEXT("tx:acceptance:fuel"));
    const FPinkCabEconomyTransaction FuelPurchase = FPinkCabEconomyTransaction::Debit(
        FuelTx, EPinkCabTransactionType::FuelPurchase, 600,
        EPinkCabEconomyPurpose::OrdinaryPurchase);
    TestEqual(TEXT("fuel settlement"), Ledger.Commit(FuelPurchase),
        EPinkCabSettlementResult::Committed);
    TestEqual(TEXT("fuel credited"), Fuel.ApplySettledFuel(
        Tank, FuelTx, 6.0f, EPinkCabSettlementResult::Committed),
        EPinkCabFuelCreditResult::Applied);
    TestTrue(TEXT("fuel completes"), Fuel.Complete());

    TestEqual(TEXT("final balance"), Ledger.GetBalanceMinor(), static_cast<int64>(16900));
    TestEqual(TEXT("inventory consumed"), Inventory.GetQuantity(Part.PartId), 0);
    TestEqual(TEXT("build installed"), Build.GetInstalledPartId(Part.SlotId), Part.PartId);
    TestTrue(TEXT("brake repaired"), FMath::IsNearlyEqual(
        Health.GetHealth(EPinkCabVehicleHealthChannel::Brake), 0.8f));
    TestTrue(TEXT("fuel final"), FMath::IsNearlyEqual(Tank.GetLiters(), 16.0f));

    FPinkCabServiceSnapshot Snapshot;
    TestTrue(TEXT("snapshot capture"), FPinkCabServiceSnapshotCodec::Capture(
        Node.GetContext(), Inventory, Build, Operations, Tank, Fuel, Snapshot));

    FPinkCabServiceContext RestoredContext;
    FPinkCabServiceInventory RestoredInventory(1, 1);
    FPinkCabVehicleBuild RestoredBuild(1);
    FPinkCabServiceOperationRuntime RestoredOperations(1);
    FPinkCabFuelTank RestoredTank(1.0f, 0.0f, 1);
    FPinkCabMovingFuelSession RestoredFuel(PinkCabServiceIntegratedAcceptance::FuelPolicy());
    TestTrue(TEXT("snapshot restore"), FPinkCabServiceSnapshotCodec::Restore(
        Snapshot, RestoredContext, RestoredInventory, RestoredBuild,
        RestoredOperations, RestoredTank, RestoredFuel));
    FPinkCabEconomyLedger ReplayLedger(20000, 5000);
    const uint32 DamageSerialBeforeReplay = Health.GetFunctionalDamageSerial();
    TestEqual(TEXT("parking replay blocked"), RestoredOperations.ChargeParking(
        ReplayLedger, ParkingOp, 500), EPinkCabServiceOperationResult::Duplicate);
    TestEqual(TEXT("purchase replay blocked"), RestoredOperations.PurchasePart(
        ReplayLedger, RestoredInventory, PurchaseOp, Part), EPinkCabServiceOperationResult::Duplicate);
    TestEqual(TEXT("install replay blocked"), RestoredOperations.InstallOwnedPart(
        RestoredInventory, RestoredBuild, InstallOp, Part, TEXT("tatra")),
        EPinkCabServiceOperationResult::Duplicate);
    TestEqual(TEXT("repair replay blocked"), RestoredOperations.Repair(
        ReplayLedger, Health, RepairOp, 800,
        EPinkCabEconomyPurpose::MinimumRoadworthyRepair, RepairLines),
        EPinkCabServiceOperationResult::Duplicate);
    TestEqual(TEXT("replay ledger untouched"), ReplayLedger.GetBalanceMinor(), static_cast<int64>(20000));
    TestEqual(TEXT("repair replay no mutation"), Health.GetFunctionalDamageSerial(), DamageSerialBeforeReplay);
    TestEqual(TEXT("fuel replay blocked"), RestoredTank.CreditFuelOnce(FuelTx, 6.0f),
        EPinkCabFuelCreditResult::Duplicate);
    TestTrue(TEXT("fuel replay no mutation"), FMath::IsNearlyEqual(RestoredTank.GetLiters(), 16.0f));
    TestEqual(TEXT("context identity restored"), RestoredContext.ServiceNodeId, Node.GetServiceNodeId());
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabServiceMovingFuelFailureAcceptanceTest,
    "PinkCab.Service.Acceptance.MovingFuelFailures",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPinkCabServiceMovingFuelFailureAcceptanceTest::RunTest(const FString& Parameters)
{
    FPinkCabLaneId Lane;
    FPinkCabRoadGraph Graph = PinkCabServiceIntegratedAcceptance::Graph(Lane);
    FPinkCabVehicleTelemetry Telemetry;
    Telemetry.SpeedKmh = 10.0f;
    const FPinkCabTransactionId Tx(TEXT("tx:failure:no-fuel"));

    FPinkCabFuelTank Tank(60.0f, 10.0f, 16);
    FPinkCabMovingFuelSession Tolerance(PinkCabServiceIntegratedAcceptance::FuelPolicy());
    TestTrue(TEXT("tolerance connect"), Tolerance.TryConnect(Graph, Telemetry, 40.0f, Lane));
    TestTrue(TEXT("tolerance begin"), Tolerance.BeginFueling());
    TestFalse(TEXT("tolerance lost"), Tolerance.MaintainConnection(Graph, Telemetry, 80.0f));
    TestEqual(TEXT("tolerance reason"), Tolerance.GetAbortReason(),
        EPinkCabMovingFuelAbortReason::ToleranceLost);
    TestEqual(TEXT("tolerance abort blocks fuel"), Tolerance.ApplySettledFuel(
        Tank, Tx, 5.0f, EPinkCabSettlementResult::Committed), EPinkCabFuelCreditResult::InvalidState);

    FPinkCabMovingFuelSession Collision(PinkCabServiceIntegratedAcceptance::FuelPolicy());
    TestTrue(TEXT("collision connect"), Collision.TryConnect(Graph, Telemetry, 40.0f, Lane));
    TestTrue(TEXT("collision begin"), Collision.BeginFueling());
    TestTrue(TEXT("collision abort"), Collision.NotifyCollision());
    TestEqual(TEXT("collision reason"), Collision.GetAbortReason(), EPinkCabMovingFuelAbortReason::Collision);

    FPinkCabMovingFuelSession Explicit(PinkCabServiceIntegratedAcceptance::FuelPolicy());
    TestTrue(TEXT("explicit connect"), Explicit.TryConnect(Graph, Telemetry, 40.0f, Lane));
    TestTrue(TEXT("explicit begin"), Explicit.BeginFueling());
    TestTrue(TEXT("explicit abort"), Explicit.Abort());
    TestEqual(TEXT("explicit reason"), Explicit.GetAbortReason(), EPinkCabMovingFuelAbortReason::Explicit);
    TestEqual(TEXT("collision blocks fuel"), Collision.ApplySettledFuel(
        Tank, FPinkCabTransactionId(TEXT("tx:failure:collision")), 5.0f,
        EPinkCabSettlementResult::Committed), EPinkCabFuelCreditResult::InvalidState);
    TestEqual(TEXT("explicit blocks fuel"), Explicit.ApplySettledFuel(
        Tank, FPinkCabTransactionId(TEXT("tx:failure:explicit")), 5.0f,
        EPinkCabSettlementResult::Committed), EPinkCabFuelCreditResult::InvalidState);
    TestTrue(TEXT("all aborts leave tank unchanged"), FMath::IsNearlyEqual(Tank.GetLiters(), 10.0f));

    FPinkCabMovingFuelSession InvalidLane(PinkCabServiceIntegratedAcceptance::FuelPolicy());
    TestFalse(TEXT("unknown lane cannot connect"), InvalidLane.TryConnect(
        Graph, Telemetry, 40.0f, FPinkCabLaneId(TEXT("lane:missing"))));
    TestEqual(TEXT("invalid lane stays disconnected"), InvalidLane.GetState(),
        EPinkCabMovingFuelState::Disconnected);
    return true;
}

#endif
