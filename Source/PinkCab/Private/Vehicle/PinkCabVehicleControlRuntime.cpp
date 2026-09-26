#include "Vehicle/PinkCabVehicleControlRuntime.h"

#include "Interaction/PinkCabPhysicalInputConvention.h"
#include "Vehicle/PinkCabCockpitState.h"
#include "Vehicle/PinkCabVehicleHealthState.h"
#include "Vehicle/PinkCabVehicleHealthService.h"
#include "Vehicle/PinkCabVehicleInputResponse.h"

FPinkCabVehicleControlRuntime::FPinkCabVehicleControlRuntime(
    const FPinkCabVehicleControlRuntimeConfig& InConfig)
    : Config(InConfig)
{
    ControlState.SetHandbrake(HandbrakeActuator.GetBrakeCommand());
    ControlState.SetDriveline(0, 0, 0.0f);
}

void FPinkCabVehicleControlRuntime::SetResponseConfig(
    const FPinkCabVehicleControlRuntimeConfig& InConfig)
{
    Config = InConfig;
}

void FPinkCabVehicleControlRuntime::UpdateMotion(
    const TOptional<FPinkCabVehicleTelemetry>& Telemetry,
    const float DeltaSeconds)
{
    if (!Telemetry.IsSet())
    {
        return;
    }
    LastSpeedKmh = Telemetry->SpeedKmh;
    LastEngineRpm = Telemetry->EngineRpm;
    MotionClassifier.Update(LastSpeedKmh, DeltaSeconds);
}

void FPinkCabVehicleControlRuntime::UpdateLaunchEdge(
    const bool bThrottleHeld,
    const EPinkCabVehicleMotionMode PreviousMotionMode)
{
    const EPinkCabVehicleMotionMode MotionMode = MotionClassifier.GetMode();
    LaunchController.NotifyMotionMode(MotionMode);
    const bool bReturnedToStationary =
        PreviousMotionMode == EPinkCabVehicleMotionMode::Moving
        && MotionMode == EPinkCabVehicleMotionMode::Stationary;
    if (MotionMode == EPinkCabVehicleMotionMode::Stationary
        && ((bThrottleHeld && !bThrottleHeldLastFrame)
            || (bReturnedToStationary && bThrottleHeld))
        && LaunchController.BeginLaunchAttempt())
    {
        SmoothedThrottle = 0.0f;
    }
}

FPinkCabVehicleInputFrame FPinkCabVehicleControlRuntime::ResolvePedalTargets(
    const FPinkCabVehicleInputFrame& InputFrame,
    const int32 WheelSteps,
    FPinkCabCockpitState& Cockpit,
    const float DeltaSeconds)
{
    const bool bClutchHeld = InputFrame.Clutch > 0.5f;
    const bool bBrakeHeld = InputFrame.Brake > 0.5f;
    const bool bThrottleHeld = InputFrame.Throttle > 0.5f;
    LastWheelRecipient = PedalDosingController.ApplyWheelSteps(
        bClutchHeld, bBrakeHeld, bThrottleHeld, WheelSteps, LaunchController, Cockpit);
    const FPinkCabPedalTargets Targets = PedalDosingController.ResolveTargets(
        bBrakeHeld, bThrottleHeld, LaunchController.GetThrottleTarget());

    FPinkCabVehicleInputFrame Result = InputFrame;
    SmoothedClutch = FPinkCabVehicleInputResponse::StepAxis(
        SmoothedClutch, InputFrame.Clutch, DeltaSeconds,
        Config.ClutchPressSeconds, Cockpit.GetClutchReleaseSeconds());
    SmoothedBrake = FPinkCabVehicleInputResponse::StepAxis(
        SmoothedBrake, Targets.Brake, DeltaSeconds,
        Config.BrakePressSeconds, Config.BrakeReleaseSeconds);
    SmoothedThrottle = FPinkCabVehicleInputResponse::StepAxis(
        SmoothedThrottle, Targets.Throttle, DeltaSeconds,
        Config.ThrottlePressSeconds, Config.ThrottleReleaseSeconds);
    Result.Clutch = SmoothedClutch;
    Result.Brake = SmoothedBrake;
    Result.Throttle = SmoothedThrottle;
    return Result;
}

FPinkCabPreparedVehicleControlFrame FPinkCabVehicleControlRuntime::PrepareInputFrame(
    const FPinkCabVehicleInputFrame& InputFrame,
    const int32 WheelSteps,
    const TOptional<FPinkCabVehicleTelemetry>& Telemetry,
    FPinkCabCockpitState& Cockpit,
    const float DeltaSeconds)
{
    CausalControlTelemetry.RawThrottle01 = FMath::Clamp(InputFrame.Throttle, 0.0f, 1.0f);
    CausalControlTelemetry.RawBrake01 = FMath::Clamp(InputFrame.Brake, 0.0f, 1.0f);
    CausalControlTelemetry.RawClutch01 = FMath::Clamp(InputFrame.Clutch, 0.0f, 1.0f);

    const EPinkCabVehicleMotionMode PreviousMotionMode = MotionClassifier.GetMode();
    UpdateMotion(Telemetry, DeltaSeconds);
    const bool bThrottleHeld = InputFrame.Throttle > 0.5f;
    UpdateLaunchEdge(bThrottleHeld, PreviousMotionMode);

    FPinkCabPreparedVehicleControlFrame Result;
    Result.Frame = ResolvePedalTargets(InputFrame, WheelSteps, Cockpit, DeltaSeconds);
    Result.WheelRecipient = LastWheelRecipient;
    CausalControlTelemetry.PreparedThrottle01 = FMath::Clamp(Result.Frame.Throttle, 0.0f, 1.0f);
    CausalControlTelemetry.PreparedBrake01 = FMath::Clamp(Result.Frame.Brake, 0.0f, 1.0f);
    CausalControlTelemetry.PreparedClutch01 = FMath::Clamp(Result.Frame.Clutch, 0.0f, 1.0f);
    bCausalPreparedFramePending = true;
    bThrottleHeldLastFrame = bThrottleHeld;
    return Result;
}

FPinkCabGearEngagementContext FPinkCabVehicleControlRuntime::BuildGearContext(
    const FPinkCabVehicleInputFrame& InputFrame,
    const FPinkCabVehicleHealthState& Health) const
{
    FPinkCabGearEngagementContext Context;
    Context.ClutchPedal = InputFrame.Clutch;
    Context.EngineRpm = LastEngineRpm;
    Context.SpeedKmh = LastSpeedKmh;
    Context.Throttle = InputFrame.Throttle;
    Context.Brake = InputFrame.Brake;
    Context.GearboxHealth = Health.GetHealth(EPinkCabVehicleHealthChannel::Gearbox);
    return Context;
}

void FPinkCabVehicleControlRuntime::EvaluateGearbox(
    const FPinkCabVehicleInputFrame& EffectiveInput,
    FPinkCabCockpitState& Cockpit,
    FPinkCabVehicleHealthState& Health)
{
    const FPinkCabGearEngagementContext Context = BuildGearContext(EffectiveInput, Health);
    if (Cockpit.GetSelectedGear() != GearboxController.GetRequestedGear())
    {
        GearboxController.RequestGear(Cockpit.GetSelectedGear(), Context);
    }
    else
    {
        GearboxController.EvaluateCurrentEngagement(Context);
    }
    if (GearboxController.GetEventSerial() != LastProcessedGearEventSerial)
    {
        DrivetrainCondition.RecordGearEvent(GearboxController.GetLastResult(), Health);
        LastProcessedGearEventSerial = GearboxController.GetEventSerial();
    }
}

FPinkCabDrivetrainConditionOutput FPinkCabVehicleControlRuntime::ApplyDrivetrainCondition(
    FPinkCabVehicleInputFrame& EffectiveInput,
    const float DeltaSeconds,
    FPinkCabCockpitState& Cockpit,
    FPinkCabVehicleHealthState& Health)
{
    FPinkCabDrivetrainConditionInput ConditionInput;
    ConditionInput.DeltaSeconds = DeltaSeconds;
    ConditionInput.bEngineRunning = Cockpit.GetIgnitionState() == EPinkCabIgnitionState::Running;
    ConditionInput.EngineRpm = LastEngineRpm;
    ConditionInput.ExpectedCoupledRpm = GearboxController.GetExpectedCoupledRpm();
    ConditionInput.SpeedKmh = LastSpeedKmh;
    ConditionInput.Throttle = EffectiveInput.Throttle;
    ConditionInput.Brake = EffectiveInput.Brake;
    ConditionInput.Handbrake = HandbrakeActuator.GetBrakeCommand();
    ConditionInput.ClutchCoupling = GearboxController.ComputeClutchCoupling(EffectiveInput.Clutch);
    ConditionInput.EngagedGear = GearboxController.GetEngagedGear();
    const FPinkCabDrivetrainConditionOutput Output = DrivetrainCondition.Step(ConditionInput, Health);
    CausalControlTelemetry.ExpectedCoupledRpm = ConditionInput.ExpectedCoupledRpm;
    CausalControlTelemetry.EngineTorqueFactor = Output.EngineTorqueFactor;
    CausalControlTelemetry.BrakeEffectiveness = Output.BrakeEffectiveness;
    CausalControlTelemetry.DrivetrainTorqueCapacity01 = Output.DrivetrainTorqueCapacity;
    EffectiveInput.Throttle *= Output.EngineTorqueFactor;
    EffectiveInput.Brake *= Output.BrakeEffectiveness;
    DisplayedEngineRpm = Output.DisplayedEngineRpm;
    DrivetrainTorqueCapacity = Output.DrivetrainTorqueCapacity;
    if (Output.bShouldStall)
    {
        Cockpit.StallEngine();
        DisplayedEngineRpm = 0.0f;
    }
    return Output;
}

void FPinkCabVehicleControlRuntime::ApplySteering(
    const float DriverMouseX,
    const bool bGazeHeld,
    const float DeltaSeconds)
{
    CausalControlTelemetry.RawSteeringMouseDelta = DriverMouseX;
    const float Steering = SteeringController.Step(
        DriverMouseX, bGazeHeld, LastSpeedKmh, MotionClassifier.GetMode(), DeltaSeconds);
    CausalControlTelemetry.SteeringVirtualCursor = SteeringController.GetVirtualCursor();
    CausalControlTelemetry.SteeringTarget = SteeringController.GetTarget();
    CausalControlTelemetry.FinalSteeringCommand = Steering;
    ControlState.SetSteering(Steering);
}

const FPinkCabVehicleControlState& FPinkCabVehicleControlRuntime::ResolveControlFrame(
    const FPinkCabVehicleInputFrame& InputFrame,
    const float SteeringMouseX,
    const float DeltaSeconds,
    FPinkCabCockpitState& Cockpit,
    FPinkCabVehicleHealthState& Health)
{
    if (!bCausalPreparedFramePending)
    {
        CausalControlTelemetry.RawThrottle01 = FMath::Clamp(InputFrame.Throttle, 0.0f, 1.0f);
        CausalControlTelemetry.RawBrake01 = FMath::Clamp(InputFrame.Brake, 0.0f, 1.0f);
        CausalControlTelemetry.RawClutch01 = FMath::Clamp(InputFrame.Clutch, 0.0f, 1.0f);
        CausalControlTelemetry.PreparedThrottle01 = CausalControlTelemetry.RawThrottle01;
        CausalControlTelemetry.PreparedBrake01 = CausalControlTelemetry.RawBrake01;
        CausalControlTelemetry.PreparedClutch01 = CausalControlTelemetry.RawClutch01;
    }
    bCausalPreparedFramePending = false;

    FPinkCabVehicleInputFrame EffectiveInput = InputFrame;
    DisplayedClutchPedal = FMath::Clamp(InputFrame.Clutch, 0.0f, 1.0f);
    DisplayedBrakePedal = FMath::Clamp(InputFrame.Brake, 0.0f, 1.0f);
    DisplayedThrottlePedal = FMath::Clamp(InputFrame.Throttle, 0.0f, 1.0f);

    EvaluateGearbox(EffectiveInput, Cockpit, Health);
    const FPinkCabDrivetrainConditionOutput Condition =
        ApplyDrivetrainCondition(EffectiveInput, DeltaSeconds, Cockpit, Health);
    CausalControlTelemetry.PostDrivetrainThrottle01 =
        FMath::Clamp(EffectiveInput.Throttle, 0.0f, 1.0f);
    const float EffectiveHandbrake = HandbrakeActuator.GetBrakeCommand()
        * (MotionClassifier.GetMode() == EPinkCabVehicleMotionMode::Moving
            ? Condition.HandbrakeEffectiveness : 1.0f);

    ApplySteering(
        SteeringMouseX,
        EffectiveInput.bGazeHeld || EffectiveInput.bSteeringHeld,
        DeltaSeconds);
    ControlState = EffectiveInput.ToControlState(GetSteeringCommand(), EffectiveHandbrake);
    ControlState.SetDriveline(
        GearboxController.GetRequestedGear(),
        GearboxController.GetEngagedGear(),
        GearboxController.ComputeClutchCoupling(EffectiveInput.Clutch));
    ControlState.SetDrivetrainTorqueCapacity(DrivetrainTorqueCapacity);
    return ControlState;
}

FPinkCabVehicleControlOutput FPinkCabVehicleControlRuntime::Update(
    const FPinkCabVehicleControlTickInput& Input,
    const FPinkCabVehicleTelemetry& Telemetry,
    FPinkCabCockpitState& Cockpit,
    FPinkCabVehicleHealthState& Health)
{
    const FPinkCabPreparedVehicleControlFrame Prepared = PrepareInputFrame(
        Input.Frame, Input.WheelSteps, Telemetry, Cockpit, Input.DeltaSeconds);
    ResolveControlFrame(Prepared.Frame, Input.SteeringMouseX, Input.DeltaSeconds, Cockpit, Health);

    FPinkCabVehicleControlOutput Output;
    Output.Controls = ControlState;
    Output.GearLeverCursor = GetGearLeverCursor();
    Output.RequestedGear = GetRequestedGear();
    Output.EngagedGear = GetEngagedGear();
    return Output;
}

void FPinkCabVehicleControlRuntime::ApplyPhysicalControl(
    const FName TargetId,
    const bool bManipulationActive,
    const float DeviceX,
    const float DeviceY,
    const float DeltaSeconds,
    FPinkCabCockpitState& Cockpit)
{
    if (TargetId == FName(TEXT("Gearbox")) && bManipulationActive)
    {
        GearboxController.ApplyLeverDriverDelta(
            FPinkCabPhysicalInputConvention::SteeringRight(DeviceX),
            FPinkCabPhysicalInputConvention::GearboxForward(DeviceY));
        Cockpit.SetSelectedGear(GearboxController.GetRequestedGear());
    }
    const bool bHandbrakeManipulation =
        TargetId == FName(TEXT("Handbrake")) && bManipulationActive;
    HandbrakeActuator.Step(
        MotionClassifier.GetMode(),
        bHandbrakeManipulation,
        bHandbrakeManipulation ? FPinkCabPhysicalInputConvention::HandbrakePull(DeviceY) : 0.0f,
        DeltaSeconds);
    Cockpit.SetHandbrakeAmount(HandbrakeActuator.GetLeverPosition());
    ControlState.SetHandbrake(HandbrakeActuator.GetBrakeCommand());
}

void FPinkCabVehicleControlRuntime::ApplyHealthCapabilities(
    const FPinkCabVehicleHealthService& HealthService,
    const FPinkCabVehicleHealthState& Health)
{
    HealthService.ApplyCapabilitiesToControls(Health, ControlState);
}

bool FPinkCabVehicleControlRuntime::ResetTransient(FPinkCabCockpitState& Cockpit)
{
    bThrottleHeldLastFrame = false;
    GearboxController.CancelPendingRequest();
    Cockpit.SetSelectedGear(GearboxController.GetEngagedGear());
    if (MotionClassifier.GetMode() != EPinkCabVehicleMotionMode::Moving)
    {
        return false;
    }
    ResetHandbrake(0.0f, false, Cockpit);
    return true;
}

void FPinkCabVehicleControlRuntime::ResetHandbrake(
    const float LeverPosition,
    const bool bLatched,
    FPinkCabCockpitState& Cockpit)
{
    HandbrakeActuator.Reset(LeverPosition, bLatched);
    Cockpit.SetHandbrakeAmount(HandbrakeActuator.GetLeverPosition());
    ControlState.SetHandbrake(HandbrakeActuator.GetBrakeCommand());
}

void FPinkCabVehicleControlRuntime::ForceGearState(
    const int32 RequestedGear,
    const int32 EngagedGear,
    FPinkCabCockpitState& Cockpit)
{
    GearboxController.ForceState(RequestedGear, EngagedGear);
    Cockpit.SetSelectedGear(GearboxController.GetRequestedGear());
    ControlState.SetDriveline(
        GearboxController.GetRequestedGear(), GearboxController.GetEngagedGear(), ControlState.ClutchCoupling);
}

FVector2D FPinkCabVehicleControlRuntime::GetGearLeverCursor() const
{
    return FVector2D(GearboxController.GetLeverX(), GearboxController.GetLeverY());
}
