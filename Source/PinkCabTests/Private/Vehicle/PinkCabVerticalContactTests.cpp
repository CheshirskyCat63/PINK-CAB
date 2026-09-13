#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include <limits>
#include "Vehicle/PinkCabWallrideController.h"
#include "World/PinkCabVerticalContactRegistry.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabVerticalMassPolicyTest,
    "PinkCab.Vertical.L1.Identity.MassPolicy",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabVerticalMassPolicyTest::RunTest(const FString& Parameters)
{
    float Seconds = 0.0f;
    TestTrue(TEXT("1657 kg accepted"), FPinkCabWallrideController::TryComputeResidualTimeoutSeconds(1657.0f, Seconds));
    TestEqual(TEXT("1657 kg gives five seconds"), Seconds, 5.0f);
    TestTrue(TEXT("1882 kg accepted"), FPinkCabWallrideController::TryComputeResidualTimeoutSeconds(1882.0f, Seconds));
    TestTrue(TEXT("1882 kg gives 4.5 seconds"), FMath::IsNearlyEqual(Seconds, 4.5f));
    TestTrue(TEXT("2107 kg accepted"), FPinkCabWallrideController::TryComputeResidualTimeoutSeconds(2107.0f, Seconds));
    TestEqual(TEXT("2107 kg gives four seconds"), Seconds, 4.0f);
    TestFalse(TEXT("above max rejected"), FPinkCabWallrideController::TryComputeResidualTimeoutSeconds(2107.01f, Seconds));
    TestFalse(TEXT("non-finite rejected"), FPinkCabWallrideController::TryComputeResidualTimeoutSeconds(std::numeric_limits<float>::infinity(), Seconds));
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabVerticalAbortResetTest,
    "PinkCab.Vertical.L1.Identity.AbortReset",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabVerticalAbortResetTest::RunTest(const FString& Parameters)
{
    FPinkCabWallrideController Controller;
    Controller.Update(true, 0.1f, 1657.0f);
    Controller.Update(false, 1.0f, 1657.0f);
    TestEqual(TEXT("precondition residual"), Controller.GetState(), EPinkCabWallrideState::Residual);
    Controller.Abort();
    TestEqual(TEXT("abort detaches"), Controller.GetState(), EPinkCabWallrideState::Detached);
    TestEqual(TEXT("abort clears residual"), Controller.GetResidualSeconds(), 0.0f);
    TestFalse(TEXT("abort clears constraint request"), Controller.BuildConstraintRequest(1657.0f).bRequested);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabVerticalContactIdTest,
    "PinkCab.Vertical.L1.Identity.ContactIds",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPinkCabVerticalContactIdTest::RunTest(const FString& Parameters)
{
    const FPinkCabCityIdentity City = FPinkCabCityIdentity::Create(TEXT("VERT"), TEXT("gen-1"), TEXT("content-1"));
    const FPinkCabLaneId Lane(TEXT("lane:l1-main"));
    const FString A = FPinkCabVerticalContactRegistry::MakeContactId(
        City, Lane, EPinkCabVerticalContactKind::WallLeft, TEXT("wall-a"));
    const FString B = FPinkCabVerticalContactRegistry::MakeContactId(
        City, Lane, EPinkCabVerticalContactKind::WallLeft, TEXT("wall-a"));
    const FString Different = FPinkCabVerticalContactRegistry::MakeContactId(
        City, Lane, EPinkCabVerticalContactKind::FreightCeiling, TEXT("wall-a"));
    TestTrue(TEXT("contact id valid"), !A.IsEmpty());
    TestEqual(TEXT("same logical contact deterministic"), A, B);
    TestNotEqual(TEXT("kind participates in identity"), A, Different);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabVerticalContactRegistryTest,
    "PinkCab.Vertical.L1.Identity.Registry",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPinkCabVerticalContactRegistryTest::RunTest(const FString& Parameters)
{
    const FPinkCabCityIdentity City = FPinkCabCityIdentity::Create(TEXT("VERT"), TEXT("gen-1"), TEXT("content-1"));
    FPinkCabVerticalContactRegistry Registry(2);
    FString WallId;
    FString ReceiveId;
    TestTrue(TEXT("wall contact registers"), Registry.TryAdd(
        City, FPinkCabLaneId(TEXT("lane:l1")), EPinkCabVerticalContactKind::WallRight, TEXT("wall"), 0, WallId));
    TestFalse(TEXT("duplicate contact rejected"), Registry.TryAdd(
        City, FPinkCabLaneId(TEXT("lane:l1")), EPinkCabVerticalContactKind::WallRight, TEXT("wall"), 0, WallId));
    TestTrue(TEXT("receiving contact registers"), Registry.TryAdd(
        City, FPinkCabLaneId(TEXT("lane:l1")), EPinkCabVerticalContactKind::ReceivingStrip, TEXT("receive"), 0, ReceiveId));
    FString OverflowId;
    TestFalse(TEXT("hard cap enforced"), Registry.TryAdd(
        City, FPinkCabLaneId(TEXT("lane:l1")), EPinkCabVerticalContactKind::PoplarGapHook, TEXT("gap"), 0, OverflowId));
    TestEqual(TEXT("bounded count"), Registry.Num(), 2);
    TestTrue(TEXT("signature exists"), !Registry.GetReconstructionSignature().IsEmpty());
    FPinkCabVerticalContactRecord Found;
    TestTrue(TEXT("registered contact queryable"), Registry.TryGet(WallId, Found));
    TestEqual(TEXT("stable lane retained"), Found.LaneId.Serialize(), FString(TEXT("lane:l1")));
    return true;
}

#endif
