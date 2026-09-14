#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Cockpit/PinkCabCockpitInteractionComponent.h"
#include "Interaction/PinkCabInteractionModel.h"
#include "Vehicle/PinkCabChaosTatraPawn.h"
#include "Vehicle/PinkCabCockpitInteractionRouter.h"
#include "Vehicle/PinkCabVehicleInputFrame.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabCockpitQuickPriorityTest,
    "PinkCab.Cockpit.Input.QuickPriority",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabCockpitQuickPriorityTest::RunTest(const FString& Parameters)
{
    UPinkCabCockpitInteractionComponent* Interaction = NewObject<UPinkCabCockpitInteractionComponent>();
    Interaction->SetQuickSlotHeld(1, true);
    TestEqual(TEXT("slot 1 selects turn signals"), Interaction->GetCurrentQuickTargetId(), FName(TEXT("TurnSignals")));
    Interaction->SetQuickSlotHeld(3, true);
    TestEqual(TEXT("latest held quick key wins"), Interaction->GetCurrentQuickTargetId(), FName(TEXT("Gearbox")));
    Interaction->SetQuickSlotHeld(3, false);
    TestEqual(TEXT("release falls back to older held quick key"), Interaction->GetCurrentQuickTargetId(), FName(TEXT("TurnSignals")));
    Interaction->SetQuickSlotHeld(1, false);
    TestTrue(TEXT("releasing all quick keys clears quick target"), Interaction->GetCurrentQuickTargetId().IsNone());
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabCockpitGazeAndAttentionTest,
    "PinkCab.Cockpit.Input.GazeAttention",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabCockpitGazeAndAttentionTest::RunTest(const FString& Parameters)
{
    UPinkCabCockpitInteractionComponent* Interaction = NewObject<UPinkCabCockpitInteractionComponent>();
    Interaction->SetGazeHeld(true);
    const float Preserved = APinkCabChaosTatraPawn::IntegrateMouseSteering(0.42f, 30.0f, Interaction->IsGazeHeld());
    TestEqual(TEXT("Space gaze ownership preserves steering command"), Preserved, 0.42f);

    Interaction->SetCandidateTarget(FName(TEXT("Horn")));
    TestTrue(TEXT("ATTENTION commits a valid target"), Interaction->CommitAttention());
    Interaction->SetCandidateTarget(NAME_None);
    TestEqual(TEXT("ATTENTION retains committed target"), Interaction->GetAttentionTargetId(), FName(TEXT("Horn")));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabCockpitGoAndPedalCoexistenceTest,
    "PinkCab.Cockpit.Input.GoAndPedals",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabCockpitGoAndPedalCoexistenceTest::RunTest(const FString& Parameters)
{
    UPinkCabCockpitInteractionComponent* Interaction = NewObject<UPinkCabCockpitInteractionComponent>();
    Interaction->SetCandidateTarget(FName(TEXT("Horn")));
    TestTrue(TEXT("ATTENTION commits horn"), Interaction->CommitAttention());

    FPinkCabInteractionEvent Press;
    TestTrue(TEXT("RMB GO emits press command"), Interaction->BeginGo(Press));
    TestEqual(TEXT("GO targets ATTENTION target"), Press.TargetId, FName(TEXT("Horn")));
    TestEqual(TEXT("GO uses press-hold gesture"), Press.Gesture, EPinkCabInteractionGesture::PressHold);
    TestEqual(TEXT("GO press is positive"), Press.SignedValue, 1);

    FPinkCabInteractionEvent Release;
    TestTrue(TEXT("RMB GO release emits release command"), Interaction->EndGo(Release));
    TestEqual(TEXT("GO release targets same control"), Release.TargetId, FName(TEXT("Horn")));
    TestEqual(TEXT("GO release is negative"), Release.SignedValue, -1);

    const FPinkCabVehicleInputFrame Frame = FPinkCabVehicleInputFrame::FromDigital(false, false, true, true);
    const FPinkCabVehicleControlState Controls = Frame.ToControlState(0.0f, 0.0f);
    TestEqual(TEXT("W brake remains live"), Controls.Brake, 1.0f);
    TestEqual(TEXT("E throttle may coexist with W brake"), Controls.Throttle, 1.0f);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabCockpitAuxiliaryRouterTest,
    "PinkCab.Cockpit.Input.AuxiliaryRouter",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabCockpitAuxiliaryRouterTest::RunTest(const FString& Parameters)
{
    FPinkCabCockpitState State;
    TestTrue(TEXT("turn signal wheel command applies"), FPinkCabCockpitInteractionRouter::Apply({TEXT("TurnSignals"), EPinkCabInteractionGesture::WheelIncrement, 1}, State));
    TestEqual(TEXT("positive turn signal command selects right"), State.GetTurnSignalDirection(), 1);

    TestTrue(TEXT("horn press applies"), FPinkCabCockpitInteractionRouter::Apply({TEXT("Horn"), EPinkCabInteractionGesture::PressHold, 1}, State));
    TestTrue(TEXT("horn state becomes active"), State.IsHornActive());
    TestTrue(TEXT("horn release applies"), FPinkCabCockpitInteractionRouter::Apply({TEXT("Horn"), EPinkCabInteractionGesture::PressHold, -1}, State));
    TestFalse(TEXT("horn state clears on release"), State.IsHornActive());
    TestTrue(TEXT("lights detent applies"), FPinkCabCockpitInteractionRouter::Apply({TEXT("Lights"), EPinkCabInteractionGesture::WheelIncrement, 1}, State));
    TestEqual(TEXT("lights advance to first active detent"), State.GetLightMode(), 1);

    TestTrue(TEXT("wipers detent applies"), FPinkCabCockpitInteractionRouter::Apply({TEXT("Wipers"), EPinkCabInteractionGesture::WheelIncrement, 1}, State));
    TestEqual(TEXT("wipers advance to low speed"), State.GetWiperMode(), 1);

    TestTrue(TEXT("washer press applies"), FPinkCabCockpitInteractionRouter::Apply({TEXT("Washer"), EPinkCabInteractionGesture::PressHold, 1}, State));
    TestTrue(TEXT("washer is active while GO held"), State.IsWasherActive());
    TestTrue(TEXT("washer release applies"), FPinkCabCockpitInteractionRouter::Apply({TEXT("Washer"), EPinkCabInteractionGesture::PressHold, -1}, State));
    TestFalse(TEXT("washer clears on GO release"), State.IsWasherActive());
    return true;
}

#endif

#if WITH_DEV_AUTOMATION_TESTS
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabCockpitPawnInteractionCompositionTest,
    "PinkCab.Cockpit.Input.PawnComposition",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabCockpitPawnInteractionCompositionTest::RunTest(const FString& Parameters)
{
    const APinkCabChaosTatraPawn* Pawn = GetDefault<APinkCabChaosTatraPawn>();
    TestNotNull(TEXT("Tatra pawn owns focused cockpit interaction component"), Pawn->GetCockpitInteraction());
    return Pawn->GetCockpitInteraction() != nullptr;
}
#endif

#if WITH_DEV_AUTOMATION_TESTS
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabCockpitWheelCommandTest,
    "PinkCab.Cockpit.Input.WheelCommand",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabCockpitWheelCommandTest::RunTest(const FString& Parameters)
{
    UPinkCabCockpitInteractionComponent* Interaction = NewObject<UPinkCabCockpitInteractionComponent>();
    Interaction->SetQuickSlotHeld(3, true);
    FPinkCabInteractionEvent Event;
    TestTrue(TEXT("wheel command emits for current quick target"), Interaction->BuildWheelEvent(1, Event));
    TestEqual(TEXT("wheel command targets gearbox"), Event.TargetId, FName(TEXT("Gearbox")));
    TestEqual(TEXT("wheel command uses detent gesture"), Event.Gesture, EPinkCabInteractionGesture::WheelIncrement);
    TestEqual(TEXT("wheel command preserves signed step"), Event.SignedValue, 1);
    return true;
}
#endif
