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

            if (Telemetry.NormalizedThrottle < 0.35f || Telemetry.EngineRpm <= 1100.0f)
            {
                if ((FPlatformTime::Seconds() - State->PhaseStartSeconds) > 2.0)
                {
                    Test->AddError(FString::Printf(
                        TEXT("real Q+E path never produced usable throttle/RPM without wheel; throttle=%.3f rpm=%.1f"),
                        Telemetry.NormalizedThrottle, Telemetry.EngineRpm));
                    return true;
                }
                return false;
            }

            Test->TestEqual(TEXT("engine remains running with clutch down and direct E throttle"),
                Pawn->GetCockpitState().GetIgnitionState(), EPinkCabIgnitionState::Running);
            Test->TestTrue(TEXT("direct E produces useful throttle before any wheel pulse"),
                Telemetry.NormalizedThrottle >= 0.35f);
            Test->TestTrue(TEXT("clutch remains disengaged while revving"),
                Telemetry.NormalizedClutch >= 0.85f);
            Test->TestTrue(TEXT("direct E raises engine above idle before wheel adjustment"),
                Telemetry.EngineRpm > 1100.0f);

            State->Phase = 4;
            return false;
        }

        if (State->Phase == 4)
        {
            if (State->WheelPulsesApplied < 11)
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
            // normal 45% launch dose while Q is still held, then release clutch.
            State->Phase = 5;
            return false;
        }

        if (State->Phase == 5)
        {
            if (State->WheelDownPulsesApplied < 11)
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
            Test->TestTrue(TEXT("wheel-down restores a sane launch throttle before clutch release"),
                Telemetry.NormalizedThrottle >= 0.35f && Telemetry.NormalizedThrottle <= 0.55f);

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
            Test->TestTrue(TEXT("real PlayerController 45 percent launch moves the taxi decisively"),
                TravelCm > 40.0f);

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
            Pawn->SetSystemMenuOpen(false);
            Test->TestTrue(TEXT("ignition primed for physical input routing"),
                Pawn->ApplyCockpitInteraction({FName(TEXT("Ignition")), EPinkCabInteractionGesture::PressHold, 1}));
            InjectKey(*PC, EKeys::Four, IE_Pressed);
            State->Phase = 1;
            return false;
        case 1:
            Test->TestEqual(TEXT("4 quick recall selects handbrake"),
                Interaction->GetCurrentTargetId(), FName(TEXT("Handbrake")));
            InjectKey(*PC, EKeys::Four, IE_Released, 0.0f);
            InjectKey(*PC, EKeys::RightMouseButton, IE_Pressed);
            State->Phase = 2;
            return false;
        case 2:
            Test->TestEqual(TEXT("RMB grips recalled handbrake"),
                Interaction->GetActiveGripTargetId(), FName(TEXT("Handbrake")));
            InjectKey(*PC, EKeys::MouseY, IE_Axis, 500.0f);
            State->Phase = 3;
            return false;
        case 3:
            InjectKey(*PC, EKeys::MouseY, IE_Axis, 0.0f);
            Test->TestEqual(TEXT("real mouse axis fully releases parking handbrake"),
                Pawn->GetCockpitState().GetHandbrakeAmount(), 0.0f);
            InjectKey(*PC, EKeys::RightMouseButton, IE_Released, 0.0f);
            InjectKey(*PC, EKeys::Three, IE_Pressed);
            State->Phase = 4;
            return false;
        case 4:
            Test->TestEqual(TEXT("3 quick recall selects gearbox"),
                Interaction->GetCurrentTargetId(), FName(TEXT("Gearbox")));
            InjectKey(*PC, EKeys::Three, IE_Released, 0.0f);
            InjectKey(*PC, EKeys::RightMouseButton, IE_Pressed);
            State->Phase = 5;
            return false;
        case 5:
            Test->TestEqual(TEXT("RMB grips recalled gearbox"),
                Interaction->GetActiveGripTargetId(), FName(TEXT("Gearbox")));
            InjectKey(*PC, EKeys::MouseX, IE_Axis, -90.0f);
            State->Phase = 6;
            return false;
        case 6:
            InjectKey(*PC, EKeys::MouseX, IE_Axis, 0.0f);
            InjectKey(*PC, EKeys::MouseY, IE_Axis, 60.0f);
            State->Phase = 7;
            return false;
        case 7:
            InjectKey(*PC, EKeys::MouseY, IE_Axis, 0.0f);
            Test->TestEqual(TEXT("short real forward flick remains neutral"),
                Pawn->GetRequestedGear(), 0);
            InjectKey(*PC, EKeys::MouseY, IE_Axis, 45.0f);
            State->Phase = 8;
            return false;
        case 8:
            InjectKey(*PC, EKeys::MouseY, IE_Axis, 0.0f);
            Test->TestEqual(TEXT("continued real forward throw requests first"),
                Pawn->GetRequestedGear(), 1);
            InjectKey(*PC, EKeys::RightMouseButton, IE_Released, 0.0f);
            State->Phase = 9;
            return false;
        default:
            Test->TestFalse(TEXT("physical grip releases after RMB"),
                Interaction->IsGripActive());
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
    "PinkCab.Vehicle.PlayerInput.ControllerThrottleDrive",
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
