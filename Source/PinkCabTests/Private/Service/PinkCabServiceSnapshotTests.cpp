#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Service/PinkCabServiceSnapshot.h"

namespace PinkCabServiceSnapshotTests
{
FPinkCabServiceContext Context()
{
    FPinkCabServiceContext C;
    C.City = FPinkCabCityIdentity::Create(TEXT("FIRST-EURO"), TEXT("gen-1"), TEXT("content-1"));
    C.SemanticKey = TEXT("garage:snapshot");
    C.ServiceNodeId = FPinkCabServiceContext::MakeNodeId(C.City, 1, C.SemanticKey);
    C.Owners.VehicleId = FPinkCabStableId(TEXT("vehicle:snapshot"));
    C.Owners.BuildOwnerId = FPinkCabStableId(TEXT("build:snapshot"));
    C.Owners.HealthOwnerId = FPinkCabStableId(TEXT("health:snapshot"));
    C.Owners.InventoryOwnerId = FPinkCabStableId(TEXT("inventory:snapshot"));
    C.Owners.EconomyOwnerId = FPinkCabStableId(TEXT("economy:snapshot"));
    return C;
}

FPinkCabPartDefinition Part()
{
    return {TEXT("part.snapshot.brake"), TEXT("FrontBrake"), 1200, TEXT("tatra")};
}

FPinkCabMovingFuelPolicyInputs FuelPolicy()
{
    FPinkCabMovingFuelPolicyInputs P;
    P.QueuePolicyId = TEXT("queue.snapshot");
    P.SettlementPolicyId = TEXT("settlement.snapshot");
    P.InsufficientFundsPolicyId = TEXT("funds.snapshot");    P.TargetLongitudinalGapCm = 40.0f;
    P.GapToleranceCm = 10.0f;
    P.MaxConnectionSpeedKmh = 20.0f;
    return P;
}

FPinkCabRoadGraph Graph(FPinkCabLaneId& OutLane)
{
    FPinkCabRoadGraph G;
    OutLane = FPinkCabLaneId(TEXT("lane:snapshot:fuel"));
    FPinkCabLogicalLane L;
    L.LaneId = OutLane;
    L.FromNode = FPinkCabRoadNodeId(TEXT("node:snapshot:a"));
    L.ToNode = FPinkCabRoadNodeId(TEXT("node:snapshot:b"));
    L.LengthCm = 500.0;
    G.AddLane(L);
    return G;
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabServiceSnapshotRoundTripTest,
    "PinkCab.Service.Snapshot.RoundTrip",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPinkCabServiceSnapshotRoundTripTest::RunTest(const FString& Parameters)
{
    const FPinkCabServiceContext Context = PinkCabServiceSnapshotTests::Context();
    const FPinkCabPartDefinition Part = PinkCabServiceSnapshotTests::Part();
    FPinkCabServiceInventory Inventory(8, 16);
    FPinkCabVehicleBuild Build(16);
    FPinkCabServiceOperationRuntime Operations(16);    FPinkCabEconomyLedger Ledger(10000, 0);
    FPinkCabFuelTank Tank(60.0f, 10.0f, 16);
    FPinkCabMovingFuelSession Fuel(PinkCabServiceSnapshotTests::FuelPolicy());

    const FPinkCabStableId PurchaseOp(TEXT("op:purchase:snapshot"));
    const FPinkCabStableId InstallOp(TEXT("op:install:snapshot"));
    const FPinkCabStableId ParkingOp(TEXT("op:parking:snapshot"));
    TestEqual(TEXT("inventory purchase recorded"), Inventory.AddOwnedPartOnce(PurchaseOp, Part.PartId),
        EPinkCabInventoryMutationResult::Applied);
    TestEqual(TEXT("build install recorded"), Build.TryInstallPartOnce(InstallOp, Part, TEXT("tatra")),
        EPinkCabPartInstallResult::Applied);
    TestEqual(TEXT("parking op recorded"), Operations.ChargeParking(Ledger, ParkingOp, 500),
        EPinkCabServiceOperationResult::Applied);
    const FPinkCabTransactionId FuelTx(TEXT("tx:fuel:snapshot"));
    TestEqual(TEXT("fuel credit recorded"), Tank.CreditFuelOnce(FuelTx, 5.0f), EPinkCabFuelCreditResult::Applied);

    FPinkCabLaneId Lane;
    FPinkCabRoadGraph Graph = PinkCabServiceSnapshotTests::Graph(Lane);
    FPinkCabVehicleTelemetry Telemetry;
    Telemetry.SpeedKmh = 10.0f;
    TestTrue(TEXT("moving fuel connected"), Fuel.TryConnect(Graph, Telemetry, 40.0f, Lane));
    TestTrue(TEXT("moving fuel active"), Fuel.BeginFueling());

    FPinkCabServiceSnapshot Snapshot;
    TestTrue(TEXT("capture succeeds"), FPinkCabServiceSnapshotCodec::Capture(
        Context, Inventory, Build, Operations, Tank, Fuel, Snapshot));
    FPinkCabServiceContext RestoredContext;
    FPinkCabServiceInventory RestoredInventory(1, 1);
    FPinkCabVehicleBuild RestoredBuild(1);
    FPinkCabServiceOperationRuntime RestoredOperations(1);
    FPinkCabFuelTank RestoredTank(1.0f, 0.0f, 1);
    FPinkCabMovingFuelSession RestoredFuel(PinkCabServiceSnapshotTests::FuelPolicy());
    TestTrue(TEXT("restore succeeds"), FPinkCabServiceSnapshotCodec::Restore(
        Snapshot, RestoredContext, RestoredInventory, RestoredBuild,
        RestoredOperations, RestoredTank, RestoredFuel));

    TestEqual(TEXT("context id restored"), RestoredContext.ServiceNodeId, Context.ServiceNodeId);
    TestEqual(TEXT("inventory restored"), RestoredInventory.GetQuantity(Part.PartId), 1);
    TestEqual(TEXT("build restored"), RestoredBuild.GetInstalledPartId(Part.SlotId), Part.PartId);
    TestTrue(TEXT("operation journal restored"), RestoredOperations.IsCompleted(ParkingOp));
    TestTrue(TEXT("fuel liters restored"), FMath::IsNearlyEqual(RestoredTank.GetLiters(), 15.0f));
    TestEqual(TEXT("moving state restored"), RestoredFuel.GetState(), EPinkCabMovingFuelState::Fueling);
    TestEqual(TEXT("moving lane restored"), RestoredFuel.GetServiceLaneId().Serialize(), Lane.Serialize());
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabServiceSnapshotReplayTest,
    "PinkCab.Service.Snapshot.ReplayGuards",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPinkCabServiceSnapshotReplayTest::RunTest(const FString& Parameters)
{
    const FPinkCabPartDefinition Part = PinkCabServiceSnapshotTests::Part();
    FPinkCabServiceInventory Inventory(8, 16);
    FPinkCabVehicleBuild Build(16);
    FPinkCabServiceOperationRuntime Operations(16);    FPinkCabEconomyLedger Ledger(10000, 0);
    FPinkCabFuelTank Tank(60.0f, 10.0f, 16);
    FPinkCabMovingFuelSession Fuel(PinkCabServiceSnapshotTests::FuelPolicy());
    const FPinkCabStableId PurchaseOp(TEXT("op:purchase:replay"));
    const FPinkCabStableId InstallOp(TEXT("op:install:replay"));
    const FPinkCabStableId ParkingOp(TEXT("op:parking:replay"));
    const FPinkCabTransactionId FuelTx(TEXT("tx:fuel:replay"));
    Inventory.AddOwnedPartOnce(PurchaseOp, Part.PartId);
    Build.TryInstallPartOnce(InstallOp, Part, TEXT("tatra"));
    Operations.ChargeParking(Ledger, ParkingOp, 500);
    Tank.CreditFuelOnce(FuelTx, 5.0f);

    FPinkCabServiceSnapshot Snapshot;
    TestTrue(TEXT("capture"), FPinkCabServiceSnapshotCodec::Capture(
        PinkCabServiceSnapshotTests::Context(), Inventory, Build, Operations, Tank, Fuel, Snapshot));

    FPinkCabServiceContext RC;
    FPinkCabServiceInventory RI(1, 1);
    FPinkCabVehicleBuild RB(1);
    FPinkCabServiceOperationRuntime RO(1);
    FPinkCabFuelTank RT(1.0f, 0.0f, 1);
    FPinkCabMovingFuelSession RF(PinkCabServiceSnapshotTests::FuelPolicy());
    TestTrue(TEXT("restore"), FPinkCabServiceSnapshotCodec::Restore(
        Snapshot, RC, RI, RB, RO, RT, RF));
    TestEqual(TEXT("inventory replay blocked"), RI.AddOwnedPartOnce(PurchaseOp, Part.PartId),
        EPinkCabInventoryMutationResult::Duplicate);
    TestEqual(TEXT("build replay blocked"), RB.TryInstallPartOnce(InstallOp, Part, TEXT("tatra")),
        EPinkCabPartInstallResult::Duplicate);
    FPinkCabEconomyLedger FreshLedger(10000, 0);
    TestEqual(TEXT("service replay blocked before fresh ledger debit"), RO.ChargeParking(FreshLedger, ParkingOp, 500),
        EPinkCabServiceOperationResult::Duplicate);
    TestEqual(TEXT("fresh ledger untouched"), FreshLedger.GetBalanceMinor(), static_cast<int64>(10000));
    TestEqual(TEXT("fuel replay blocked"), RT.CreditFuelOnce(FuelTx, 5.0f), EPinkCabFuelCreditResult::Duplicate);
    TestTrue(TEXT("fuel not duplicated"), FMath::IsNearlyEqual(RT.GetLiters(), 15.0f));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabServiceSnapshotInvalidSchemaTest,
    "PinkCab.Service.Snapshot.InvalidSchema",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPinkCabServiceSnapshotInvalidSchemaTest::RunTest(const FString& Parameters)
{
    FPinkCabServiceSnapshot Snapshot;
    Snapshot.SchemaVersion = FPinkCabServiceSnapshot::CurrentSchemaVersion + 1;
    FPinkCabServiceContext Context;
    FPinkCabServiceInventory Inventory(4, 4);
    FPinkCabVehicleBuild Build(4);
    FPinkCabServiceOperationRuntime Operations(4);
    FPinkCabFuelTank Tank(60.0f, 10.0f, 4);
    FPinkCabMovingFuelSession Fuel(PinkCabServiceSnapshotTests::FuelPolicy());
    TestFalse(TEXT("unknown schema rejected"), FPinkCabServiceSnapshotCodec::Restore(
        Snapshot, Context, Inventory, Build, Operations, Tank, Fuel));
    TestEqual(TEXT("tank unchanged"), Tank.GetLiters(), 10.0f);
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabServiceReplayJournalBoundsTest,
    "PinkCab.Service.Snapshot.ReplayJournalBounds",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPinkCabServiceReplayJournalBoundsTest::RunTest(const FString& Parameters)
{
    const FPinkCabPartDefinition Part = PinkCabServiceSnapshotTests::Part();
    FPinkCabServiceInventory Inventory(8, 1);
    TestEqual(TEXT("first inventory op applies"), Inventory.AddOwnedPartOnce(
        FPinkCabStableId(TEXT("inv:1")), Part.PartId), EPinkCabInventoryMutationResult::Applied);
    TestEqual(TEXT("inventory journal bounded"), Inventory.AddOwnedPartOnce(
        FPinkCabStableId(TEXT("inv:2")), Part.PartId), EPinkCabInventoryMutationResult::CapacityExceeded);

    FPinkCabVehicleBuild Build(1);
    TestEqual(TEXT("first build op applies"), Build.TryInstallPartOnce(
        FPinkCabStableId(TEXT("build:1")), Part, TEXT("tatra")), EPinkCabPartInstallResult::Applied);
    TestEqual(TEXT("build journal bounded"), Build.TryInstallPartOnce(
        FPinkCabStableId(TEXT("build:2")), Part, TEXT("tatra")), EPinkCabPartInstallResult::CapacityExceeded);

    FPinkCabServiceOperationRuntime Operations(1);
    FPinkCabEconomyLedger Ledger(10000, 0);
    TestEqual(TEXT("first parking applies"), Operations.ChargeParking(
        Ledger, FPinkCabStableId(TEXT("parking:1")), 100), EPinkCabServiceOperationResult::Applied);
    TestEqual(TEXT("operation journal bounded before debit"), Operations.ChargeParking(
        Ledger, FPinkCabStableId(TEXT("parking:2")), 100), EPinkCabServiceOperationResult::CapacityExceeded);
    TestEqual(TEXT("second debit not taken"), Ledger.GetBalanceMinor(), static_cast<int64>(9900));
    FPinkCabFuelTank Tank(60.0f, 10.0f, 1);
    TestEqual(TEXT("first fuel op applies"), Tank.CreditFuelOnce(
        FPinkCabTransactionId(TEXT("fuel:1")), 5.0f), EPinkCabFuelCreditResult::Applied);
    TestEqual(TEXT("fuel journal bounded"), Tank.CreditFuelOnce(
        FPinkCabTransactionId(TEXT("fuel:2")), 5.0f), EPinkCabFuelCreditResult::JournalFull);
    TestTrue(TEXT("second fuel not added"), FMath::IsNearlyEqual(Tank.GetLiters(), 15.0f));
    return true;
}

#endif
