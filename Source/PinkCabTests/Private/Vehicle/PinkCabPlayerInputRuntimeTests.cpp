#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "GenericPlatform/GenericPlatformInputDeviceMapper.h"
#include "InputKeyEventArgs.h"
#include "Interaction/PinkCabInteractionModel.h"
#include "Vehicle/PinkCabChaosTatraPawn.h"

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
    bool bWheelPulsePendingTick = false;
    double PhaseStartSeconds = 0.0;
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
            State->Phase = 3;
            return false;
        }

        if (State->Phase == 3)
        {
            Test->TestTrue(TEXT("PlayerController stores real E held state after a frame"),
                PC->IsInputKeyDown(EKeys::E));
            Test->TestEqual(TEXT("engine is still running before throttle dosing"),
                Pawn->GetCockpitState().GetIgnitionState(), EPinkCabIgnitionState::Running);
            State->PhaseStartSeconds = FPlatformTime::Seconds();
            State->Phase = 4;
            return false;
        }

        if (State->Phase == 4)
        {
            if (State->WheelPulsesApplied < 5)
            {
                if (!State->bWheelPulsePendingTick)
                {
                    // Axis injection is consumed by the next Unreal input frame.
                    // The gameplay assertion is the resulting 5% throttle dose,
                    // not an immediate analog-key-state read in this latent callback.
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
                if ((FPlatformTime::Seconds() - State->PhaseStartSeconds) > 2.0)
                {
                    Test->AddError(FString::Printf(
                        TEXT("five real wheel pulses did not build throttle target; throttle=%.3f rpm=%.1f"),
                        Telemetry.NormalizedThrottle, Telemetry.EngineRpm));
                    return true;
                }
                return false;
            }

            Test->TestEqual(TEXT("engine remains running while throttle is dosed with clutch down"),
                Pawn->GetCockpitState().GetIgnitionState(), EPinkCabIgnitionState::Running);
            Test->TestTrue(TEXT("real E plus five wheel pulses reaches useful throttle"),
                Telemetry.NormalizedThrottle >= 0.20f);
            Test->TestTrue(TEXT("clutch remains disengaged until throttle is prepared"),
                Telemetry.NormalizedClutch >= 0.85f);

            InjectKey(*PC, EKeys::Q, IE_Released, 0.0f);
            State->DriveStartSeconds = FPlatformTime::Seconds();
            State->Phase = 5;
            return false;
        }

        if (State->Phase == 5)
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
            Test->TestTrue(TEXT("real PlayerController launch moves the taxi"), TravelCm > 20.0f);

            InjectKey(*PC, EKeys::E, IE_Released, 0.0f);
            State->Phase = 6;
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
