#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Service/PinkCabServiceOperationRuntime.h"
#include "Service/PinkCabPartCatalog.h"
#include "Service/PinkCabServiceInventory.h"
#include "Service/PinkCabVehicleBuild.h"
#include "Vehicle/PinkCabVehicleHealthState.h"
#include "Economy/PinkCabEconomyLedger.h"

namespace PinkCabServiceOperationTests
{
FPinkCabPartDefinition MakePart()
{
    FPinkCabPartDefinition Part;
    Part.PartId = TEXT("part.brake.street.v1");
    Part.SlotId = FName(TEXT("FrontBrake"));
    Part.PriceMinor = 1200;
    Part.CompatibilityTag = FName(TEXT("tatra603"));
    return Part;
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabServicePurchaseCommitOrderTest,
    "PinkCab.Service.Operations.PurchaseCommitOrder",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPinkCabServicePurchaseCommitOrderTest::RunTest(const FString& Parameters)
{
    FPinkCabEconomyLedger Ledger(5000, 0);
    FPinkCabServiceInventory Inventory(4);
    FPinkCabServiceOperationRuntime Runtime;
    const FPinkCabPartDefinition Part = PinkCabServiceOperationTests::MakePart();

    const EPinkCabServiceOperationResult Result = Runtime.PurchasePart(
        Ledger, Inventory, FPinkCabStableId(TEXT("purchase-001")), Part);
    TestEqual(TEXT("purchase applies"), Result, EPinkCabServiceOperationResult::Applied);
    TestEqual(TEXT("ledger debited once"), Ledger.GetBalanceMinor(), static_cast<int64>(3800));
    TestEqual(TEXT("inventory credited after settlement"), Inventory.GetQuantity(Part.PartId), 1);

    const EPinkCabServiceOperationResult Retry = Runtime.PurchasePart(
        Ledger, Inventory, FPinkCabStableId(TEXT("purchase-001")), Part);
    TestEqual(TEXT("retry is duplicate"), Retry, EPinkCabServiceOperationResult::Duplicate);
    TestEqual(TEXT("retry does not debit"), Ledger.GetBalanceMinor(), static_cast<int64>(3800));
    TestEqual(TEXT("retry does not duplicate item"), Inventory.GetQuantity(Part.PartId), 1);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabServiceInsufficientFundsNoMutationTest,
    "PinkCab.Service.Operations.InsufficientFunds",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPinkCabServiceInsufficientFundsNoMutationTest::RunTest(const FString& Parameters)
{
    FPinkCabEconomyLedger Ledger(500, 0);
    FPinkCabServiceInventory Inventory(4);
    FPinkCabServiceOperationRuntime Runtime;
    const FPinkCabPartDefinition Part = PinkCabServiceOperationTests::MakePart();

    TestEqual(TEXT("purchase rejected"),
        Runtime.PurchasePart(Ledger, Inventory, FPinkCabStableId(TEXT("purchase-poor")), Part),
        EPinkCabServiceOperationResult::SettlementRejected);
    TestEqual(TEXT("balance unchanged"), Ledger.GetBalanceMinor(), static_cast<int64>(500));
    TestEqual(TEXT("inventory unchanged"), Inventory.GetQuantity(Part.PartId), 0);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabServiceParkingReplayTest,
    "PinkCab.Service.Operations.ParkingReplay",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPinkCabServiceParkingReplayTest::RunTest(const FString& Parameters)
{
    FPinkCabEconomyLedger Ledger(3000, 0);
    FPinkCabServiceOperationRuntime Runtime;
    const FPinkCabStableId OperationId(TEXT("parking-001"));

    TestEqual(TEXT("parking charged"), Runtime.ChargeParking(Ledger, OperationId, 400),
        EPinkCabServiceOperationResult::Applied);
    TestEqual(TEXT("parking retry duplicate"), Runtime.ChargeParking(Ledger, OperationId, 400),
        EPinkCabServiceOperationResult::Duplicate);
    TestEqual(TEXT("parking debited once"), Ledger.GetBalanceMinor(), static_cast<int64>(2600));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabServiceInstallReplayTest,
    "PinkCab.Service.Operations.InstallReplay",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPinkCabServiceInstallReplayTest::RunTest(const FString& Parameters)
{
    FPinkCabServiceInventory Inventory(4);
    FPinkCabVehicleBuild Build;
    FPinkCabServiceOperationRuntime Runtime;
    const FPinkCabPartDefinition Part = PinkCabServiceOperationTests::MakePart();
    TestEqual(TEXT("owned part seeded"),
        Inventory.AddOwnedPartOnce(FPinkCabStableId(TEXT("seed-part")), Part.PartId),
        EPinkCabInventoryMutationResult::Applied);

    TestEqual(TEXT("install applies"), Runtime.InstallOwnedPart(
        Inventory, Build, FPinkCabStableId(TEXT("install-001")), Part, FName(TEXT("tatra603"))),
        EPinkCabServiceOperationResult::Applied);
    TestEqual(TEXT("part consumed once"), Inventory.GetQuantity(Part.PartId), 0);
    TestEqual(TEXT("build updated"), Build.GetInstalledPartId(Part.SlotId), Part.PartId);
    TestEqual(TEXT("install retry duplicate"), Runtime.InstallOwnedPart(
        Inventory, Build, FPinkCabStableId(TEXT("install-001")), Part, FName(TEXT("tatra603"))),
        EPinkCabServiceOperationResult::Duplicate);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabServiceRepairReplayTest,
    "PinkCab.Service.Operations.RepairReplay",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPinkCabServiceRepairReplayTest::RunTest(const FString& Parameters)
{
    FPinkCabEconomyLedger Ledger(1000, 1000);
    FPinkCabVehicleHealthState Health;
    FPinkCabServiceOperationRuntime Runtime;
    Health.ApplyFunctionalDamage(EPinkCabVehicleHealthChannel::Brake, 0.7f);
    const TArray<FPinkCabRepairLine> Lines = {
        FPinkCabRepairLine{EPinkCabVehicleHealthChannel::Brake, 1.0f}
    };

    TestEqual(TEXT("minimum repair applies through debt policy"), Runtime.Repair(
        Ledger, Health, FPinkCabStableId(TEXT("repair-op-001")), 1500,
        EPinkCabEconomyPurpose::MinimumRoadworthyRepair, Lines),
        EPinkCabServiceOperationResult::Applied);
    TestTrue(TEXT("brake repaired"), FMath::IsNearlyEqual(
        Health.GetHealth(EPinkCabVehicleHealthChannel::Brake), 1.0f));
    TestEqual(TEXT("repair retry duplicate"), Runtime.Repair(
        Ledger, Health, FPinkCabStableId(TEXT("repair-op-001")), 1500,
        EPinkCabEconomyPurpose::MinimumRoadworthyRepair, Lines),
        EPinkCabServiceOperationResult::Duplicate);
    TestEqual(TEXT("repair charged once"), Ledger.GetBalanceMinor(), static_cast<int64>(-500));
    return true;
}

#endif
