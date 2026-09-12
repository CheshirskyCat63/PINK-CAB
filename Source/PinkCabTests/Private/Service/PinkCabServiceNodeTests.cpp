#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Service/PinkCabServiceNode.h"
#include "Service/PinkCabVehicleBuild.h"
#include "Service/PinkCabRepairService.h"
#include "Economy/PinkCabEconomyLedger.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabServiceNodeLifecycleTest,
    "PinkCab.Service.Nodes.Lifecycle",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabServiceNodeLifecycleTest::RunTest(const FString& Parameters)
{
    FPinkCabServiceNode Node(EPinkCabServiceNodeKind::RepairService);
    TestEqual(TEXT("starts at approach"), Node.GetState(), EPinkCabServiceNodeState::Approach);
    TestFalse(TEXT("cannot enter before eligible"), Node.Enter());
    TestTrue(TEXT("approach becomes eligible"), Node.MarkEligible());
    TestTrue(TEXT("eligible enters"), Node.Enter());
    TestTrue(TEXT("enter becomes active"), Node.Activate());
    TestTrue(TEXT("active begins commit"), Node.BeginCommit());
    TestTrue(TEXT("commit exits"), Node.Exit());
    TestTrue(TEXT("exit resumes world"), Node.ResumeWorld());
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabVehicleBuildTest,
    "PinkCab.Service.Nodes.VehicleBuild",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabVehicleBuildTest::RunTest(const FString& Parameters)
{
    FPinkCabVehicleBuild Build;
    TestTrue(TEXT("stable part installs into named slot"),
        Build.InstallPart(FName(TEXT("FrontBrake")), TEXT("part.brake.street.v1")));
    TestEqual(TEXT("build schema remains explicit"), Build.GetSchemaVersion(), 1);
    TestEqual(TEXT("installed part resolves by stable id"),
        Build.GetInstalledPartId(FName(TEXT("FrontBrake"))), FString(TEXT("part.brake.street.v1")));
    TestFalse(TEXT("empty part id rejected"),
        Build.InstallPart(FName(TEXT("RearBrake")), TEXT("")));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabRepairBoundaryTest,
    "PinkCab.Service.Nodes.RepairBoundary",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPinkCabRepairBoundaryTest::RunTest(const FString& Parameters)
{
    FPinkCabVehicleHealthState Health;
    TestTrue(TEXT("synthetic damage applied"),
        Health.ApplyFunctionalDamage(EPinkCabVehicleHealthChannel::Brake, 0.6f));
    FPinkCabEconomyLedger Ledger(10000, 0);

    FPinkCabRepairRequest Request;
    TestTrue(TEXT("repair request built from current health"),
        FPinkCabRepairService::BuildRequest(
            Health,
            FPinkCabTransactionId(TEXT("repair-001")),
            2000,
            EPinkCabEconomyPurpose::MinimumRoadworthyRepair,
            { FPinkCabRepairLine{EPinkCabVehicleHealthChannel::Brake, 1.0f} },
            Request));

    TestEqual(TEXT("service does not own balance"), Ledger.GetBalanceMinor(), static_cast<int64>(10000));
    TestEqual(TEXT("request is repair debit"), Request.Transaction.Type, EPinkCabTransactionType::Repair);
    TestFalse(TEXT("request is not credit"), Request.Transaction.bCredit);
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabRepairCommitTest,
    "PinkCab.Service.Nodes.RepairCommit",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabRepairCommitTest::RunTest(const FString& Parameters)
{
    FPinkCabVehicleHealthState Health;
    Health.ApplyFunctionalDamage(EPinkCabVehicleHealthChannel::Brake, 0.6f);
    FPinkCabEconomyLedger Ledger(10000, 0);
    FPinkCabRepairRequest Request;
    FPinkCabRepairService::BuildRequest(
        Health, FPinkCabTransactionId(TEXT("repair-002")), 2000,
        EPinkCabEconomyPurpose::MinimumRoadworthyRepair,
        { FPinkCabRepairLine{EPinkCabVehicleHealthChannel::Brake, 1.0f} }, Request);

    TestEqual(TEXT("external economy commit succeeds"),
        Ledger.Commit(Request.Transaction), EPinkCabSettlementResult::Committed);
    TestEqual(TEXT("balance changed only in ledger"), Ledger.GetBalanceMinor(), static_cast<int64>(8000));
    TestTrue(TEXT("committed repair mutates same health state"),
        FPinkCabRepairService::ApplyCommittedRepair(Health, Request));
    TestTrue(TEXT("brake restored to target"),
        FMath::IsNearlyEqual(Health.GetHealth(EPinkCabVehicleHealthChannel::Brake), 1.0f));
    return true;
}

#endif
