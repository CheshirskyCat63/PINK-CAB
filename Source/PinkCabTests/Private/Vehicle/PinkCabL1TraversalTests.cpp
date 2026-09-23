#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Vehicle/PinkCabL1TraversalState.h"
#include "Vehicle/PinkCabL1TraversalConfig.h"
#include "Vehicle/PinkCabL1TraversalTransitionPolicy.h"

namespace PinkCabL1TraversalTests
{
FPinkCabL1TraversalInput Contact(
    EPinkCabVerticalContactKind Kind,
    const TCHAR* ContactId,
    float MassKg = 1657.0f,
    float DeltaSeconds = 0.1f)
{
    FPinkCabL1TraversalInput Input;
    Input.bHasContact = true;
    Input.ContactKind = Kind;
    Input.ContactId = ContactId;
    Input.TotalMassKg = MassKg;
    Input.DeltaSeconds = DeltaSeconds;
    return Input;
}

FPinkCabL1TraversalInput Air(float DeltaSeconds, float MassKg = 1657.0f)
{
    FPinkCabL1TraversalInput Input;
    Input.TotalMassKg = MassKg;
    Input.DeltaSeconds = DeltaSeconds;
    return Input;
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabL1WallResidualReacquireTest,
    "PinkCab.Vertical.L1.Traversal.WallResidualReacquire",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabL1WallResidualReacquireTest::RunTest(const FString& Parameters)
{
    using namespace PinkCabL1TraversalTests;
    FPinkCabL1TraversalState State;
    FPinkCabL1TraversalResult Result = State.Step(
        Contact(EPinkCabVerticalContactKind::WallLeft, TEXT("contact:wall-left")));
    TestEqual(TEXT("wall contact phase"), Result.Phase, EPinkCabL1TraversalPhase::WallContact);
    TestTrue(TEXT("constraint requested"), Result.bConstraintRequested);
    TestFalse(TEXT("no hidden force applied"), Result.bForceApplied);

    Result = State.Step(Air(1.0f));
    TestEqual(TEXT("loss enters residual"), Result.Phase, EPinkCabL1TraversalPhase::WallResidual);
    TestTrue(TEXT("residual still requests constraint"), Result.bConstraintRequested);
    Result = State.Step(Contact(EPinkCabVerticalContactKind::WallLeft, TEXT("contact:wall-left"), 1657.0f, 0.25f));
    TestEqual(TEXT("same wall reacquired"), Result.Phase, EPinkCabL1TraversalPhase::WallContact);
    TestEqual(TEXT("contact identity retained"), Result.CurrentContactId, FString(TEXT("contact:wall-left")));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabL1FreightReceivingTest,
    "PinkCab.Vertical.L1.Traversal.FreightReceiving",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabL1FreightReceivingTest::RunTest(const FString& Parameters)
{
    using namespace PinkCabL1TraversalTests;
    FPinkCabL1TraversalState State;
    FPinkCabL1TraversalResult Result = State.Step(
        Contact(EPinkCabVerticalContactKind::FreightCeiling, TEXT("contact:freight")));
    TestEqual(TEXT("freight contact phase"), Result.Phase, EPinkCabL1TraversalPhase::FreightContact);
    TestTrue(TEXT("freight accepted"), Result.bAccepted);
    TestFalse(TEXT("freight never claims force application"), Result.bForceApplied);

    Result = State.Step(Contact(EPinkCabVerticalContactKind::ReceivingStrip, TEXT("contact:receive")));
    TestEqual(TEXT("receiving completes traversal"), Result.Phase, EPinkCabL1TraversalPhase::Completed);
    TestTrue(TEXT("completion flag"), Result.bCompleted);
    TestEqual(TEXT("previous contact retained"), Result.PreviousContactId, FString(TEXT("contact:freight")));
    TestEqual(TEXT("receiving identity retained"), Result.CurrentContactId, FString(TEXT("contact:receive")));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabL1GapTransferTest,
    "PinkCab.Vertical.L1.Traversal.GapTransfer",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabL1GapTransferTest::RunTest(const FString& Parameters)
{
    using namespace PinkCabL1TraversalTests;
    FPinkCabL1TraversalState State;
    FPinkCabL1TraversalResult Result = State.Step(
        Contact(EPinkCabVerticalContactKind::PoplarGapHook, TEXT("contact:gap")));
    TestEqual(TEXT("gap hook transfer phase"), Result.Phase, EPinkCabL1TraversalPhase::GapTransfer);
    TestTrue(TEXT("gap hook accepted"), Result.bAccepted);
    TestFalse(TEXT("gap hook no hidden force"), Result.bForceApplied);

    Result = State.Step(Contact(EPinkCabVerticalContactKind::ReceivingStrip, TEXT("contact:receive")));
    TestEqual(TEXT("gap transfer reaches receiving"), Result.Phase, EPinkCabL1TraversalPhase::Completed);
    TestTrue(TEXT("gap transfer completed"), Result.bCompleted);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabL1InvalidCrossKindTest,
    "PinkCab.Vertical.L1.Traversal.InvalidCrossKind",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabL1InvalidCrossKindTest::RunTest(const FString& Parameters)
{
    using namespace PinkCabL1TraversalTests;
    FPinkCabL1TraversalState State;
    State.Step(Contact(EPinkCabVerticalContactKind::WallLeft, TEXT("contact:wall-left")));
    const FPinkCabL1TraversalResult Result = State.Step(
        Contact(EPinkCabVerticalContactKind::WallRight, TEXT("contact:wall-right")));
    TestEqual(TEXT("cross-wall transition fails"), Result.Phase, EPinkCabL1TraversalPhase::Failed);
    TestEqual(TEXT("reason is deterministic"), Result.Failure, EPinkCabL1TraversalFailure::InvalidTransition);
    TestFalse(TEXT("failure emits no constraint"), Result.bConstraintRequested);
    TestFalse(TEXT("failure emits no force"), Result.bForceApplied);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabL1AbortCollisionTest,
    "PinkCab.Vertical.L1.Traversal.AbortCollisionCleanup",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabL1AbortCollisionTest::RunTest(const FString& Parameters)
{
    using namespace PinkCabL1TraversalTests;
    FPinkCabL1TraversalState State;
    State.Step(Contact(EPinkCabVerticalContactKind::WallLeft, TEXT("contact:wall-left")));
    FPinkCabL1TraversalInput AbortInput = Air(0.0f);
    AbortInput.bAbort = true;
    FPinkCabL1TraversalResult Result = State.Step(AbortInput);
    TestEqual(TEXT("abort phase"), Result.Phase, EPinkCabL1TraversalPhase::Aborted);
    TestTrue(TEXT("abort clears current contact"), Result.CurrentContactId.IsEmpty());
    TestFalse(TEXT("abort clears constraint"), Result.bConstraintRequested);

    State.Reset();
    State.Step(Contact(EPinkCabVerticalContactKind::FreightCeiling, TEXT("contact:freight")));
    FPinkCabL1TraversalInput CollisionInput = Air(0.0f);
    CollisionInput.bCollision = true;
    Result = State.Step(CollisionInput);
    TestEqual(TEXT("collision fails traversal"), Result.Phase, EPinkCabL1TraversalPhase::Failed);
    TestEqual(TEXT("collision reason"), Result.Failure, EPinkCabL1TraversalFailure::Collision);
    TestTrue(TEXT("collision clears current contact"), Result.CurrentContactId.IsEmpty());
    TestFalse(TEXT("collision emits no force"), Result.bForceApplied);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabL1UnsupportedMassTest,
    "PinkCab.Vertical.L1.Traversal.UnsupportedMass",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabL1UnsupportedMassTest::RunTest(const FString& Parameters)
{
    using namespace PinkCabL1TraversalTests;
    FPinkCabL1TraversalState State;
    const FPinkCabL1TraversalResult Result = State.Step(
        Contact(EPinkCabVerticalContactKind::WallLeft, TEXT("contact:wall-left"), 2200.0f));
    TestEqual(TEXT("unsupported mass fails closed"), Result.Phase, EPinkCabL1TraversalPhase::Failed);
    TestEqual(TEXT("unsupported mass reason"), Result.Failure, EPinkCabL1TraversalFailure::UnsupportedMass);
    TestFalse(TEXT("unsupported mass emits no constraint"), Result.bConstraintRequested);
    return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabL1TransitionPolicySeamTest,
    "PinkCab.Vertical.L1.Traversal.TransitionPolicySeam",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabL1TransitionPolicySeamTest::RunTest(const FString& Parameters)
{
    FPinkCabL1TraversalConfig Config;
    TestTrue(TEXT("road accepts wall"),
        FPinkCabL1TraversalTransitionPolicy::CanEnterFromRoad(
            Config, EPinkCabVerticalContactKind::WallLeft));
    TestTrue(TEXT("road accepts freight"),
        FPinkCabL1TraversalTransitionPolicy::CanEnterFromRoad(
            Config, EPinkCabVerticalContactKind::FreightCeiling));
    TestTrue(TEXT("road accepts gap hook"),
        FPinkCabL1TraversalTransitionPolicy::CanEnterFromRoad(
            Config, EPinkCabVerticalContactKind::PoplarGapHook));
    TestFalse(TEXT("road rejects receiving strip"),
        FPinkCabL1TraversalTransitionPolicy::CanEnterFromRoad(
            Config, EPinkCabVerticalContactKind::ReceivingStrip));
    TestTrue(TEXT("wall accepts receiving strip"),
        FPinkCabL1TraversalTransitionPolicy::CanEnterFromWall(
            Config, EPinkCabVerticalContactKind::ReceivingStrip));

    Config.bAllowWallToGapTransfer = false;
    TestFalse(TEXT("config can close only wall-to-gap transition"),
        FPinkCabL1TraversalTransitionPolicy::CanEnterFromWall(
            Config, EPinkCabVerticalContactKind::PoplarGapHook));
    TestTrue(TEXT("wall-to-freight remains allowed"),
        FPinkCabL1TraversalTransitionPolicy::CanEnterFromWall(
            Config, EPinkCabVerticalContactKind::FreightCeiling));
    return true;
}

#endif
