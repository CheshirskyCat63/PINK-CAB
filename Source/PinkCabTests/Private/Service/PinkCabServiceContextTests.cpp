#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Service/PinkCabServiceContext.h"
#include "Service/PinkCabServiceNode.h"

namespace PinkCabServiceContextTests
{
FPinkCabServiceOwnerIdentity MakeOwners(const TCHAR* Suffix)
{
    FPinkCabServiceOwnerIdentity Owners;
    Owners.VehicleId = FPinkCabStableId(FString::Printf(TEXT("vehicle:%s"), Suffix));
    Owners.BuildOwnerId = FPinkCabStableId(FString::Printf(TEXT("build:%s"), Suffix));
    Owners.HealthOwnerId = FPinkCabStableId(FString::Printf(TEXT("health:%s"), Suffix));
    Owners.InventoryOwnerId = FPinkCabStableId(FString::Printf(TEXT("inventory:%s"), Suffix));
    Owners.EconomyOwnerId = FPinkCabStableId(FString::Printf(TEXT("economy:%s"), Suffix));
    return Owners;
}

FPinkCabCityIdentity MakeCity()
{
    return FPinkCabCityIdentity::Create(TEXT("FIRST-EURO"), TEXT("gen-1"), TEXT("content-1"));
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabServiceContextIdentityTest,
    "PinkCab.Service.Context.Identity",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPinkCabServiceContextIdentityTest::RunTest(const FString& Parameters)
{
    const FPinkCabCityIdentity City = PinkCabServiceContextTests::MakeCity();
    const FPinkCabServiceOwnerIdentity Owners = PinkCabServiceContextTests::MakeOwners(TEXT("primary"));
    FPinkCabServiceNode A(EPinkCabServiceNodeKind::GarageTuning);
    FPinkCabServiceNode B(EPinkCabServiceNodeKind::GarageTuning);

    TestTrue(TEXT("first node binds context"), A.BindContext(City, TEXT("central-garage"), Owners));
    TestTrue(TEXT("second node binds same logical context"), B.BindContext(City, TEXT("central-garage"), Owners));
    TestTrue(TEXT("stable service node id valid"), !A.GetServiceNodeId().IsEmpty());
    TestEqual(TEXT("same city kind semantic key reconstructs same node id"),
        A.GetServiceNodeId(), B.GetServiceNodeId());
    TestTrue(TEXT("bound context is valid"), A.GetContext().IsValid());
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabServiceOwnerContinuityTest,
    "PinkCab.Service.Context.OwnerContinuity",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabServiceOwnerContinuityTest::RunTest(const FString& Parameters)
{
    const FPinkCabServiceOwnerIdentity Owners = PinkCabServiceContextTests::MakeOwners(TEXT("same"));
    FPinkCabServiceNode Node(EPinkCabServiceNodeKind::RepairService);
    TestTrue(TEXT("context binds"), Node.BindContext(
        PinkCabServiceContextTests::MakeCity(), TEXT("repair-east"), Owners));
    TestTrue(TEXT("same owners recognized"), Node.HasSameOwners(Owners));
    FPinkCabServiceOwnerIdentity Other = Owners;
    Other.EconomyOwnerId = FPinkCabStableId(TEXT("economy:other"));
    TestFalse(TEXT("mismatched owner context rejected"), Node.HasSameOwners(Other));

    TestTrue(TEXT("eligible"), Node.MarkEligible());
    TestTrue(TEXT("enter"), Node.Enter());
    TestTrue(TEXT("activate"), Node.Activate());
    TestTrue(TEXT("begin commit"), Node.BeginCommit());
    TestTrue(TEXT("exit"), Node.Exit());
    TestTrue(TEXT("resume"), Node.ResumeWorld());
    TestTrue(TEXT("owners remain identical after lifecycle"), Node.HasSameOwners(Owners));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabServiceLifecycleResetTest,
    "PinkCab.Service.Context.LifecycleReset",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabServiceLifecycleResetTest::RunTest(const FString& Parameters)
{
    const FPinkCabServiceOwnerIdentity Owners = PinkCabServiceContextTests::MakeOwners(TEXT("reset"));
    FPinkCabServiceNode Node(EPinkCabServiceNodeKind::Parking);
    TestTrue(TEXT("bind"), Node.BindContext(PinkCabServiceContextTests::MakeCity(), TEXT("parking-west"), Owners));
    TestTrue(TEXT("eligible before abort"), Node.MarkEligible());
    TestTrue(TEXT("enter before abort"), Node.Enter());
    Node.Abort();
    TestEqual(TEXT("abort returns lifecycle to approach"), Node.GetState(), EPinkCabServiceNodeState::Approach);
    TestTrue(TEXT("abort preserves bound owner identity"), Node.HasSameOwners(Owners));
    TestFalse(TEXT("abort does not erase stable node id"), Node.GetServiceNodeId().IsEmpty());

    Node.ResetContext();
    TestEqual(TEXT("reset returns lifecycle to approach"), Node.GetState(), EPinkCabServiceNodeState::Approach);
    TestFalse(TEXT("reset clears binding"), Node.GetContext().IsValid());
    TestTrue(TEXT("reset clears node id"), Node.GetServiceNodeId().IsEmpty());
    return true;
}

#endif
