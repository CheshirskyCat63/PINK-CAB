#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "InputCoreTypes.h"
#include "Cockpit/PinkCabCockpitInteractionComponent.h"
#include "Cockpit/PinkCabCockpitAssemblyComponent.h"
#include "Components/SceneComponent.h"
#include "Interaction/PinkCabInteractionModel.h"
#include "Interaction/PinkCabSemanticInputRouter.h"
#include "Runtime/PinkCabChaosTatraPawn.h"
#include "Vehicle/PinkCabCockpitInteractionRouter.h"
#include "Vehicle/PinkCabCockpitState.h"
#include "Vehicle/PinkCabVehicleInputFrame.h"
#include "Vehicle/PinkCabSteeringController.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabCanonicalBindingComplianceTest,
    "PinkCab.Cockpit.Input.Compliance.Bindings",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabCanonicalBindingComplianceTest::RunTest(const FString& Parameters)
{
    FPinkCabSemanticInputRouter Router = FPinkCabSemanticInputRouter::CreateDefaults();
    TestEqual(TEXT("Space owns gaze"), Router.Resolve(EKeys::SpaceBar), EPinkCabSemanticAction::GazeHold);
    TestEqual(TEXT("RMB owns grip"), Router.Resolve(EKeys::RightMouseButton), EPinkCabSemanticAction::Grip);
    TestEqual(TEXT("LMB owns momentary press"), Router.Resolve(EKeys::LeftMouseButton), EPinkCabSemanticAction::MomentaryPress);
    TestEqual(TEXT("wheel owns contextual adjustment"), Router.Resolve(EKeys::MouseWheelAxis), EPinkCabSemanticAction::Wheel);
    TestEqual(TEXT("R is not a cockpit semantic shortcut"), Router.Resolve(EKeys::R), EPinkCabSemanticAction::None);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabGazeOwnershipComplianceTest,
    "PinkCab.Cockpit.Input.Compliance.PC_T_INP_001",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabGazeOwnershipComplianceTest::RunTest(const FString& Parameters)
{
    UPinkCabCockpitInteractionComponent* Interaction = NewObject<UPinkCabCockpitInteractionComponent>();
    FPinkCabSteeringControllerConfig Config;
    Config.MouseCountsForFullScale = 100.0f;
    FPinkCabSteeringController Steering(Config);

    Interaction->SetGazeHeld(false);
    const float Steered = Steering.Step(
        50.0f, false, 60.0f, EPinkCabVehicleMotionMode::Moving, 0.5f);
    TestTrue(TEXT("mouse changes steering outside gaze"), Steered > 0.0f);

    Interaction->SetGazeHeld(true);
    const float Preserved = Steering.Step(
        20.0f, true, 60.0f, EPinkCabVehicleMotionMode::Moving, 0.5f);
    TestEqual(TEXT("Space gaze preserves steering command"), Preserved, Steered);

    Interaction->SetGazeHeld(false);
    const float Returned = Steering.Step(
        -80.0f, false, 60.0f, EPinkCabVehicleMotionMode::Moving, 0.5f);
    TestTrue(TEXT("Space release returns mouse to steering"), Returned < Preserved);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabQuickRecallComplianceTest,
    "PinkCab.Cockpit.Input.Compliance.PC_T_INP_002_006",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPinkCabQuickRecallComplianceTest::RunTest(const FString& Parameters)
{
    UPinkCabCockpitInteractionComponent* Interaction = NewObject<UPinkCabCockpitInteractionComponent>();
    const uint32 Before = Interaction->GetActuationSerial();
    Interaction->SetQuickSlotHeld(1, true);
    TestEqual(TEXT("1 recalls signals"), Interaction->GetCurrentQuickTargetId(), FName(TEXT("TurnSignals")));
    Interaction->SetQuickSlotHeld(3, true);
    TestEqual(TEXT("latest held quick key wins"), Interaction->GetCurrentQuickTargetId(), FName(TEXT("Gearbox")));
    TestEqual(TEXT("quick recall never actuates"), Interaction->GetActuationSerial(), Before);
    Interaction->SetQuickSlotHeld(3, false);
    TestEqual(TEXT("release falls back to older held quick key"), Interaction->GetCurrentQuickTargetId(), FName(TEXT("TurnSignals")));
    Interaction->SetQuickSlotHeld(1, false);
    TestTrue(TEXT("all released clears quick target"), Interaction->GetCurrentQuickTargetId().IsNone());
    TestEqual(TEXT("release still does not actuate"), Interaction->GetActuationSerial(), Before);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabQuickRecallGripLatchTest,
    "PinkCab.Cockpit.Input.QuickRecallGripLatch",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabQuickRecallGripLatchTest::RunTest(const FString& Parameters)
{
    UPinkCabCockpitInteractionComponent* Interaction = NewObject<UPinkCabCockpitInteractionComponent>();
    FPinkCabCockpitInteractionFrame Frame;
    TArray<FPinkCabInteractionEvent> Events;

    Frame.bQuickRecall4Held = true;
    Interaction->ProcessFrame(Frame, nullptr, Events);
    TestEqual(TEXT("4 recalls handbrake without actuation"),
        Interaction->GetCurrentTargetId(), FName(TEXT("Handbrake")));
    const uint32 Before = Interaction->GetActuationSerial();

    Frame.bQuickRecall4Held = false;
    Frame.bGripHeld = true;
    Interaction->ProcessFrame(Frame, nullptr, Events);
    TestTrue(TEXT("RMB acquires the remembered quick-recall target"),
        Interaction->IsGripActive());
    TestEqual(TEXT("remembered quick-recall target is the handbrake"),
        Interaction->GetActiveGripTargetId(), FName(TEXT("Handbrake")));
    TestEqual(TEXT("quick recall plus grip still does not actuate"),
        Interaction->GetActuationSerial(), Before);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabConsumedRecallLifecycleTest,
    "PinkCab.Cockpit.Input.Recovery.ConsumedRecallLifecycle",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabConsumedRecallLifecycleTest::RunTest(const FString& Parameters)
{
    UPinkCabCockpitInteractionComponent* Interaction =
        NewObject<UPinkCabCockpitInteractionComponent>();
    FPinkCabCockpitInteractionFrame Frame;
    TArray<FPinkCabInteractionEvent> Events;

    Frame.bQuickRecall4Held = true;
    Interaction->ProcessFrame(Frame, nullptr, Events);
    Frame.bQuickRecall4Held = false;
    Frame.bGripHeld = true;
    Events.Reset();
    Interaction->ProcessFrame(Frame, nullptr, Events);
    TestTrue(TEXT("RMB acquires recalled handbrake"), Interaction->IsGripActive());

    Frame.bGripHeld = false;
    Events.Reset();
    Interaction->ProcessFrame(Frame, nullptr, Events);
    TestFalse(TEXT("RMB release ends handbrake grip"), Interaction->IsGripActive());
    TestTrue(TEXT("consumed recalled handbrake is no longer an eligible target"),
        Interaction->GetCurrentTargetId().IsNone());

    Interaction->ResetTransientInputState();
    Frame = {};
    Frame.bQuickRecall2Held = true;
    Interaction->ProcessFrame(Frame, nullptr, Events);
    Frame.bQuickRecall2Held = false;
    Frame.bMomentaryHeld = true;
    Frame.NowSeconds = 1.0;
    Events.Reset();
    Interaction->ProcessFrame(Frame, nullptr, Events);
    TestTrue(TEXT("LMB starts recalled horn action"), Interaction->IsMomentaryActive());

    Frame.bMomentaryHeld = false;
    Frame.NowSeconds = 1.1;
    Events.Reset();
    Interaction->ProcessFrame(Frame, nullptr, Events);
    TestFalse(TEXT("LMB release ends horn action"), Interaction->IsMomentaryActive());
    TestTrue(TEXT("consumed recalled horn is no longer an eligible target"),
        Interaction->GetCurrentTargetId().IsNone());
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabGripComplianceTest,
    "PinkCab.Cockpit.Input.Compliance.PC_T_INP_003",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabGripComplianceTest::RunTest(const FString& Parameters)
{
    UPinkCabCockpitInteractionComponent* Interaction = NewObject<UPinkCabCockpitInteractionComponent>();
    Interaction->SetCurrentTarget(FPinkCabInteractionControlSpec(TEXT("Horn"), false, true, false));
    FPinkCabInteractionEvent Event;
    TestFalse(TEXT("RMB rejects target without grip capability"), Interaction->BeginGrip(Event));

    Interaction->SetCurrentTarget(FPinkCabInteractionControlSpec(TEXT("Gearbox"), true, false, true));
    const uint32 Before = Interaction->GetActuationSerial();
    TestTrue(TEXT("RMB grips grip-capable target"), Interaction->BeginGrip(Event));
    TestTrue(TEXT("grip state is retained"), Interaction->IsGripActive());
    TestEqual(TEXT("grip alone never actuates"), Interaction->GetActuationSerial(), Before);
    TestTrue(TEXT("RMB release ends grip"), Interaction->EndGrip(Event));
    TestFalse(TEXT("grip release clears hand ownership"), Interaction->IsGripActive());
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabMomentaryComplianceTest,
    "PinkCab.Cockpit.Input.Compliance.PC_T_INP_004",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabMomentaryComplianceTest::RunTest(const FString& Parameters)
{
    UPinkCabCockpitInteractionComponent* Interaction = NewObject<UPinkCabCockpitInteractionComponent>();
    FPinkCabInteractionEvent Event;
    Interaction->SetCurrentTarget(FPinkCabInteractionControlSpec(TEXT("Gearbox"), true, false, true));
    TestFalse(TEXT("LMB rejects non-momentary target"), Interaction->BeginMomentary(10.0, Event));
    Interaction->SetCurrentTarget(FPinkCabInteractionControlSpec(TEXT("Horn"), false, true, false));
    const uint32 Before = Interaction->GetActuationSerial();
    TestTrue(TEXT("LMB begins authored momentary control"), Interaction->BeginMomentary(10.0, Event));
    TestTrue(TEXT("momentary state remains held"), Interaction->IsMomentaryActive());
    TestTrue(TEXT("LMB release ends momentary control"), Interaction->EndMomentary(10.18, Event));
    TestFalse(TEXT("release clears momentary state"), Interaction->IsMomentaryActive());
    TestTrue(TEXT("momentary actuation increments serial"), Interaction->GetActuationSerial() > Before);
    TestTrue(TEXT("horn hold duration is observable"), FMath::IsNearlyEqual(Interaction->GetLastMomentaryHoldSeconds(), 0.18, 1.0e-6));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabWheelComplianceTest,
    "PinkCab.Cockpit.Input.Compliance.PC_T_INP_005",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabWheelComplianceTest::RunTest(const FString& Parameters)
{
    UPinkCabCockpitInteractionComponent* Interaction = NewObject<UPinkCabCockpitInteractionComponent>();
    FPinkCabInteractionEvent Event;
    Interaction->SetCurrentTarget(FPinkCabInteractionControlSpec(TEXT("Horn"), false, true, false));
    TestFalse(TEXT("wheel rejects control without wheel capability"), Interaction->BuildWheelEvent(1, Event));
    Interaction->SetCurrentTarget(PinkCabInteractionSpecForTargetId(TEXT("Gearbox")));
    TestFalse(TEXT("gearbox wheel is disabled because H-gate owns mouse travel"),
        Interaction->BuildWheelEvent(-1, Event));

    Interaction->SetCurrentTarget(PinkCabInteractionSpecForTargetId(TEXT("PassengerDoor")));
    TestFalse(TEXT("wheel cannot move a grip-required lever before RMB grip"),
        Interaction->BuildWheelEvent(-1, Event));
    TestTrue(TEXT("RMB establishes grip for authored wheel lever"), Interaction->BeginGrip(Event));
    TestTrue(TEXT("wheel emits for the passenger-door lever"), Interaction->BuildWheelEvent(-1, Event));
    TestEqual(TEXT("wheel keeps signed step"), Event.SignedValue, -1);
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabBoundedTargetComplianceTest,
    "PinkCab.Cockpit.Input.Compliance.PC_T_INP_007",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabBoundedTargetComplianceTest::RunTest(const FString& Parameters)
{
    TArray<FPinkCabInteractionCandidate> Candidates;
    Candidates.Emplace(TEXT("Far"), 900.0f, 0.01f, true);
    Candidates.Emplace(TEXT("Horn"), 120.0f, 0.10f, true);
    Candidates.Emplace(TEXT("Gearbox"), 95.0f, 0.20f, true);
    Candidates.Emplace(TEXT("OutOfBudget"), 20.0f, 0.001f, true);
    TestEqual(TEXT("bounded selector ignores distance and candidate budget overflow"),
        FPinkCabInteractionTargetSelector::SelectCurrentTarget(Candidates, 3, 250.0f),
        FName(TEXT("Horn")));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabTransientCleanupComplianceTest,
    "PinkCab.Cockpit.Input.Compliance.PC_T_INP_008",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabTransientCleanupComplianceTest::RunTest(const FString& Parameters)
{
    UPinkCabCockpitInteractionComponent* Interaction = NewObject<UPinkCabCockpitInteractionComponent>();
    FPinkCabInteractionEvent Event;
    Interaction->SetGazeHeld(true);
    Interaction->SetQuickSlotHeld(2, true);
    Interaction->SetCurrentTarget(FPinkCabInteractionControlSpec(TEXT("Horn"), false, true, false));
    TestTrue(TEXT("precondition horn press begins"), Interaction->BeginMomentary(10.0, Event));
    FPinkCabCockpitState CockpitState;
    TestTrue(TEXT("horn press reaches authoritative cockpit state"), FPinkCabCockpitInteractionRouter::Apply(Event, CockpitState));
    TestTrue(TEXT("horn is active before cleanup"), CockpitState.IsHornActive());
    TArray<FPinkCabInteractionEvent> ReleaseEvents;
    Interaction->ResetTransientInputState(&ReleaseEvents);
    for (const FPinkCabInteractionEvent& ReleaseEvent : ReleaseEvents)
    {
        FPinkCabCockpitInteractionRouter::Apply(ReleaseEvent, CockpitState);
    }
    TestFalse(TEXT("cleanup releases gaze"), Interaction->IsGazeHeld());
    TestFalse(TEXT("cleanup releases grip"), Interaction->IsGripActive());
    TestFalse(TEXT("cleanup releases momentary state"), Interaction->IsMomentaryActive());
    TestTrue(TEXT("cleanup releases quick target ownership"), Interaction->GetCurrentQuickTargetId().IsNone());
    TestFalse(TEXT("cleanup emits release so horn cannot stick"), CockpitState.IsHornActive());
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabInteractionFrameProcessorTest,
    "PinkCab.Cockpit.Input.Compliance.FrameProcessor",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabInteractionFrameProcessorTest::RunTest(const FString& Parameters)
{
    UPinkCabCockpitInteractionComponent* Interaction = NewObject<UPinkCabCockpitInteractionComponent>();
    FPinkCabCockpitInteractionFrame Frame;
    Frame.bQuickRecall2Held = true;
    Frame.bMomentaryHeld = true;
    Frame.NowSeconds = 10.0;
    TArray<FPinkCabInteractionEvent> Events;
    Interaction->ProcessFrame(Frame, nullptr, Events);
    TestEqual(TEXT("frame processor emits horn press"), Events.Num(), 1);
    TestEqual(TEXT("frame processor targets recalled horn"), Events[0].TargetId, FName(TEXT("Horn")));

    Frame.bMomentaryHeld = false;
    Frame.NowSeconds = 10.2;
    Events.Reset();
    Interaction->ProcessFrame(Frame, nullptr, Events);
    TestEqual(TEXT("frame processor emits horn release"), Events.Num(), 1);
    TestEqual(TEXT("release keeps horn target"), Events[0].TargetId, FName(TEXT("Horn")));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabPhysicalFareControlCapabilitiesTest,
    "PinkCab.Cockpit.Input.Compliance.PhysicalFareControls",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabPhysicalFareControlCapabilitiesTest::RunTest(const FString& Parameters)
{
    const FPinkCabInteractionControlSpec Door = PinkCabInteractionSpecForTargetId(TEXT("PassengerDoor"));
    TestTrue(TEXT("passenger door lever requires grip"), Door.bSupportsGrip);
    TestTrue(TEXT("passenger door lever moves incrementally"), Door.bSupportsWheel);
    TestFalse(TEXT("passenger door is not a hidden momentary button"), Door.bSupportsMomentary);

    const FPinkCabInteractionControlSpec Meter = PinkCabInteractionSpecForTargetId(TEXT("Taximeter"));
    TestFalse(TEXT("taximeter button does not require grip"), Meter.bSupportsGrip);
    TestTrue(TEXT("taximeter START STOP RESET is a physical momentary control"), Meter.bSupportsMomentary);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabContinuousHandbrakeComplianceTest,
    "PinkCab.Cockpit.Input.Compliance.ContinuousHandbrake",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabContinuousHandbrakeComplianceTest::RunTest(const FString& Parameters)
{
    const FPinkCabInteractionControlSpec Spec =
        PinkCabInteractionSpecForTargetId(TEXT("Handbrake"));
    TestTrue(TEXT("handbrake requires RMB grip"), Spec.bSupportsGrip);
    TestFalse(TEXT("handbrake no longer uses wheel actuation"), Spec.bSupportsWheel);

    FPinkCabCockpitState State;
    State.SetHandbrakeAmount(0.37f);
    TestEqual(TEXT("handbrake preserves intermediate analog command"), State.GetHandbrakeAmount(), 0.37f);
    State.SetHandbrakeAmount(2.0f);
    TestEqual(TEXT("handbrake clamps high"), State.GetHandbrakeAmount(), 1.0f);
    State.SetHandbrakeAmount(-1.0f);
    TestEqual(TEXT("handbrake clamps low"), State.GetHandbrakeAmount(), 0.0f);
    return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabPawnTransientCleanupComplianceTest,
    "PinkCab.Cockpit.Input.Compliance.PawnTransientCleanup",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabPawnTransientCleanupComplianceTest::RunTest(const FString& Parameters)
{
    APinkCabChaosTatraPawn* Pawn = GetMutableDefault<APinkCabChaosTatraPawn>();
    UPinkCabCockpitInteractionComponent* Interaction = Pawn->GetCockpitInteraction();
    TestNotNull(TEXT("pawn owns interaction component"), Interaction);
    if (!Interaction) return false;
    FPinkCabInteractionEvent Event;
    Interaction->SetGazeHeld(true);
    Interaction->SetQuickSlotHeld(3, true);
    Interaction->SetCurrentTarget(PinkCabInteractionSpecForTargetId(TEXT("Gearbox")));
    TestTrue(TEXT("precondition grip begins"), Interaction->BeginGrip(Event));
    Pawn->ResetTransientCockpitInput();
    TestFalse(TEXT("pawn cleanup releases gaze"), Interaction->IsGazeHeld());
    TestFalse(TEXT("pawn cleanup releases grip"), Interaction->IsGripActive());
    TestTrue(TEXT("pawn cleanup releases quick recall"), Interaction->GetCurrentQuickTargetId().IsNone());
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabPrimaryPointerGearboxGripTest,
    "PinkCab.Cockpit.Input.Compliance.PrimaryPointerGearboxGrip",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPinkCabPrimaryPointerGearboxGripTest::RunTest(const FString& Parameters)
{
    UPinkCabCockpitInteractionComponent* Interaction = NewObject<UPinkCabCockpitInteractionComponent>();
    FPinkCabCockpitInteractionFrame Frame;
    Frame.bQuickRecall3Held = true;
    Frame.bMomentaryHeld = true;
    Frame.NowSeconds = 1.0;
    TArray<FPinkCabInteractionEvent> Events;

    Interaction->ProcessFrame(Frame, nullptr, Events);
    TestFalse(TEXT("LMB without RMB never grips the gearbox"), Interaction->IsGripActive());
    TestFalse(TEXT("LMB without RMB never starts lever manipulation"),
        Interaction->IsManipulationActive());
    TestEqual(TEXT("LMB without grip does not actuate grip-only gearbox"), Events.Num(), 0);

    Events.Reset();
    Frame.bMomentaryHeld = false;
    Frame.bGripHeld = true;
    Interaction->ProcessFrame(Frame, nullptr, Events);
    TestTrue(TEXT("RMB grips the gearbox"), Interaction->IsGripActive());
    TestFalse(TEXT("RMB alone only holds the gearbox ready"),
        Interaction->IsManipulationActive());
    TestEqual(TEXT("RMB emits one contextual grip-begin event"), Events.Num(), 1);
    if (Events.Num() == 1)
    {
        TestEqual(TEXT("grip begin keeps gearbox target"), Events[0].TargetId, FName(TEXT("Gearbox")));
        TestEqual(TEXT("grip begin uses grip gesture"), Events[0].Gesture, EPinkCabInteractionGesture::GripBegin);
        TestEqual(TEXT("grip begin signed value is positive"), Events[0].SignedValue, 1);
    }

    Events.Reset();
    Frame.bMomentaryHeld = true;
    Interaction->ProcessFrame(Frame, nullptr, Events);
    TestTrue(TEXT("LMB while RMB holds gearbox starts manipulation"),
        Interaction->IsManipulationActive());
    TestEqual(TEXT("lever manipulation itself does not emit a duplicate momentary action"),
        Events.Num(), 0);

    Frame.bMomentaryHeld = false;
    Interaction->ProcessFrame(Frame, nullptr, Events);
    TestFalse(TEXT("LMB release stops manipulation while RMB keeps grip"),
        Interaction->IsManipulationActive());
    TestTrue(TEXT("RMB still retains gearbox after LMB release"),
        Interaction->IsGripActive());

    Events.Reset();
    Frame.bGripHeld = false;
    Interaction->ProcessFrame(Frame, nullptr, Events);
    TestFalse(TEXT("RMB release returns gearbox grip ownership"), Interaction->IsGripActive());
    TestFalse(TEXT("RMB release leaves no manipulation active"),
        Interaction->IsManipulationActive());
    TestEqual(TEXT("RMB release emits one contextual grip-end event"), Events.Num(), 1);
    if (Events.Num() == 1)
    {
        TestEqual(TEXT("grip release keeps gearbox target"), Events[0].TargetId, FName(TEXT("Gearbox")));
        TestEqual(TEXT("grip release is signed negative"), Events[0].SignedValue, -1);
    }
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabContextualRmbPickTest,
    "PinkCab.Cockpit.Input.Compliance.ContextualRmbPickWithoutSpace",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPinkCabContextualRmbPickTest::RunTest(const FString& Parameters)
{
    UPinkCabCockpitInteractionComponent* Interaction = NewObject<UPinkCabCockpitInteractionComponent>();
    UPinkCabCockpitAssemblyComponent* Assembly = NewObject<UPinkCabCockpitAssemblyComponent>();
    USceneComponent* DoorControl = NewObject<USceneComponent>();
    DoorControl->SetRelativeLocation(FVector(100.0f, 0.0f, 0.0f));
    Assembly->RegisterExternalSlot(EPinkCabCockpitSlot::PassengerDoor, DoorControl);

    FPinkCabCockpitInteractionFrame Frame;
    Frame.bGripHeld = true;
    Frame.bGazeHeld = false;
    Frame.GazeOrigin = FVector::ZeroVector;
    Frame.GazeForward = FVector::ForwardVector;
    Frame.GazeMaxDistanceCm = 250.0f;
    Frame.NowSeconds = 2.0;

    TArray<FPinkCabInteractionEvent> Events;
    Interaction->ProcessFrame(Frame, Assembly, Events);

    TestEqual(TEXT("RMB alone picks the centered contextual physical target"),
        Interaction->GetCurrentTargetId(), FName(TEXT("PassengerDoor")));
    TestTrue(TEXT("RMB alone acquires contextual grip without Space"), Interaction->IsGripActive());
    TestEqual(TEXT("contextual RMB emits grip-begin event"), Events.Num(), 1);
    return true;
}
#endif
