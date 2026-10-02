#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "EngineUtils.h"
#include "Vehicle/PinkCabChaosVehicleMovementComponent.h"
#include "GameFramework/Controller.h"
#include "Interaction/PinkCabInteractionModel.h"
#include "Runtime/PinkCabChaosTatraPawn.h"
#include "Vehicle/PinkCabVehicleInputFrame.h"
#include "World/PinkCabVerticalAcceptanceCourse.h"

struct FCoreCodeCompleteRuntimeState
{
    TWeakObjectPtr<APinkCabChaosTatraPawn> Pawn;
    double RunStartSeconds = -1.0;
    FVector StartLocation = FVector::ZeroVector;
    int64 LastMechanicalStep = -1;
    double SimulatedSeconds = 0.0;
    bool bSteeringGestureApplied = false;
    bool bCockpitPrimed = false;
    bool bSawLiveTelemetry = false;
    float MaxObservedSpeedKmh = 0.0f;
};

class FCoreCodeCompleteRuntimeProbeCommand final : public IAutomationLatentCommand
{
public:
    FCoreCodeCompleteRuntimeProbeCommand(
        FAutomationTestBase* InTest,
        TSharedRef<FCoreCodeCompleteRuntimeState> InState)
        : Test(InTest), State(InState) {}

    virtual bool Update() override;

private:
    FAutomationTestBase* Test = nullptr;
    TSharedRef<FCoreCodeCompleteRuntimeState> State;
};

bool FCoreCodeCompleteRuntimeProbeCommand::Update()
{
    if (State->RunStartSeconds < 0.0)
    {
        State->RunStartSeconds = FPlatformTime::Seconds();
    }
    if (FPlatformTime::Seconds() - State->RunStartSeconds > 30.0)
    {
        Test->AddError(TEXT("Runtime smoke exceeded 30 seconds waiting for its world, pawn or drive phase"));
        return true;
    }
    UWorld* World = AutomationCommon::GetAnyGameWorld();
    if (!World) return false;

    if (!State->Pawn.IsValid())
    {
        for (TActorIterator<APinkCabChaosTatraPawn> It(World); It; ++It)
        {
            State->Pawn = *It;
            State->StartLocation = It->GetActorLocation();
            Test->TestTrue(TEXT("system menu starts open before exact-head drive"), It->IsSystemMenuOpen());
            It->SetSystemMenuOpen(false);
            Test->TestFalse(TEXT("exact-head drive resumes from system menu"), It->IsSystemMenuOpen());
            It->SetActorTickEnabled(false);
            AController* Controller = It->GetController();
            Test->TestNotNull(TEXT("exact-head Tatra pawn is possessed"), Controller);
            Test->TestTrue(TEXT("exact-head controller is local"),
                Controller && Controller->IsLocalController());
            break;
        }
    }

    APinkCabChaosTatraPawn* Pawn = State->Pawn.Get();
    if (!Pawn) return false;

    UPinkCabChaosVehicleMovementComponent* Movement =
        Cast<UPinkCabChaosVehicleMovementComponent>(Pawn->GetChaosMovement());
    Test->TestNotNull(TEXT("native Tatra exposes live Chaos movement"), Movement);
    if (!Movement) return true;
    if (!State->bCockpitPrimed)
    {
        Test->TestTrue(TEXT("ignition reaches native Tatra"),
            Pawn->ApplyCockpitInteraction({FName(TEXT("Ignition")),
                EPinkCabInteractionGesture::PressHold, 1}));
        Pawn->ApplyPhysicalControlMouseDelta(
            FName(TEXT("Handbrake")), true, 0.0f, 500.0f, 0.1f);
        Pawn->ApplyPhysicalControlMouseDelta(
            NAME_None, false, 0.0f, 0.0f, 0.1f);
        Test->TestEqual(TEXT("physical parking lever releases"),
            Pawn->GetCockpitState().GetHandbrakeAmount(), 0.0f);

        const FPinkCabVehicleInputFrame ClutchFrame =
            FPinkCabVehicleInputFrame::FromDigital(false, true, false, false);
        Pawn->ApplyVehicleInputFrame(ClutchFrame, 0.0f);
        Pawn->ApplyPhysicalControlMouseDelta(
            FName(TEXT("Gearbox")), true, -640.0f, 0.0f, 0.05f);
        Pawn->ApplyPhysicalControlMouseDelta(
            FName(TEXT("Gearbox")), true, 0.0f, 480.0f, 0.05f);
        Pawn->ApplyVehicleInputFrame(ClutchFrame, 0.0f);
        Pawn->ApplyVehicleInputFrame(
            FPinkCabVehicleInputFrame::FromDigital(false, false, false, false), 0.0f);

        Test->TestTrue(TEXT("mechanical simulation enabled"), Movement->bMechanicalSimEnabled);
        Test->TestFalse(TEXT("legacy bool handbrake path stays disabled"), Movement->GetHandbrakeInput());
        Test->TestEqual(TEXT("physical H-gate engages authoritative first"), Pawn->GetEngagedGear(), 1);
        Test->TestEqual(TEXT("single driveline leaves native transmission neutral"), Movement->GetTargetGear(), 0);
        State->bCockpitPrimed = true;
    }

    const int64 MechanicalStep = Movement->GetPinkCabMechanicalIntegrationStepCount();
    const float DeltaSeconds = State->LastMechanicalStep < 0
        ? 1.0f / 60.0f
        : static_cast<float>(MechanicalStep - State->LastMechanicalStep)
            * Movement->GetPinkCabLastMechanicalIntegrationDeltaSeconds();
    State->LastMechanicalStep = MechanicalStep;
    if (DeltaSeconds <= KINDA_SMALL_NUMBER) return false;
    State->SimulatedSeconds += DeltaSeconds;
    const double Elapsed = State->SimulatedSeconds;

    // One deliberate gesture has fixed travel regardless of latent polling rate.
    // The calibrated stationary steering scale needs thousands of mouse counts;
    // one count per poll barely moved the wheel and did not exercise a real turn.
    const bool bApplySteeringGesture = Elapsed >= 2.5 && !State->bSteeringGestureApplied;
    const float SteeringMouseDelta = bApplySteeringGesture ? 2400.0f : 0.0f;
    State->bSteeringGestureApplied |= bApplySteeringGesture;

    // Keep ignition, engagement and coupling on the production cockpit bridge.
    // A fresh provider-only control struct would silently replace them with N.
    const FPinkCabVehicleInputFrame DriveFrame =
        FPinkCabVehicleInputFrame::FromDigital(false, false, false, true);
    Movement->SetSleeping(false);
    Pawn->ApplyVehicleInputFrame(DriveFrame, SteeringMouseDelta, DeltaSeconds);

    FPinkCabVehicleTelemetry Telemetry;
    if (Pawn->GetPinkCabDynamicsProvider().ReadTelemetry(Telemetry))
    {
        State->bSawLiveTelemetry = true;
        State->MaxObservedSpeedKmh = FMath::Max(
            State->MaxObservedSpeedKmh, FMath::Abs(Telemetry.SpeedKmh));
    }
    if (Elapsed < 4.0) return false;

    const FVector EndLocation = Pawn->GetActorLocation();
    const float ForwardTravelCm = EndLocation.X - State->StartLocation.X;
    const float LateralTravelCm = FMath::Abs(EndLocation.Y - State->StartLocation.Y);
    Test->AddInfo(FString::Printf(
        TEXT("core PIE diagnostics forward=%.1fcm lateral=%.1fcm maxSpeed=%.2fkmh nativeForward=%.1fcm/s steering=%.3f simulated=%.3fs"),
        ForwardTravelCm,
        LateralTravelCm,
        State->MaxObservedSpeedKmh,
        Movement->GetForwardSpeed(),
        Pawn->GetSteeringCommand(),
        Elapsed));
    // This runtime smoke proves forward movement and steering. Specific slip
    // behavior belongs to the fixed-load/surface physics calibration fixtures.
    Test->TestTrue(TEXT("exact-head heavy Chaos Tatra moved forward"), ForwardTravelCm > 50.0f);
    Test->TestTrue(TEXT("deliberate mouse gesture produces meaningful right steering"),
        Pawn->GetSteeringCommand() > 0.2f);
    Test->TestTrue(TEXT("exact-head steering produced lateral motion"), LateralTravelCm > 1.0f);
    Test->TestTrue(TEXT("live provider telemetry was observed"), State->bSawLiveTelemetry);
    Test->TestTrue(TEXT("live telemetry observed meaningful speed"),
        State->MaxObservedSpeedKmh > 1.0f);
    Test->TestTrue(TEXT("native movement still has forward speed"),
        FMath::Abs(Movement->GetForwardSpeed()) > 30.0f);
    Test->TestFalse(TEXT("torque arcade control remains off"), Movement->TorqueControl.Enabled);
    Test->TestFalse(TEXT("target rotation arcade control remains off"),
        Movement->TargetRotationControl.Enabled);
    Test->TestFalse(TEXT("stabilize arcade control remains off"),
        Movement->StabilizeControl.Enabled);

    FPinkCabVehicleTelemetry FinalTelemetry;
    Test->TestTrue(TEXT("provider returns final live telemetry"),
        Pawn->GetPinkCabDynamicsProvider().ReadTelemetry(FinalTelemetry));
    Test->TestTrue(TEXT("live engine rpm is running"), FinalTelemetry.EngineRpm > 750.0f);
    Test->TestEqual(TEXT("live telemetry has four wheel slots"), FinalTelemetry.Wheels.Num(), 4);

    Pawn->ApplyVehicleInputFrame(
        FPinkCabVehicleInputFrame::FromDigital(false, false, true, false), 0.0f);
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabCoreCodeCompleteRuntimePIETest,
    "PinkCab.Core.CodeComplete.Runtime.PIE",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabCoreCodeCompleteRuntimePIETest::RunTest(const FString& Parameters)
{
    const APinkCabVerticalAcceptanceCourse* Course =
        GetDefault<APinkCabVerticalAcceptanceCourse>();
    TestNotNull(TEXT("vertical acceptance course contract exists"), Course);
    TestEqual(TEXT("presentation geometry remains non-authoritative"),
        Course ? Course->GetAuthorityMarker() : FString(),
        FString(TEXT("NON_AUTHORITATIVE_GEOMETRY")));
    TestTrue(TEXT("vertical presentation stays presentation-only"),
        Course && Course->IsPresentationOnly());

    const bool bOpened = AutomationOpenMap(
        TEXT("/Game/Dev/Maps/L_PinkCab_ChaosWeave"), true);
    TestTrue(TEXT("primitive Chaos acceptance map opens"), bOpened);
    if (!bOpened) return false;

    const TSharedRef<FCoreCodeCompleteRuntimeState> State =
        MakeShared<FCoreCodeCompleteRuntimeState>();
    ADD_LATENT_AUTOMATION_COMMAND(
        FCoreCodeCompleteRuntimeProbeCommand(this, State));
    return true;
}

#endif
