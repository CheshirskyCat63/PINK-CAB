#include "Runtime/PinkCabChaosTatraPawn.h"
#include "Runtime/PinkCabDriverUiComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"
#include "Cockpit/PinkCabCockpitAssemblyComponent.h"
#include "Cockpit/PinkCabCockpitInteractionComponent.h"
#include "Cockpit/PinkCabCockpitPresentationState.h"
#include "Cockpit/PinkCabCockpitServiceBridge.h"
#include "Cockpit/PinkCabCockpitVisualDriverComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Interaction/PinkCabPlayerInputAdapter.h"
#include "Vehicle/PinkCabVehicleInputFrame.h"
#include "Vehicle/PinkCabVehicleInputResponse.h"

void APinkCabChaosTatraPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
    if (PlayerInputComponent)
    {
        PlayerInputComponent->BindKey(
            EKeys::MouseScrollUp,
            IE_Pressed,
            this,
            &APinkCabChaosTatraPawn::CaptureMouseWheelUp);
        PlayerInputComponent->BindKey(
            EKeys::MouseScrollDown,
            IE_Pressed,
            this,
            &APinkCabChaosTatraPawn::CaptureMouseWheelDown);
    }
}

void APinkCabChaosTatraPawn::CaptureMouseWheelUp()
{
    QueueMouseWheelStep(1);
}

void APinkCabChaosTatraPawn::CaptureMouseWheelDown()
{
    QueueMouseWheelStep(-1);
}

void APinkCabChaosTatraPawn::QueueMouseWheelStep(const int32 Step)
{
    if (Step == 0)
    {
        return;
    }
    PendingMouseWheelSteps = FMath::Clamp(PendingMouseWheelSteps + FMath::Sign(Step), -8, 8);
}

void APinkCabChaosTatraPawn::ApplyMouseSteeringDelta(
    const float DeltaX,
    const bool bGazeHeld,
    const float DeltaSeconds)
{
    VehicleControlRuntime.ApplySteering(DeltaX, bGazeHeld, DeltaSeconds);
}

void APinkCabChaosTatraPawn::ApplyPhysicalControlMouseDelta(
    const FName TargetId,
    const bool bManipulationActive,
    const float MouseDeltaX,
    const float MouseDeltaY,
    const float DeltaSeconds)
{
    VehicleControlRuntime.ApplyPhysicalControl(
        TargetId, bManipulationActive, MouseDeltaX, MouseDeltaY, DeltaSeconds, CockpitState);
    GearLeverCursor = VehicleControlRuntime.GetGearLeverCursor();
    SyncCockpitToChaos();
}

void APinkCabChaosTatraPawn::ApplyVehicleInputFrame(
    const FPinkCabVehicleInputFrame& InputFrame,
    const float MouseDeltaX,
    const float DeltaSeconds)
{
    VehicleControlRuntime.ResolveControlFrame(
        InputFrame, MouseDeltaX, DeltaSeconds, CockpitState, GetMutableVehicleHealthState());
    if (CockpitInteraction)
    {
        CockpitInteraction->SetGazeHeld(InputFrame.bGazeHeld);
    }
    SyncCockpitToChaos();
}

bool APinkCabChaosTatraPawn::BeginDriverFrame(
    APlayerController& PC,
    FPinkCabPlayerInputSample& OutInput)
{
    if (DriverUi)
    {
        DriverUi->EnsurePlayableHudMounted();
    }
    OutInput = FPinkCabPlayerInputAdapter().Capture(PC, InputRouter);
    if (PendingMouseWheelSteps != 0)
    {
        const int32 EventStep = FMath::Clamp(PendingMouseWheelSteps, -1, 1);
        PendingMouseWheelSteps -= EventStep;
        OutInput.WheelSteps = EventStep;
    }
    if (OutInput.bSystemMenuToggleRequested)
    {
        SetSystemMenuOpen(!IsSystemMenuOpen());
    }
    if (!UGameplayStatics::IsGamePaused(this))
    {
        return true;
    }
    ResetTransientCockpitInput();
    return false;
}

FPinkCabVehicleInputFrame APinkCabChaosTatraPawn::PrepareVehicleFrame(
    const FPinkCabPlayerInputSample& PlayerInput,
    const float DeltaSeconds,
    EPinkCabPedalWheelRecipient& OutWheelRecipient)
{
    const FPinkCabVehicleInputFrame RawInputFrame = FPinkCabVehicleInputFrame::FromDigital(
        PlayerInput.bGazeHeld,
        PlayerInput.bClutchHeld,
        PlayerInput.bBrakeHeld,
        PlayerInput.bThrottleHeld);
    TOptional<FPinkCabVehicleTelemetry> MotionTelemetry;
    FPinkCabVehicleTelemetry TelemetrySample;
    if (DynamicsProvider.ReadTelemetry(TelemetrySample))
    {
        MotionTelemetry = TelemetrySample;
    }
    const FPinkCabPreparedVehicleControlFrame PreparedControl =
        VehicleControlRuntime.PrepareInputFrame(
            RawInputFrame,
            PlayerInput.WheelSteps,
            MotionTelemetry,
            CockpitState,
            DeltaSeconds);
    OutWheelRecipient = PreparedControl.WheelRecipient;
    return PreparedControl.Frame;
}

bool APinkCabChaosTatraPawn::ProcessCockpitFrame(
    APlayerController& PC,
    const FPinkCabPlayerInputSample& PlayerInput,
    const EPinkCabPedalWheelRecipient WheelRecipient,
    const float DeltaSeconds)
{
    FPinkCabCockpitInteractionFrame InteractionFrame;
    InteractionFrame.bGazeHeld = PlayerInput.bGazeHeld;
    InteractionFrame.bQuickRecall1Held = PlayerInput.bQuickRecall1Held;
    InteractionFrame.bQuickRecall2Held = PlayerInput.bQuickRecall2Held;
    // Q owns the clutch pedal and stages the driver's right hand over the
    // H-gate through the same non-actuating quick-recall path as key 3.
    // RMB only retains that control; steering stays mouse-owned until LMB
    // starts the explicit lever manipulation.
    const bool bCanStageGearboxFromClutch =
        !CockpitInteraction->IsGripActive()
        && (CockpitInteraction->GetCurrentTargetId().IsNone()
            || CockpitInteraction->IsGearboxStageFromClutchHeld());
    InteractionFrame.bQuickRecall3Held = PlayerInput.bQuickRecall3Held;
    InteractionFrame.bQuickRecall4Held = PlayerInput.bQuickRecall4Held;
    InteractionFrame.bGearboxStageFromClutchHeld =
        PlayerInput.bClutchHeld && bCanStageGearboxFromClutch;
    InteractionFrame.bGripHeld = PlayerInput.bGripHeld;
    InteractionFrame.bMomentaryHeld = PlayerInput.bMomentaryHeld;
    InteractionFrame.WheelSteps =
        WheelRecipient == EPinkCabPedalWheelRecipient::None ? PlayerInput.WheelSteps : 0;
    InteractionFrame.NowSeconds = FPlatformTime::Seconds();
    if (DriverCamera)
    {
        InteractionFrame.GazeOrigin = DriverCamera->GetComponentLocation();
        InteractionFrame.GazeForward = DriverCamera->GetForwardVector();
    }

    TArray<FPinkCabInteractionEvent> InteractionEvents;
    CockpitInteraction->ProcessFrame(InteractionFrame, CockpitAssembly, InteractionEvents);
    for (const FPinkCabInteractionEvent& InteractionEvent : InteractionEvents)
    {
        ApplyCockpitInteraction(InteractionEvent);
    }

    const FName ActiveGripTarget = CockpitInteraction->GetActiveGripTargetId();
    const bool bManipulationActive = CockpitInteraction->IsManipulationActive();
    const bool bGearboxManipulationActive =
        bManipulationActive && ActiveGripTarget == FName(TEXT("Gearbox"));

    // RMB only acquires/retains the selected physical control. Steering stays
    // mouse-owned until LMB starts an authored lever manipulation. During that
    // manipulation mouse XY belongs to exactly one lever and the existing
    // steering angle is held rather than reset.
    if (DriverUi)
    {
        DriverUi->SetPointerCapture(PC, bManipulationActive);
    }
    ApplyPhysicalControlMouseDelta(
        ActiveGripTarget,
        bManipulationActive,
        PlayerInput.DeviceX,
        PlayerInput.DeviceY,
        DeltaSeconds);
    bGearLeverDragging = bGearboxManipulationActive;
    GearLeverCursor = bGearLeverDragging
        ? VehicleControlRuntime.GetGearLeverCursor()
        : UPinkCabCockpitVisualDriverComponent::GearCursorForGear(CockpitState.GetSelectedGear());
    return bManipulationActive;
}

void APinkCabChaosTatraPawn::UpdateDriverLook(
    const FPinkCabPlayerInputSample& PlayerInput,
    const bool bManipulationActive,
    const float DeltaSeconds)
{
    const bool bGazeHeld = CockpitInteraction->IsGazeHeld() && !bManipulationActive;
    SmoothedLookMouseX = FMath::FInterpTo(
        SmoothedLookMouseX, bGazeHeld ? PlayerInput.LookMouseX : 0.0f, DeltaSeconds, 12.0f);
    SmoothedLookMouseY = FMath::FInterpTo(
        SmoothedLookMouseY, bGazeHeld ? PlayerInput.LookMouseY : 0.0f, DeltaSeconds, 12.0f);
    if (bGazeHeld)
    {
        LookYaw = FMath::Clamp(LookYaw + SmoothedLookMouseX * 0.58f, -130.0f, 130.0f);
        LookPitch = FMath::Clamp(LookPitch + SmoothedLookMouseY * 0.46f, -58.0f, 48.0f);
    }
    else
    {
        LookYaw = FMath::FInterpTo(LookYaw, 0.0f, DeltaSeconds, 3.2f);
        LookPitch = FMath::FInterpTo(LookPitch, 0.0f, DeltaSeconds, 3.2f);
    }
    DriverHeadRoot->SetRelativeRotation(FRotator(LookPitch, LookYaw, 0.0f));
}

FPinkCabCockpitPresentationState APinkCabChaosTatraPawn::BuildCockpitPresentation(
    const float DeltaSeconds)
{
    FPinkCabCockpitPresentationState Presentation;
    VisualSteering = VehicleControlRuntime.GetControlState().Steering;
    Presentation.Steering = VisualSteering;
    Presentation.Clutch = VehicleControlRuntime.GetDisplayedClutchPedal();
    Presentation.Brake = VehicleControlRuntime.GetDisplayedBrakePedal();
    Presentation.Throttle = VehicleControlRuntime.GetDisplayedThrottlePedal();
    Presentation.SelectedGear = CockpitState.GetSelectedGear();
    Presentation.bGearLeverDragging = bGearLeverDragging;
    Presentation.GearLeverCursor = GearLeverCursor;
    Presentation.bIgnitionRunning = CockpitState.GetIgnitionState() == EPinkCabIgnitionState::Running;
    Presentation.Handbrake = CockpitState.GetHandbrakeAmount();
    Presentation.bHandbrakeEngaged = CockpitState.IsHandbrakeEngaged();

    FPinkCabCockpitServiceSources ServiceSources;
    ServiceSources.Taximeter = CockpitTaximeterSource;
    ServiceSources.CockpitState = &CockpitState;
    ServiceSources.RouteProgress01 = CockpitRouteProgress01;
    ServiceSources.bRadioAvailable = bCockpitRadioAvailable;
    ServiceSources.bMirrorsAvailable = bCockpitMirrorsAvailable;
    FPinkCabCockpitServiceBridge::ApplyToPresentation(
        FPinkCabCockpitServiceBridge::Read(ServiceSources), Presentation);
    Presentation.bTurnSignalLeft = CockpitState.GetTurnSignalDirection() < 0;
    Presentation.bTurnSignalRight = CockpitState.GetTurnSignalDirection() > 0;
    Presentation.bHornActive = CockpitState.IsHornActive();
    Presentation.bLightsOn = CockpitState.GetLightMode() > 0;
    Presentation.bWipersOn = CockpitState.GetWiperMode() > 0;
    Presentation.bWasherActive = CockpitState.IsWasherActive();

    FPinkCabVehicleTelemetry Telemetry;
    if (DynamicsProvider.ReadTelemetry(Telemetry))
    {
        Presentation.SpeedKmh = Telemetry.SpeedKmh;
    }
    Presentation.EngineRpm = VehicleControlRuntime.GetDisplayedEngineRpm();
    const float Rpm01 = FMath::Clamp(Presentation.EngineRpm / 8500.0f, 0.0f, 1.0f);
    const float TemperatureTarget = Presentation.bIgnitionRunning
        ? FMath::Lerp(0.62f, 0.90f, Rpm01)
        : 0.12f;
    EngineTemperature01 = FMath::FInterpTo(
        EngineTemperature01,
        TemperatureTarget,
        DeltaSeconds,
        Presentation.bIgnitionRunning ? 0.08f : 0.03f);
    Presentation.EngineTemperature01 = EngineTemperature01;
    Presentation.Fuel01 = TatraProfile.FullFuelMassKg > KINDA_SMALL_NUMBER
        ? FMath::Clamp(VehicleLoadState.GetFuelMassKg() / TatraProfile.FullFuelMassKg, 0.0f, 1.0f)
        : 0.0f;
    return Presentation;
}

void APinkCabChaosTatraPawn::UpdateDriverUiState(
    const FPinkCabCockpitPresentationState& Presentation)
{
    if (!DriverUi)
    {
        return;
    }
    FPinkCabDriverUiState UiState;
    UiState.CurrentTargetId = CockpitInteraction ? CockpitInteraction->GetCurrentTargetId() : NAME_None;
    UiState.GearLeverCursor = GearLeverCursor;
    UiState.SpeedKmh = Presentation.SpeedKmh;
    UiState.EngineRpm = Presentation.EngineRpm;
    UiState.Clutch = Presentation.Clutch;
    UiState.Brake = Presentation.Brake;
    UiState.Throttle = Presentation.Throttle;
    UiState.Handbrake = VehicleControlRuntime.GetHandbrakeCommand();
    UiState.Steering = Presentation.Steering;
    UiState.Fuel01 = Presentation.Fuel01;
    UiState.EngineTemperature01 = Presentation.EngineTemperature01;
    UiState.BrakeTemperature01 = GetVehicleHealthState().GetBrakeTemperature01();
    UiState.ClutchTemperature01 = GetVehicleHealthState().GetClutchTemperature01();
    UiState.RequestedGear = VehicleControlRuntime.GetRequestedGear();
    UiState.EngagedGear = VehicleControlRuntime.GetEngagedGear();
    UiState.bGearLeverDragging = bGearLeverDragging;
    UiState.bEngineRunning = CockpitState.GetIgnitionState() == EPinkCabIgnitionState::Running;
    UiState.bEngineStalled = CockpitState.GetIgnitionState() == EPinkCabIgnitionState::Stalled;
    UiState.bMoving = GetMotionMode() == EPinkCabVehicleMotionMode::Moving;
    UiState.bRequiresThrottleDose = VehicleControlRuntime.RequiresThrottleDose();
    UiState.bParkingHandbrakeLatched = VehicleControlRuntime.IsParkingHandbrakeLatched();
    UiState.GearResult = VehicleControlRuntime.GetLastGearResult();
    DriverUi->UpdateState(UiState);
}
