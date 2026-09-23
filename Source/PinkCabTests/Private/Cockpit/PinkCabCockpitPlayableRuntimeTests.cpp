#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "EngineUtils.h"
#include "Camera/CameraComponent.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "GameFramework/Controller.h"
#include "Cockpit/PinkCabCockpitInteractionComponent.h"
#include "Interaction/PinkCabSemanticInputRouter.h"
#include "Runtime/PinkCabChaosTatraPawn.h"
#include "Vehicle/PinkCabVehicleInputFrame.h"

struct FPinkCabPlayableCockpitRuntimeState
{
    TWeakObjectPtr<APinkCabChaosTatraPawn> Pawn;
    FVector StartLocation = FVector::ZeroVector;
    FVector ReverseStartLocation = FVector::ZeroVector;
    double DriveStartSeconds = -1.0;
    double ReverseDriveStartSeconds = -1.0;
    double ReadyWaitStartSeconds = -1.0;
    bool bReverseProved = false;
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
        Pawn->ApplyPhysicalControlMouseDelta(TEXT("Handbrake"), true, 0.0f, 500.0f, 0.1f);
        Test->TestEqual(TEXT("pushing handbrake away fully releases parking lever"),
            Pawn->GetCockpitState().GetHandbrakeAmount(), 0.0f);
        Pawn->ApplyPhysicalControlMouseDelta(TEXT("Handbrake"), true, 0.0f, -110.0f, 0.05f);
        Test->TestTrue(TEXT("pulling handbrake toward driver applies analog pressure"),
            Pawn->GetCockpitState().GetHandbrakeAmount() > 0.0f);
        Pawn->ApplyPhysicalControlMouseDelta(TEXT("Handbrake"), true, 0.0f, 500.0f, 0.05f);
        Test->TestEqual(TEXT("pushing away again releases before driving"),
            Pawn->GetCockpitState().GetHandbrakeAmount(), 0.0f);
        Interaction->EndGrip(Event);
        Pawn->ApplyPhysicalControlMouseDelta(NAME_None, false, 0.0f, 0.0f, 0.1f);

        Interaction->SetCurrentTarget(
            UPinkCabCockpitInteractionComponent::SpecForTargetId(TEXT("Gearbox")));
        Test->TestTrue(TEXT("physical gearbox grip begins"), Interaction->BeginGrip(Event));
        Pawn->ApplyPhysicalControlMouseDelta(TEXT("Gearbox"), true, -160.0f, 0.0f, 0.05f);
        Test->TestTrue(TEXT("visible lever follows continuous neutral cross-gate left"),
            Pawn->GetGearLeverVisualCursor().Equals(FVector2D(-1.0f, 0.0f), 0.01f));
        Test->TestEqual(TEXT("neutral cross-gate does not engage a gear"),
            Pawn->GetEngagedGear(), 0);
        Pawn->ApplyPhysicalControlMouseDelta(TEXT("Gearbox"), true, 0.0f, 140.0f, 0.05f);
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

        // Physically traverse first -> neutral -> right corridor -> reverse.
        // Driver-space rearward motion exits the top row into the neutral cross-gate.
        Pawn->ApplyPhysicalControlMouseDelta(TEXT("Gearbox"), true, 0.0f, -100.0f, 0.05f);
        Test->TestEqual(TEXT("first exits into neutral corridor before crossing right"),
            Pawn->GetRequestedGear(), 0);
        Pawn->ApplyPhysicalControlMouseDelta(TEXT("Gearbox"), true, 320.0f, 0.0f, 0.05f);
        Pawn->ApplyPhysicalControlMouseDelta(TEXT("Gearbox"), true, 0.0f, -140.0f, 0.05f);
        Test->TestEqual(TEXT("physical bottom-right gate requests reverse"), Pawn->GetRequestedGear(), -1);
        Pawn->ApplyVehicleInputFrame(ClutchFrame, 0.0f);
        Pawn->ApplyVehicleInputFrame(CoupledFrame, 0.0f);
        Test->TestEqual(TEXT("reverse engages through common validator"), Pawn->GetEngagedGear(), -1);
        if (Movement)
        {
            Test->TestEqual(TEXT("validated reverse reaches Chaos target gear"), Movement->GetTargetGear(), -1);
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

    if (!State->bReverseProved)
    {
        FPinkCabVehicleTelemetry ReverseTelemetry;
        Pawn->GetPinkCabDynamicsProvider().ReadTelemetry(ReverseTelemetry);
        int32 ReverseContactCount = 0;
        for (const FPinkCabWheelTelemetry& Wheel : ReverseTelemetry.Wheels)
        {
            ReverseContactCount += Wheel.bInContact ? 1 : 0;
        }
        if (ReverseContactCount < 2)
        {
            if ((FPlatformTime::Seconds() - State->ReadyWaitStartSeconds) > 2.0)
            {
                Test->TestTrue(TEXT("taxi reaches stable road contact before reverse proof"), false);
                return true;
            }
            const FPinkCabVehicleInputFrame HoldFrame =
                FPinkCabVehicleInputFrame::FromDigital(false, false, true, false);
            Pawn->ApplyVehicleInputFrame(HoldFrame, 0.0f);
            return false;
        }

        if (State->ReverseDriveStartSeconds < 0.0)
        {
            State->ReverseStartLocation = Pawn->GetActorLocation();
            State->ReverseDriveStartSeconds = FPlatformTime::Seconds();
        }

        const FPinkCabVehicleInputFrame ReverseThrottleFrame =
            FPinkCabVehicleInputFrame::FromDigital(false, false, false, true);
        Pawn->ApplyVehicleInputFrame(ReverseThrottleFrame, 0.0f);
        if ((FPlatformTime::Seconds() - State->ReverseDriveStartSeconds) < 1.5)
        {
            return false;
        }

        Pawn->GetPinkCabDynamicsProvider().ReadTelemetry(ReverseTelemetry);
        const float ReverseTravelCm = FVector::Dist2D(
            Pawn->GetActorLocation(), State->ReverseStartLocation);
        UChaosWheeledVehicleMovementComponent* ReverseMovement = Pawn->GetChaosMovement();
        const float RearLeftDriveTorque =
            ReverseTelemetry.Wheels.IsValidIndex(2) ? ReverseTelemetry.Wheels[2].DriveTorque : 0.0f;
        const float RearRightDriveTorque =
            ReverseTelemetry.Wheels.IsValidIndex(3) ? ReverseTelemetry.Wheels[3].DriveTorque : 0.0f;
        Test->AddInfo(FString::Printf(
            TEXT("reverse diagnostics requested=%d engaged=%d current=%d target=%d speed=%.2f travel=%.1f rearTorque=(%.1f,%.1f) throttle=%.2f rpm=%.1f"),
            Pawn->GetRequestedGear(),
            Pawn->GetEngagedGear(),
            ReverseMovement ? ReverseMovement->GetCurrentGear() : 99,
            ReverseMovement ? ReverseMovement->GetTargetGear() : 99,
            ReverseTelemetry.SpeedKmh,
            ReverseTravelCm,
            RearLeftDriveTorque,
            RearRightDriveTorque,
            ReverseTelemetry.NormalizedThrottle,
            ReverseTelemetry.EngineRpm));
        Test->TestTrue(TEXT("engaged reverse produces negative forward speed"),
            ReverseTelemetry.SpeedKmh < -0.25f);
        Test->TestTrue(TEXT("engaged reverse physically moves taxi backward while tires spin"),
            ReverseTravelCm > 10.0f);
        const bool bRearLeftBurnout =
            ReverseTelemetry.Wheels.IsValidIndex(2)
            && (ReverseTelemetry.Wheels[2].bIsSlipping
                || ReverseTelemetry.Wheels[2].bIsSkidding
                || ReverseTelemetry.Wheels[2].SlipMagnitude > 20.0f);
        const bool bRearRightBurnout =
            ReverseTelemetry.Wheels.IsValidIndex(3)
            && (ReverseTelemetry.Wheels[3].bIsSlipping
                || ReverseTelemetry.Wheels[3].bIsSkidding
                || ReverseTelemetry.Wheels[3].SlipMagnitude > 20.0f);
        Test->TestTrue(TEXT("full-throttle reverse spins left rear driven tire"),
            bRearLeftBurnout);
        Test->TestTrue(TEXT("full-throttle reverse spins right rear driven tire"),
            bRearRightBurnout);

        const FPinkCabVehicleInputFrame BrakeFrame =
            FPinkCabVehicleInputFrame::FromDigital(false, false, true, false);
        Pawn->ApplyVehicleInputFrame(BrakeFrame, 0.0f);

        // Return R -> neutral -> left corridor -> first after actual reverse motion.
        Pawn->ApplyPhysicalControlMouseDelta(TEXT("Gearbox"), true, 0.0f, 100.0f, 0.05f);
        Test->TestEqual(TEXT("reverse exits into neutral corridor before crossing left"),
            Pawn->GetRequestedGear(), 0);
        Pawn->ApplyPhysicalControlMouseDelta(TEXT("Gearbox"), true, -320.0f, 0.0f, 0.05f);
        Pawn->ApplyPhysicalControlMouseDelta(TEXT("Gearbox"), true, 0.0f, 140.0f, 0.05f);
        const FPinkCabVehicleInputFrame ClutchFrame =
            FPinkCabVehicleInputFrame::FromDigital(false, true, false, false);
        Pawn->ApplyVehicleInputFrame(ClutchFrame, 0.0f);
        const FPinkCabVehicleInputFrame CoupledFrame =
            FPinkCabVehicleInputFrame::FromDigital(false, false, false, false);
        Pawn->ApplyVehicleInputFrame(CoupledFrame, 0.0f);
        Test->TestEqual(TEXT("first is restored after physical reverse motion proof"),
            Pawn->GetEngagedGear(), 1);
        State->bReverseProved = true;
        State->ReadyWaitStartSeconds = FPlatformTime::Seconds();
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
    Test->TestTrue(TEXT("canonical E throttle moves the heavy taxi without rocket acceleration"), TravelCm > 50.0f);
    Test->TestTrue(TEXT("live telemetry reports engine rpm"), Telemetry.EngineRpm > 750.0f);
    Test->TestEqual(TEXT("live telemetry exposes four wheels"), Telemetry.Wheels.Num(), 4);
    const bool bForwardLeftBurnout =
        Telemetry.Wheels.IsValidIndex(2)
        && (Telemetry.Wheels[2].bIsSlipping
            || Telemetry.Wheels[2].bIsSkidding
            || Telemetry.Wheels[2].SlipMagnitude > 20.0f);
    const bool bForwardRightBurnout =
        Telemetry.Wheels.IsValidIndex(3)
        && (Telemetry.Wheels[3].bIsSlipping
            || Telemetry.Wheels[3].bIsSkidding
            || Telemetry.Wheels[3].SlipMagnitude > 20.0f);
    Test->TestTrue(TEXT("full-throttle first spins left rear driven tire"), bForwardLeftBurnout);
    Test->TestTrue(TEXT("full-throttle first spins right rear driven tire"), bForwardRightBurnout);

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
