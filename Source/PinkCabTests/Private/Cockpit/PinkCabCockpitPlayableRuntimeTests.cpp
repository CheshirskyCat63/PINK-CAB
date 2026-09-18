#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "EngineUtils.h"
#include "Camera/CameraComponent.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "GameFramework/Controller.h"
#include "Cockpit/PinkCabCockpitInteractionComponent.h"
#include "Interaction/PinkCabSemanticInputRouter.h"
#include "Vehicle/PinkCabChaosTatraPawn.h"
#include "Vehicle/PinkCabVehicleInputFrame.h"

struct FPinkCabPlayableCockpitRuntimeState
{
    TWeakObjectPtr<APinkCabChaosTatraPawn> Pawn;
    FVector StartLocation = FVector::ZeroVector;
    double DriveStartSeconds = -1.0;
    double ReadyWaitStartSeconds = -1.0;
    bool bPrimed = false;
};

class FPinkCabPlayableCockpitDriveCommand final : public IAutomationLatentCommand
{
public:
    FPinkCabPlayableCockpitDriveCommand(
        FAutomationTestBase* InTest,
        TSharedRef<FPinkCabPlayableCockpitRuntimeState> InState)
        : Test(InTest), State(InState) {}

    virtual bool Update() override;
private:
    FAutomationTestBase* Test = nullptr;
    TSharedRef<FPinkCabPlayableCockpitRuntimeState> State;
};

bool FPinkCabPlayableCockpitDriveCommand::Update()
{
    UWorld* World = AutomationCommon::GetAnyGameWorld();
    if (!World) return false;

    if (!State->Pawn.IsValid())
    {
        for (TActorIterator<APinkCabChaosTatraPawn> It(World); It; ++It)
        {
            State->Pawn = *It;
            break;
        }
    }

    APinkCabChaosTatraPawn* Pawn = State->Pawn.Get();
    if (!Pawn) return false;

    if (!State->bPrimed)
    {
        AController* Controller = Pawn->GetController();
        Test->TestNotNull(TEXT("playable taxi is possessed"), Controller);
        Test->TestTrue(TEXT("playable taxi controller is local"), Controller && Controller->IsLocalController());
        Test->TestNotNull(TEXT("driver camera exists"), Pawn->GetDriverCamera());
        Test->TestTrue(TEXT("driver camera is active"), Pawn->GetDriverCamera() && Pawn->GetDriverCamera()->IsActive());
        Test->TestTrue(TEXT("system menu starts open before playable drive"), Pawn->IsSystemMenuOpen());
        Pawn->SetSystemMenuOpen(false);
        Test->TestFalse(TEXT("playable drive resumes from system menu"), Pawn->IsSystemMenuOpen());
        Pawn->SetActorTickEnabled(false);
        const FPinkCabSemanticInputRouter Router = FPinkCabSemanticInputRouter::CreateDefaults();
        Test->TestEqual(TEXT("canonical throttle is E"),
            Router.GetKeyForAction(EPinkCabSemanticAction::Throttle), EKeys::E);
        Test->TestEqual(TEXT("canonical gaze is Space"),
            Router.GetKeyForAction(EPinkCabSemanticAction::GazeHold), EKeys::SpaceBar);
        Test->TestEqual(TEXT("R has no player-facing action"),
            Router.Resolve(EKeys::R), EPinkCabSemanticAction::None);

        UPinkCabCockpitInteractionComponent* Interaction = Pawn->GetCockpitInteraction();
        Test->TestNotNull(TEXT("physical cockpit interaction exists"), Interaction);
        if (!Interaction) return true;

        FPinkCabInteractionEvent Event;
        Interaction->SetCurrentTarget(
            UPinkCabCockpitInteractionComponent::SpecForTargetId(TEXT("Ignition")));
        Test->TestTrue(TEXT("physical ignition press begins"), Interaction->BeginMomentary(1.0, Event));
        Test->TestTrue(TEXT("physical ignition event reaches pawn"), Pawn->ApplyCockpitInteraction(Event));
        Interaction->EndMomentary(1.1, Event);

        Interaction->SetCurrentTarget(
            UPinkCabCockpitInteractionComponent::SpecForTargetId(TEXT("Handbrake")));
        Test->TestTrue(TEXT("physical handbrake grip begins"), Interaction->BeginGrip(Event));
        Pawn->ApplyPhysicalControlMouseDelta(TEXT("Handbrake"), true, 0.0f, -500.0f, 0.1f);
        Interaction->EndGrip(Event);
        Pawn->ApplyPhysicalControlMouseDelta(NAME_None, false, 0.0f, 0.0f, 0.1f);
        Test->TestEqual(TEXT("stationary mouse pull fully releases parking lever"),
            Pawn->GetCockpitState().GetHandbrakeAmount(), 0.0f);

        Interaction->SetCurrentTarget(
            UPinkCabCockpitInteractionComponent::SpecForTargetId(TEXT("Gearbox")));
        Test->TestTrue(TEXT("physical gearbox grip begins"), Interaction->BeginGrip(Event));
        Pawn->ApplyPhysicalControlMouseDelta(TEXT("Gearbox"), true, -160.0f, 0.0f, 0.05f);
        Test->TestTrue(TEXT("visible lever follows continuous neutral cross-gate left"),
            Pawn->GetGearLeverVisualCursor().Equals(FVector2D(-1.0f, 0.0f), 0.01f));
        Test->TestEqual(TEXT("neutral cross-gate does not engage a gear"),
            Pawn->GetEngagedGear(), 0);
        Pawn->ApplyPhysicalControlMouseDelta(TEXT("Gearbox"), true, 0.0f, -140.0f, 0.05f);
        Test->TestTrue(TEXT("visible lever reaches first through the same physical cursor"),
            Pawn->GetGearLeverVisualCursor().Equals(FVector2D(-1.0f, 1.0f), 0.01f));
        Test->TestEqual(TEXT("physical H-gate requests first"),
            Pawn->GetRequestedGear(), 1);
        Test->TestEqual(TEXT("requested first is still separate from engagement before clutch validation"),
            Pawn->GetEngagedGear(), 0);
        Test->TestEqual(TEXT("cockpit lever selection mirrors requested gear"),
            Pawn->GetCockpitState().GetSelectedGear(), 1);
        const FPinkCabVehicleInputFrame ClutchFrame =
            FPinkCabVehicleInputFrame::FromDigital(false, true, false, false);
        Pawn->ApplyVehicleInputFrame(ClutchFrame, 0.0f);
        const FPinkCabVehicleInputFrame CoupledFrame =
            FPinkCabVehicleInputFrame::FromDigital(false, false, false, false);
        Pawn->ApplyVehicleInputFrame(CoupledFrame, 0.0f);
        Interaction->EndGrip(Event);

        UChaosWheeledVehicleMovementComponent* Movement = Pawn->GetChaosMovement();
        Test->TestNotNull(TEXT("live Chaos movement exists"), Movement);
        if (Movement)
        {
            Test->TestTrue(TEXT("ignition enabled mechanical simulation"), Movement->bMechanicalSimEnabled);
            Test->TestFalse(TEXT("legacy bool handbrake path remains disabled"), Movement->GetHandbrakeInput());
            Test->TestEqual(TEXT("validated first gear reaches Chaos after clutch release"),
                Movement->GetTargetGear(), 1);
        }

        const FPinkCabVehicleInputFrame GazeThrottleFrame = FPinkCabVehicleInputFrame::FromRouter(
            Router, [](const FKey& Key) { return Key == EKeys::E || Key == EKeys::SpaceBar; });
        const float SteeringBeforeGaze = Pawn->GetSteeringCommand();
        Pawn->ApplyVehicleInputFrame(GazeThrottleFrame, 4.0f);
        Test->TestEqual(TEXT("Space gaze owns mouse so steering does not move"),
            Pawn->GetSteeringCommand(), SteeringBeforeGaze);

        const FPinkCabVehicleInputFrame ThrottleFrame = FPinkCabVehicleInputFrame::FromRouter(
            Router, [](const FKey& Key) { return Key == EKeys::E; });
        Pawn->ApplyVehicleInputFrame(ThrottleFrame, 4.0f);
        Test->TestTrue(TEXT("Space release returns mouse steering ownership"),
            Pawn->GetSteeringCommand() > SteeringBeforeGaze);
        Pawn->ApplyMouseSteeringDelta(-4.0f, false);

        const FVector BeforeCleanup = Pawn->GetActorLocation();
        Pawn->ResetTransientCockpitInput();
        Test->TestEqual(TEXT("transient cleanup is not a recovery teleport"),
            Pawn->GetActorLocation(), BeforeCleanup);
        Test->TestFalse(TEXT("transient cleanup releases gaze"), Interaction->IsGazeHeld());

        State->ReadyWaitStartSeconds = FPlatformTime::Seconds();
        State->bPrimed = true;
    }

    if (State->DriveStartSeconds < 0.0)
    {
        FPinkCabVehicleTelemetry ReadyTelemetry;
        Pawn->GetPinkCabDynamicsProvider().ReadTelemetry(ReadyTelemetry);
        int32 ContactCount = 0;
        for (const FPinkCabWheelTelemetry& Wheel : ReadyTelemetry.Wheels)
        {
            ContactCount += Wheel.bInContact ? 1 : 0;
        }
        if (ContactCount < 2)
        {
            if ((FPlatformTime::Seconds() - State->ReadyWaitStartSeconds) > 2.0)
            {
                Test->TestTrue(TEXT("taxi reaches stable road contact before drive"), false);
                return true;
            }
            const FPinkCabVehicleInputFrame HoldFrame =
                FPinkCabVehicleInputFrame::FromDigital(false, false, true, false);
            Pawn->ApplyVehicleInputFrame(HoldFrame, 0.0f);
            return false;
        }
        State->StartLocation = Pawn->GetActorLocation();
        State->DriveStartSeconds = FPlatformTime::Seconds();
    }

    const FPinkCabSemanticInputRouter Router = FPinkCabSemanticInputRouter::CreateDefaults();
    const FPinkCabVehicleInputFrame ThrottleFrame = FPinkCabVehicleInputFrame::FromRouter(
        Router, [](const FKey& Key) { return Key == EKeys::E; });
    Pawn->ApplyVehicleInputFrame(ThrottleFrame, 0.0f);

    if ((FPlatformTime::Seconds() - State->DriveStartSeconds) < 3.0)
    {
        return false;
    }

    const FVector EndLocation = Pawn->GetActorLocation();
    const float TravelCm = FVector::Dist2D(EndLocation, State->StartLocation);
    Test->TestTrue(TEXT("driver camera remains active after drive"),
        Pawn->GetDriverCamera() && Pawn->GetDriverCamera()->IsActive());
    Test->TestNotNull(TEXT("pawn remains possessed after drive"), Pawn->GetController());

    FPinkCabVehicleTelemetry Telemetry;
    Test->TestTrue(TEXT("live telemetry remains available"),
        Pawn->GetPinkCabDynamicsProvider().ReadTelemetry(Telemetry));
    Test->AddInfo(FString::Printf(
        TEXT("playable diagnostics travel=%.1fcm speed=%.2fkmh gear=%d rpm=%.1f throttle=%.2f handbrake=%.2f"),
        TravelCm, Telemetry.SpeedKmh, Telemetry.CurrentGear, Telemetry.EngineRpm,
        Telemetry.NormalizedThrottle, Telemetry.NormalizedHandbrake));
    Test->TestTrue(TEXT("canonical E throttle moves taxi"), TravelCm > 100.0f);
    Test->TestTrue(TEXT("live telemetry reports engine rpm"), Telemetry.EngineRpm > 750.0f);
    Test->TestEqual(TEXT("live telemetry exposes four wheels"), Telemetry.Wheels.Num(), 4);

    const FPinkCabVehicleInputFrame BrakeFrame = FPinkCabVehicleInputFrame::FromDigital(
        false, false, true, false);
    Pawn->ApplyVehicleInputFrame(BrakeFrame, 0.0f);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabPlayableCockpitRuntimeTest,
    "PinkCab.Cockpit.Playable.Runtime",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabPlayableCockpitRuntimeTest::RunTest(const FString& Parameters)
{
    const bool bOpened = AutomationOpenMap(TEXT("/Game/Dev/Maps/L_PinkCab_ChaosWeave"), true);
    TestTrue(TEXT("cockpit playable map opens for PIE"), bOpened);
    if (!bOpened) return false;

    const TSharedRef<FPinkCabPlayableCockpitRuntimeState> State =
        MakeShared<FPinkCabPlayableCockpitRuntimeState>();
    ADD_LATENT_AUTOMATION_COMMAND(FPinkCabPlayableCockpitDriveCommand(this, State));
    return true;
}

#endif
