#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "GenericPlatform/GenericPlatformInputDeviceMapper.h"
#include "InputKeyEventArgs.h"
#include "Cockpit/PinkCabCockpitInteractionComponent.h"
#include "Interaction/PinkCabInteractionModel.h"
#include "Runtime/PinkCabChaosTatraPawn.h"

namespace
{
void InjectKey(APlayerController& PC, const FKey& Key, const EInputEvent Event, const float Amount = 1.0f)
{
    const FInputDeviceId Device = IPlatformInputDeviceMapper::Get().GetDefaultInputDevice();
    PC.InputKey(FInputKeyEventArgs(
        nullptr, Device, Key, Event, Amount, false, FPlatformTime::Cycles64()));
}
}

struct FPinkCabPlayerInputRuntimeState
{
    TWeakObjectPtr<APinkCabChaosTatraPawn> Pawn;
    TWeakObjectPtr<APlayerController> Controller;
    FVector StartLocation = FVector::ZeroVector;
    int32 Phase = 0;
    int32 WheelPulsesApplied = 0;
    int32 WheelDownPulsesApplied = 0;
    bool bWheelPulsePendingTick = false;
    bool bWheelDownPulsePendingTick = false;
    bool bFullThrottleWaitStarted = false;
    double PhaseStartSeconds = 0.0;
    double FullThrottleStartSeconds = 0.0;
    double DriveStartSeconds = 0.0;
};

class FPinkCabPlayerInputDriveCommand final : public IAutomationLatentCommand
{
public:
    FPinkCabPlayerInputDriveCommand(
        FAutomationTestBase* InTest,
        TSharedRef<FPinkCabPlayerInputRuntimeState> InState)
        : Test(InTest), State(InState) {}

    virtual bool Update() override
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

        if (!State->Controller.IsValid())
        {
            State->Controller = Cast<APlayerController>(Pawn->GetController());
        }
        APlayerController* PC = State->Controller.Get();
        if (!PC) return false;

        FPinkCabVehicleTelemetry Telemetry;
        if (!Pawn->GetPinkCabDynamicsProvider().ReadTelemetry(Telemetry)) return false;

        if (State->Phase == 0)
        {
            Pawn->SetSystemMenuOpen(false);
            Test->TestFalse(TEXT("system menu closed for real input path"), Pawn->IsSystemMenuOpen());
            Test->TestTrue(TEXT("ignition can be deterministically primed"),
                Pawn->ApplyCockpitInteraction({FName(TEXT("Ignition")), EPinkCabInteractionGesture::PressHold, 1}));
            Pawn->ApplyPhysicalControlMouseDelta(TEXT("Handbrake"), true, 0.0f, 500.0f, 0.1f);
            Test->TestEqual(TEXT("parking brake released before input proof"),
                Pawn->GetCockpitState().GetHandbrakeAmount(), 0.0f);

            State->StartLocation = Pawn->GetActorLocation();
            InjectKey(*PC, EKeys::Q, IE_Pressed, 1.0f);
            State->PhaseStartSeconds = FPlatformTime::Seconds();
            State->Phase = 1;
            return false;
        }

        if (State->Phase == 1)
        {
            if (Telemetry.NormalizedClutch < 0.90f)
            {
                if ((FPlatformTime::Seconds() - State->PhaseStartSeconds) > 2.0)
                {
                    Test->AddError(FString::Printf(
                        TEXT("real Q never depressed clutch: %.3f"), Telemetry.NormalizedClutch));
                    return true;
                }
                return false;
            }
            Test->TestTrue(TEXT("PlayerController stores real Q held state after a frame"),
                PC->IsInputKeyDown(EKeys::Q));
            Test->TestEqual(TEXT("Q stages the right hand over the gearbox"),
                Pawn->GetCockpitInteraction()->GetCurrentTargetId(), FName(TEXT("Gearbox")));
            Test->TestFalse(TEXT("Q alone never grips gearbox so steering remains available"),
                Pawn->GetCockpitInteraction()->IsGripActive());
            Test->TestEqual(TEXT("engine stays running with clutch disengaged"),
                Pawn->GetCockpitState().GetIgnitionState(), EPinkCabIgnitionState::Running);

            Pawn->ApplyPhysicalControlMouseDelta(TEXT("Gearbox"), true, -160.0f, 0.0f, 0.05f);
            Pawn->ApplyPhysicalControlMouseDelta(TEXT("Gearbox"), true, 0.0f, 140.0f, 0.05f);
            State->PhaseStartSeconds = FPlatformTime::Seconds();
            State->Phase = 2;
            return false;
        }

        if (State->Phase == 2)
        {
            if (Pawn->GetEngagedGear() != 1)
            {
                if ((FPlatformTime::Seconds() - State->PhaseStartSeconds) > 1.0)
                {
                    Test->AddError(FString::Printf(
                        TEXT("first did not engage with clutch down; requested=%d engaged=%d"),
                        Pawn->GetRequestedGear(), Pawn->GetEngagedGear()));
                    return true;
                }
                return false;
            }
            Test->TestEqual(TEXT("owner-forward H-gate gesture engages first"), Pawn->GetEngagedGear(), 1);
            InjectKey(*PC, EKeys::E, IE_Pressed, 1.0f);
            State->PhaseStartSeconds = FPlatformTime::Seconds();
            State->Phase = 3;
            return false;
        }

        if (State->Phase == 3)
        {
            Test->TestTrue(TEXT("PlayerController stores real E held state after a frame"),
                PC->IsInputKeyDown(EKeys::E));
            Test->TestTrue(TEXT("Q remains physically held while E is pressed"),
                PC->IsInputKeyDown(EKeys::Q));
            Test->TestTrue(TEXT("clutch remains disengaged before throttle dosing"),
                Telemetry.NormalizedClutch >= 0.85f);
            Test->TestTrue(TEXT("new launch E without wheel does not invent throttle"),
                Telemetry.NormalizedThrottle <= 0.01f);

            State->Phase = 4;
            return false;
        }

        if (State->Phase == 4)
        {
            if (State->WheelPulsesApplied < 20)
            {
                if (!State->bWheelPulsePendingTick)
                {
                    InjectKey(*PC, EKeys::MouseWheelAxis, IE_Axis, 1.0f);
                    State->bWheelPulsePendingTick = true;
                    return false;
                }

                InjectKey(*PC, EKeys::MouseWheelAxis, IE_Axis, 0.0f);
                State->bWheelPulsePendingTick = false;
                ++State->WheelPulsesApplied;
                return false;
            }

            if (!State->bFullThrottleWaitStarted)
            {
                State->bFullThrottleWaitStarted = true;
                State->FullThrottleStartSeconds = FPlatformTime::Seconds();
            }

            if (Telemetry.NormalizedThrottle < 0.95f || Telemetry.EngineRpm < 7990.0f)
            {
                if ((FPlatformTime::Seconds() - State->FullThrottleStartSeconds) > 3.0)
                {
                    Test->AddError(FString::Printf(
                        TEXT("real 100%% Q+E path did not reach redline band; throttle=%.3f rpm=%.1f"),
                        Telemetry.NormalizedThrottle, Telemetry.EngineRpm));
                    return true;
                }
                return false;
            }

            Test->TestTrue(TEXT("E+wheel can fine-adjust direct throttle to full"),
                Telemetry.NormalizedThrottle >= 0.95f);
            Test->TestTrue(TEXT("100 percent real PlayerController throttle reaches the 8500 rpm band"),
                Telemetry.EngineRpm >= 7990.0f);
            Test->TestTrue(TEXT("clutch remains disengaged during free rev proof"),
                Telemetry.NormalizedClutch >= 0.85f);

            // Redline proof and launch proof are intentionally separate. At 100%
            // in first gear the authored car is a burnout case. Return to the
            // established 25% clean-launch calibration while Q is still held,
            // then release clutch.
            State->Phase = 5;
            return false;
        }

        if (State->Phase == 5)
        {
            if (State->WheelDownPulsesApplied < 15)
            {
                if (!State->bWheelDownPulsePendingTick)
                {
                    InjectKey(*PC, EKeys::MouseWheelAxis, IE_Axis, -1.0f);
                    State->bWheelDownPulsePendingTick = true;
                    return false;
                }

                InjectKey(*PC, EKeys::MouseWheelAxis, IE_Axis, 0.0f);
                State->bWheelDownPulsePendingTick = false;
                ++State->WheelDownPulsesApplied;
                return false;
            }

            if (Telemetry.NormalizedThrottle > 0.55f)
            {
                return false;
            }

            Test->TestTrue(TEXT("Q remains held while throttle is returned to launch dose"),
                PC->IsInputKeyDown(EKeys::Q));
            Test->TestTrue(TEXT("wheel-down restores the established 25 percent clean-launch band before clutch release"),
                Telemetry.NormalizedThrottle >= 0.20f && Telemetry.NormalizedThrottle <= 0.30f);

            State->StartLocation = Pawn->GetActorLocation();
            InjectKey(*PC, EKeys::Q, IE_Released, 0.0f);
            State->DriveStartSeconds = FPlatformTime::Seconds();
            State->Phase = 6;
            return false;
        }

        if (State->Phase == 6)
        {
            if ((FPlatformTime::Seconds() - State->DriveStartSeconds) < 2.5)
            {
                return false;
            }

            const float TravelCm = FVector::Dist2D(Pawn->GetActorLocation(), State->StartLocation);
            Test->AddInfo(FString::Printf(
                TEXT("controller-input diagnostics travel=%.1fcm throttle=%.3f clutch=%.3f gear=%d rpm=%.1f E=%d Q=%d"),
                TravelCm, Telemetry.NormalizedThrottle, Telemetry.NormalizedClutch,
                Telemetry.CurrentGear, Telemetry.EngineRpm,
                PC->IsInputKeyDown(EKeys::E) ? 1 : 0,
                PC->IsInputKeyDown(EKeys::Q) ? 1 : 0));
            Test->TestEqual(TEXT("controller path keeps first engaged"), Telemetry.CurrentGear, 1);
            Test->TestEqual(TEXT("engine remains running through launch"),
                Pawn->GetCockpitState().GetIgnitionState(), EPinkCabIgnitionState::Running);
            Test->TestTrue(TEXT("real PlayerController 25 percent clean launch moves the taxi"),
                TravelCm > 20.0f);

            InjectKey(*PC, EKeys::E, IE_Released, 0.0f);
            State->Phase = 7;
            return false;
        }

        Test->TestFalse(TEXT("PlayerController releases E after a frame"),
            PC->IsInputKeyDown(EKeys::E));
        Test->TestFalse(TEXT("PlayerController releases Q after a frame"),
            PC->IsInputKeyDown(EKeys::Q));
        return true;
    }

private:
    FAutomationTestBase* Test = nullptr;
    TSharedRef<FPinkCabPlayerInputRuntimeState> State;
};

struct FPinkCabPhysicalPlayerInputState
{
    TWeakObjectPtr<APinkCabChaosTatraPawn> Pawn;
    TWeakObjectPtr<APlayerController> Controller;
    float InitialHandbrake = 0.0f;
    float SteeringBeforeRmb = 0.0f;
    float SteeringAtManipulationStart = 0.0f;
    FVector DriveStartLocation = FVector::ZeroVector;
    int32 WheelPulsesApplied = 0;
    bool bWheelPulsePendingTick = false;
    double PhaseStartSeconds = 0.0;
    double DriveStartSeconds = 0.0;
    bool bInitialInputModeSettled = false;
    bool bInputWarmupPressed = false;
    double InputWarmupStartSeconds = 0.0;
    int32 Phase = 0;
};

class FPinkCabPhysicalPlayerInputCommand final : public IAutomationLatentCommand
{
public:
    FPinkCabPhysicalPlayerInputCommand(
        FAutomationTestBase* InTest,
        TSharedRef<FPinkCabPhysicalPlayerInputState> InState)
        : Test(InTest), State(InState) {}

    virtual bool Update() override
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
        if (!State->Controller.IsValid())
        {
            State->Controller = Cast<APlayerController>(Pawn->GetController());
        }
        APlayerController* PC = State->Controller.Get();
        UPinkCabCockpitInteractionComponent* Interaction = Pawn->GetCockpitInteraction();
        if (!PC || !Interaction) return false;

        switch (State->Phase)
        {
        case 0:
            if (!State->bInitialInputModeSettled)
            {
                Pawn->SetSystemMenuOpen(false);
                Test->TestFalse(TEXT("system menu closed before physical input routing"),
                    Pawn->IsSystemMenuOpen());
                Test->TestTrue(TEXT("ignition primed for physical input routing"),
                    Pawn->ApplyCockpitInteraction({FName(TEXT("Ignition")), EPinkCabInteractionGesture::PressHold, 1}));
                State->InputWarmupStartSeconds = FPlatformTime::Seconds();
                State->bInitialInputModeSettled = true;
                return false;
            }
            // Standalone AutomationOpenMap can expose a controller before its
            // input state retains injected keys. Probe with an unbound key so
            // this test does not depend on another test warming the viewport.
            if (!State->bInputWarmupPressed)
            {
                InjectKey(*PC, EKeys::F12, IE_Pressed);
                State->bInputWarmupPressed = true;
                return false;
            }
            if (!PC->IsInputKeyDown(EKeys::F12))
            {
                InjectKey(*PC, EKeys::F12, IE_Released, 0.0f);
                State->bInputWarmupPressed = false;
                if ((FPlatformTime::Seconds() - State->InputWarmupStartSeconds) > 2.0)
                {
                    Test->AddError(TEXT("PlayerController input stack never retained the neutral warmup key"));
                    return true;
                }
                return false;
            }
            InjectKey(*PC, EKeys::F12, IE_Released, 0.0f);
            InjectKey(*PC, EKeys::Four, IE_Pressed);
            State->Phase = 1;
            return false;
        case 1:
            Test->TestEqual(TEXT("4 quick recall selects handbrake"),
                Interaction->GetCurrentTargetId(), FName(TEXT("Handbrake")));
            State->InitialHandbrake = Pawn->GetCockpitState().GetHandbrakeAmount();
            InjectKey(*PC, EKeys::Four, IE_Released, 0.0f);
            InjectKey(*PC, EKeys::Q, IE_Pressed);
            InjectKey(*PC, EKeys::RightMouseButton, IE_Pressed);
            State->Phase = 2;
            return false;
        case 2:
            Test->TestTrue(TEXT("Q may coexist while recalled handbrake stays selected"),
                PC->IsInputKeyDown(EKeys::Q));
            Test->TestEqual(TEXT("Q staging never steals recalled handbrake from RMB"),
                Interaction->GetActiveGripTargetId(), FName(TEXT("Handbrake")));
            Test->AddInfo(FString::Printf(
                TEXT("RECOVERY_R1_TRACE phase=handbrake_grip target=%s grip=%d manip=%d steer=%.3f handbrake=%.3f"),
                *Interaction->GetActiveGripTargetId().ToString(),
                Interaction->IsGripActive() ? 1 : 0,
                Interaction->IsManipulationActive() ? 1 : 0,
                Pawn->GetSteeringCommand(),
                Pawn->GetCockpitState().GetHandbrakeAmount()));
            InjectKey(*PC, EKeys::MouseY, IE_Axis, 500.0f);
            State->Phase = 3;
            return false;
        case 3:
            InjectKey(*PC, EKeys::MouseY, IE_Axis, 0.0f);
            Test->TestEqual(TEXT("RMB alone only grips; handbrake does not move before LMB action"),
                Pawn->GetCockpitState().GetHandbrakeAmount(), State->InitialHandbrake);
            InjectKey(*PC, EKeys::LeftMouseButton, IE_Pressed);
            InjectKey(*PC, EKeys::MouseY, IE_Axis, 500.0f);
            State->Phase = 4;
            return false;
        case 4:
            InjectKey(*PC, EKeys::MouseY, IE_Axis, 0.0f);
            Test->TestEqual(TEXT("RMB plus LMB mouse axis releases parking handbrake"),
                Pawn->GetCockpitState().GetHandbrakeAmount(), 0.0f);
            Test->AddInfo(FString::Printf(
                TEXT("RECOVERY_R1_TRACE phase=handbrake_manip target=%s grip=%d manip=%d handbrake=%.3f"),
                *Interaction->GetActiveGripTargetId().ToString(),
                Interaction->IsGripActive() ? 1 : 0,
                Interaction->IsManipulationActive() ? 1 : 0,
                Pawn->GetCockpitState().GetHandbrakeAmount()));
            InjectKey(*PC, EKeys::LeftMouseButton, IE_Released, 0.0f);
            InjectKey(*PC, EKeys::RightMouseButton, IE_Released, 0.0f);
            InjectKey(*PC, EKeys::Q, IE_Released, 0.0f);
            State->Phase = 5;
            return false;
        case 5:
            Test->TestTrue(TEXT("consumed handbrake target clears after complete release"),
                Interaction->GetCurrentTargetId().IsNone());
            InjectKey(*PC, EKeys::Three, IE_Pressed);
            State->Phase = 6;
            return false;
        case 6:
            Test->TestEqual(TEXT("3 quick recall selects gearbox"),
                Interaction->GetCurrentTargetId(), FName(TEXT("Gearbox")));
            InjectKey(*PC, EKeys::Three, IE_Released, 0.0f);
            InjectKey(*PC, EKeys::RightMouseButton, IE_Pressed);
            State->Phase = 7;
            return false;
        case 7:
            Test->TestEqual(TEXT("RMB grips recalled gearbox"),
                Interaction->GetActiveGripTargetId(), FName(TEXT("Gearbox")));
            State->SteeringBeforeRmb = Pawn->GetSteeringCommand();
            InjectKey(*PC, EKeys::MouseX, IE_Axis, -90.0f);
            InjectKey(*PC, EKeys::MouseY, IE_Axis, 105.0f);
            State->Phase = 8;
            return false;
        case 8:
            InjectKey(*PC, EKeys::MouseX, IE_Axis, 0.0f);
            InjectKey(*PC, EKeys::MouseY, IE_Axis, 0.0f);
            Test->TestEqual(TEXT("RMB alone does not move gearbox or request a gear"),
                Pawn->GetRequestedGear(), 0);
            Test->TestTrue(TEXT("RMB-only grip keeps mouse steering live in the injected left direction"),
                Pawn->GetSteeringCommand() < State->SteeringBeforeRmb - 0.0005f);
            Test->AddInfo(FString::Printf(
                TEXT("RECOVERY_R1_TRACE phase=gearbox_grip target=%s grip=%d manip=%d steer=%.3f requested=%d"),
                *Interaction->GetActiveGripTargetId().ToString(),
                Interaction->IsGripActive() ? 1 : 0,
                Interaction->IsManipulationActive() ? 1 : 0,
                Pawn->GetSteeringCommand(),
                Pawn->GetRequestedGear()));
            State->SteeringAtManipulationStart = Pawn->GetSteeringCommand();
            InjectKey(*PC, EKeys::LeftMouseButton, IE_Pressed);
            InjectKey(*PC, EKeys::MouseX, IE_Axis, -90.0f);
            State->Phase = 9;
            return false;
        case 9:
            InjectKey(*PC, EKeys::MouseX, IE_Axis, 0.0f);
            InjectKey(*PC, EKeys::MouseY, IE_Axis, 60.0f);
            State->Phase = 10;
            return false;
        case 10:
            InjectKey(*PC, EKeys::MouseY, IE_Axis, 0.0f);
            Test->TestEqual(TEXT("short LMB manipulation remains neutral"),
                Pawn->GetRequestedGear(), 0);
            Test->TestTrue(TEXT("lever manipulation holds the existing steering command"),
                FMath::IsNearlyEqual(
                    Pawn->GetSteeringCommand(), State->SteeringAtManipulationStart, 0.02f));
            InjectKey(*PC, EKeys::MouseY, IE_Axis, 45.0f);
            State->Phase = 11;
            return false;
        case 11:
            InjectKey(*PC, EKeys::MouseY, IE_Axis, 0.0f);
            Test->TestEqual(TEXT("continued RMB plus LMB forward throw requests first"),
                Pawn->GetRequestedGear(), 1);
            Test->TestEqual(TEXT("no-clutch first request keeps actual gearbox in neutral"),
                Pawn->GetEngagedGear(), 0);
            Test->AddInfo(FString::Printf(
                TEXT("RECOVERY_R1_TRACE phase=gearbox_manip target=%s grip=%d manip=%d steer=%.3f requested=%d engaged=%d"),
                *Interaction->GetActiveGripTargetId().ToString(),
                Interaction->IsGripActive() ? 1 : 0,
                Interaction->IsManipulationActive() ? 1 : 0,
                Pawn->GetSteeringCommand(),
                Pawn->GetRequestedGear(),
                Pawn->GetEngagedGear()));
            InjectKey(*PC, EKeys::LeftMouseButton, IE_Released, 0.0f);
            InjectKey(*PC, EKeys::RightMouseButton, IE_Released, 0.0f);
            State->Phase = 12;
            return false;
        case 12:
            Test->TestFalse(TEXT("physical grip releases after RMB"),
                Interaction->IsGripActive());
            Test->TestTrue(TEXT("consumed gearbox target clears after complete release"),
                Interaction->GetCurrentTargetId().IsNone());
            InjectKey(*PC, EKeys::Q, IE_Pressed);
            State->PhaseStartSeconds = FPlatformTime::Seconds();
            State->Phase = 13;
            return false;
        case 13:
            if (Pawn->GetEngagedGear() != 1)
            {
                if ((FPlatformTime::Seconds() - State->PhaseStartSeconds) > 2.0)
                {
                    Test->AddError(FString::Printf(
                        TEXT("pending first never engaged after real Q clutch input; requested=%d engaged=%d"),
                        Pawn->GetRequestedGear(), Pawn->GetEngagedGear()));
                    return true;
                }
                return false;
            }
            Test->TestTrue(TEXT("real Q accepts the already-requested first gear"),
                PC->IsInputKeyDown(EKeys::Q));
            Test->TestEqual(TEXT("actual gearbox engages first only after Q"),
                Pawn->GetEngagedGear(), 1);
            InjectKey(*PC, EKeys::E, IE_Pressed);
            State->Phase = 14;
            return false;
        case 14:
        {
            FPinkCabVehicleTelemetry Telemetry;
            if (!Pawn->GetPinkCabDynamicsProvider().ReadTelemetry(Telemetry))
            {
                return false;
            }
            Test->TestTrue(TEXT("Q remains held while E begins launch dosing"),
                PC->IsInputKeyDown(EKeys::Q));
            Test->TestTrue(TEXT("E is held for launch dosing"),
                PC->IsInputKeyDown(EKeys::E));
            // Prove E-alone before the first wheel pulse is injected. Once the
            // positive axis pulse is pending, the next pawn tick is allowed to
            // consume it even though WheelPulsesApplied is incremented one latent
            // command update later.
            if (State->WheelPulsesApplied == 0 && !State->bWheelPulsePendingTick)
            {
                Test->TestTrue(TEXT("fresh E press alone still has zero throttle"),
                    Telemetry.NormalizedThrottle <= 0.01f);
            }

            if (State->WheelPulsesApplied < 5)
            {
                if (!State->bWheelPulsePendingTick)
                {
                    InjectKey(*PC, EKeys::MouseWheelAxis, IE_Axis, 1.0f);
                    State->bWheelPulsePendingTick = true;
                    return false;
                }
                InjectKey(*PC, EKeys::MouseWheelAxis, IE_Axis, 0.0f);
                State->bWheelPulsePendingTick = false;
                ++State->WheelPulsesApplied;
                return false;
            }

            if (Telemetry.NormalizedThrottle < 0.20f)
            {
                return false;
            }

            Test->TestTrue(TEXT("five E+wheel pulses create the 25 percent launch band"),
                Telemetry.NormalizedThrottle >= 0.20f && Telemetry.NormalizedThrottle <= 0.30f);
            State->DriveStartLocation = Pawn->GetActorLocation();
            InjectKey(*PC, EKeys::Q, IE_Released, 0.0f);
            State->DriveStartSeconds = FPlatformTime::Seconds();
            State->Phase = 15;
            return false;
        }
        case 15:
        {
            if ((FPlatformTime::Seconds() - State->DriveStartSeconds) < 2.5)
            {
                return false;
            }
            FPinkCabVehicleTelemetry Telemetry;
            if (!Pawn->GetPinkCabDynamicsProvider().ReadTelemetry(Telemetry))
            {
                return false;
            }
            const float TravelCm = FVector::Dist2D(Pawn->GetActorLocation(), State->DriveStartLocation);
            Test->AddInfo(FString::Printf(
                TEXT("RECOVERY_R1_TRACE phase=playercontroller_drive travel=%.1f throttle=%.3f clutch=%.3f requested=%d engaged=%d"),
                TravelCm,
                Telemetry.NormalizedThrottle,
                Telemetry.NormalizedClutch,
                Pawn->GetRequestedGear(),
                Pawn->GetEngagedGear()));
            Test->TestEqual(TEXT("PlayerController drivetrain path keeps first engaged"),
                Pawn->GetEngagedGear(), 1);
            Test->TestTrue(TEXT("PlayerController cockpit path moves the taxi"),
                TravelCm > 20.0f);
            InjectKey(*PC, EKeys::E, IE_Released, 0.0f);
            State->Phase = 16;
            return false;
        }
        default:
            Test->TestFalse(TEXT("Q is released after physical drivetrain path"),
                PC->IsInputKeyDown(EKeys::Q));
            Test->TestFalse(TEXT("E is released after physical drivetrain path"),
                PC->IsInputKeyDown(EKeys::E));
            return true;
        }
    }

private:
    FAutomationTestBase* Test = nullptr;
    TSharedRef<FPinkCabPhysicalPlayerInputState> State;
};

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabPhysicalPlayerInputRuntimeTest,
    "PinkCab.Vehicle.PlayerInput.PhysicalControlRouting",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabPhysicalPlayerInputRuntimeTest::RunTest(const FString& Parameters)
{
    const bool bOpened = AutomationOpenMap(TEXT("/Game/Dev/Maps/L_PinkCab_ChaosWeave"), true);
    TestTrue(TEXT("physical-input map opens for PIE"), bOpened);
    if (!bOpened) return false;
    ADD_LATENT_AUTOMATION_COMMAND(FPinkCabPhysicalPlayerInputCommand(
        this, MakeShared<FPinkCabPhysicalPlayerInputState>()));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabPlayerInputRuntimeTest,
    "PinkCab.Vehicle.PlayerInput.ControllerPedalDriveWithFixtureDrivetrain",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabPlayerInputRuntimeTest::RunTest(const FString& Parameters)
{
    const bool bOpened = AutomationOpenMap(TEXT("/Game/Dev/Maps/L_PinkCab_ChaosWeave"), true);
    TestTrue(TEXT("player-input map opens for PIE"), bOpened);
    if (!bOpened) return false;

    ADD_LATENT_AUTOMATION_COMMAND(FPinkCabPlayerInputDriveCommand(
        this, MakeShared<FPinkCabPlayerInputRuntimeState>()));
    return true;
}

#endif
