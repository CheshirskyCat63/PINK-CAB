#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "InputCoreTypes.h"
#include "Interaction/PinkCabInteractionModel.h"
#include "Interaction/PinkCabSemanticInputRouter.h"
#include "Interaction/PinkCabSemanticCommand.h"
#include "Interaction/PinkCabContractCabinPrimitive.h"
#include "Editor.h"
#include "FileHelpers.h"
#include "EngineUtils.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabGazeOwnershipTest,
    "PinkCab.Interaction.GazeOwnership",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabGazeOwnershipTest::RunTest(const FString& Parameters)
{
    FPinkCabInteractionState State;
    TestEqual(TEXT("mouse steers by default"), State.GetMouseOwner(), EPinkCabMouseOwner::Steering);
    State.SetGazeHeld(true);
    TestEqual(TEXT("Space transfers mouse to gaze"), State.GetMouseOwner(), EPinkCabMouseOwner::Gaze);
    State.SetGazeHeld(false);
    TestEqual(TEXT("releasing Space returns steering"), State.GetMouseOwner(), EPinkCabMouseOwner::Steering);

    const FPinkCabReticleFeedback Empty = FPinkCabReticleFeedback::Resolve(false);
    const FPinkCabReticleFeedback Valid = FPinkCabReticleFeedback::Resolve(true);
    TestTrue(TEXT("valid target reticle becomes more opaque"), Valid.Opacity > Empty.Opacity);
    TestTrue(TEXT("valid target reticle uses white-matte emphasis"), Valid.bWhiteMatte);
    TestFalse(TEXT("reticle never requires a text label"), Valid.bShowTextLabel);
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabQuickRecallNoActuationTest,
    "PinkCab.Interaction.QuickRecallNoActuation",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabQuickRecallNoActuationTest::RunTest(const FString& Parameters)
{
    FPinkCabInteractionState State;
    const uint32 Before = State.GetActuationSerial();
    TestTrue(TEXT("slot 2 recalls horn target"), State.RecallQuickTarget(2));
    TestEqual(TEXT("slot 2 target is horn"), State.GetCurrentTargetId(), FName(TEXT("Horn")));
    TestEqual(TEXT("quick recall does not actuate"), State.GetActuationSerial(), Before);
    TestTrue(TEXT("slot 1 recalls turn signals"), State.RecallQuickTarget(1));
    TestEqual(TEXT("slot 1 target is turn signals"), State.GetCurrentTargetId(), FName(TEXT("TurnSignals")));
    TestEqual(TEXT("recall still does not actuate"), State.GetActuationSerial(), Before);
    TestFalse(TEXT("unknown quick slot is rejected"), State.RecallQuickTarget(9));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabContextualGestureTest,
    "PinkCab.Interaction.ContextualGestures",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPinkCabContextualGestureTest::RunTest(const FString& Parameters)
{
    FPinkCabInteractionState State;
    const FPinkCabInteractionControlSpec Horn(FName(TEXT("Horn")), false, true, false);
    State.SetCurrentTarget(Horn);
    const uint32 BeforeHorn = State.GetActuationSerial();
    TestFalse(TEXT("horn does not require RMB grip"), State.TryBeginGrip());
    TestTrue(TEXT("horn accepts LMB press"), State.TryMomentaryPress(true));
    TestTrue(TEXT("horn remains held"), State.IsMomentaryHeld());
    TestTrue(TEXT("horn release is accepted"), State.TryMomentaryPress(false));
    TestFalse(TEXT("horn release clears hold"), State.IsMomentaryHeld());
    TestTrue(TEXT("horn actuation increments serial"), State.GetActuationSerial() > BeforeHorn);

    const FPinkCabInteractionControlSpec Gearbox(FName(TEXT("Gearbox")), true, false, false);
    State.SetCurrentTarget(Gearbox);
    const uint32 BeforeGrip = State.GetActuationSerial();
    TestTrue(TEXT("gearbox accepts optional grip"), State.TryBeginGrip());
    TestTrue(TEXT("grip state is gameplay-hand-agnostic"), State.IsGripActive());
    TestEqual(TEXT("grip itself does not actuate"), State.GetActuationSerial(), BeforeGrip);

    const FPinkCabInteractionControlSpec HeadlampToggle(FName(TEXT("Headlamps")), false, false, true);
    State.SetCurrentTarget(HeadlampToggle);
    const uint32 BeforeWheel = State.GetActuationSerial();
    TestEqual(TEXT("wheel up forwards positive detent"), State.ApplyWheelSteps(1), 1);
    TestEqual(TEXT("wheel action increments serial once"), State.GetActuationSerial(), BeforeWheel + 1);
    TestFalse(TEXT("toggle rejects LMB when not authored"), State.TryMomentaryPress(true));
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabBoundedTargetSelectionTest,
    "PinkCab.Interaction.BoundedTargetSelection",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabBoundedTargetSelectionTest::RunTest(const FString& Parameters)
{
    TArray<FPinkCabInteractionCandidate> Candidates;
    Candidates.Emplace(FName(TEXT("Far")), 900.0f, 0.1f, true);
    Candidates.Emplace(FName(TEXT("Horn")), 120.0f, 0.08f, true);
    Candidates.Emplace(FName(TEXT("Gearbox")), 100.0f, 0.20f, true);
    Candidates.Emplace(FName(TEXT("OutOfBudget")), 50.0f, 0.01f, true);

    const FName Selected = FPinkCabInteractionTargetSelector::SelectCurrentTarget(Candidates, 3, 250.0f);
    TestEqual(TEXT("bounded prefix picks eligible target"), Selected, FName(TEXT("Horn")));
    const FName None = FPinkCabInteractionTargetSelector::SelectCurrentTarget(Candidates, 1, 250.0f);
    TestTrue(TEXT("no eligible target returns None"), None.IsNone());
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabSemanticInputRouterTest,
    "PinkCab.Interaction.SemanticInputRouter",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPinkCabSemanticInputRouterTest::RunTest(const FString& Parameters)
{
    FPinkCabSemanticInputRouter Router = FPinkCabSemanticInputRouter::CreateDefaults();
    TestEqual(TEXT("Space resolves to gaze"), Router.Resolve(EKeys::SpaceBar), EPinkCabSemanticAction::GazeHold);
    TestEqual(TEXT("1 resolves to quick recall 1"), Router.Resolve(EKeys::One), EPinkCabSemanticAction::QuickRecall1);
    TestEqual(TEXT("RMB resolves to grip"), Router.Resolve(EKeys::RightMouseButton), EPinkCabSemanticAction::Grip);
    TestEqual(TEXT("LMB resolves to momentary press"), Router.Resolve(EKeys::LeftMouseButton), EPinkCabSemanticAction::MomentaryPress);

    TestFalse(TEXT("conflicting rebind is rejected"), Router.TryRebind(EPinkCabSemanticAction::QuickRecall1, EKeys::SpaceBar));
    TestTrue(TEXT("free key can be rebound"), Router.TryRebind(EPinkCabSemanticAction::QuickRecall1, EKeys::F1));
    TestEqual(TEXT("rebind updates semantic mapping"), Router.Resolve(EKeys::F1), EPinkCabSemanticAction::QuickRecall1);
    Router.RestoreDefaults();
    TestEqual(TEXT("Restore Defaults returns slot 1 to key 1"), Router.Resolve(EKeys::One), EPinkCabSemanticAction::QuickRecall1);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabSemanticEventBoundaryTest,
    "PinkCab.Interaction.SemanticEventBoundary",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabSemanticEventBoundaryTest::RunTest(const FString& Parameters)
{
    FPinkCabInteractionState State;
    State.SetCurrentTarget(FPinkCabInteractionControlSpec(FName(TEXT("Headlamps")), false, false, true));
    State.ApplyWheelSteps(1);
    const FPinkCabInteractionEvent Event = State.GetLastEvent();
    TestEqual(TEXT("event keeps physical target id"), Event.TargetId, FName(TEXT("Headlamps")));
    TestEqual(TEXT("event exposes wheel semantic"), Event.Gesture, EPinkCabInteractionGesture::WheelIncrement);
    TestEqual(TEXT("event keeps signed value"), Event.SignedValue, 1);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabContinuousControlsTest,
    "PinkCab.Interaction.ContinuousControls",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabContinuousControlsTest::RunTest(const FString& Parameters)
{
    FPinkCabContinuousVehicleControls Controls;
    Controls.SetSteering(2.0f);
    Controls.SetThrottle(-0.5f);
    Controls.SetBrake(1.4f);
    Controls.SetClutch(0.35f);
    Controls.SetHandbrake(0.8f);
    TestEqual(TEXT("steering clamps to signed normalized range"), Controls.GetSteering(), 1.0f);
    TestEqual(TEXT("throttle clamps to zero-one range"), Controls.GetThrottle(), 0.0f);
    TestEqual(TEXT("brake clamps to zero-one range"), Controls.GetBrake(), 1.0f);
    TestEqual(TEXT("clutch preserves normalized value"), Controls.GetClutch(), 0.35f);
    TestEqual(TEXT("handbrake preserves normalized value"), Controls.GetHandbrake(), 0.8f);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabContractZeroCabinPrimitiveTest,
    "PinkCab.Interaction.ContractZeroCabinPrimitive",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabContractZeroCabinPrimitiveTest::RunTest(const FString& Parameters)
{
    TestTrue(TEXT("ContractZero map loads"), FEditorFileUtils::LoadMap(TEXT("/Game/Dev/Maps/L_PinkCab_ContractZero"), false, true));
    UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
    TestNotNull(TEXT("editor world exists"), World);
    if (!World) return false;
    APinkCabContractCabinPrimitive* Primitive = nullptr;
    for (TActorIterator<APinkCabContractCabinPrimitive> It(World); It; ++It) { Primitive = *It; break; }
    TestNotNull(TEXT("ContractZero contains cabin primitive"), Primitive);
    if (!Primitive) return false;
    FPinkCabInteractionState State;
    State.SetCurrentTarget(FPinkCabInteractionControlSpec(Primitive->GetInteractionTargetId(), false, false, true));
    State.ApplyWheelSteps(1);
    Primitive->ApplyInteractionEvent(State.GetLastEvent());
    TestTrue(TEXT("wheel-up semantic event turns developer-visible state on"), Primitive->IsDeveloperStateOn());
    return true;
}

#endif
