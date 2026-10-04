#include "Runtime/PinkCabChaosTatraPawn.h"
#include "Runtime/PinkCabDriverUiComponent.h"
#include "Cockpit/PinkCabCockpitInteractionComponent.h"
#include "Cockpit/PinkCabCockpitPresentationState.h"
#include "Cockpit/PinkCabCockpitServiceBridge.h"

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
    if (CockpitInteraction)
    {
        const FName QuickTarget = CockpitInteraction->GetCurrentQuickTargetId();
        const bool bShowHeldTarget =
            CockpitInteraction->IsGripActive()
            || CockpitInteraction->IsManipulationActive()
            || CockpitInteraction->IsMomentaryActive()
            || CockpitInteraction->IsGazeHeld();
        UiState.CurrentTargetId = !QuickTarget.IsNone()
            ? QuickTarget
            : (bShowHeldTarget ? CockpitInteraction->GetCurrentTargetId() : NAME_None);
    }
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
