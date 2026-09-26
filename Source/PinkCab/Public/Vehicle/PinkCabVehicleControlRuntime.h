#pragma once

#include "CoreMinimal.h"
#include "Vehicle/PinkCabCausalTelemetry.h"
#include "Vehicle/PinkCabDrivetrainCondition.h"
#include "Vehicle/PinkCabGearboxController.h"
#include "Vehicle/PinkCabHandbrakeActuator.h"
#include "Vehicle/PinkCabLaunchController.h"
#include "Vehicle/PinkCabPedalDosingController.h"
#include "Vehicle/PinkCabSteeringController.h"
#include "Vehicle/PinkCabVehicleControlState.h"
#include "Vehicle/PinkCabVehicleInputFrame.h"
#include "Vehicle/PinkCabVehicleMotionClassifier.h"
#include "Vehicle/PinkCabVehicleTelemetry.h"

struct FPinkCabCockpitState;
struct FPinkCabVehicleHealthState;
struct FPinkCabVehicleHealthService;

struct FPinkCabVehicleControlRuntimeConfig
{
    float ClutchPressSeconds = 0.16f;
    float BrakePressSeconds = 0.20f;
    float BrakeReleaseSeconds = 0.28f;
    float ThrottlePressSeconds = 0.35f;
    float ThrottleReleaseSeconds = 0.22f;
};

struct FPinkCabVehicleControlTickInput
{
    FPinkCabVehicleInputFrame Frame;
    int32 WheelSteps = 0;
    float SteeringMouseX = 0.0f;
    float DeltaSeconds = 0.0f;
};

struct FPinkCabPreparedVehicleControlFrame
{
    FPinkCabVehicleInputFrame Frame;
    EPinkCabPedalWheelRecipient WheelRecipient = EPinkCabPedalWheelRecipient::None;
};

struct FPinkCabVehicleControlOutput
{
    FPinkCabVehicleControlState Controls;
    FVector2D GearLeverCursor = FVector2D::ZeroVector;
    int32 RequestedGear = 0;
    int32 EngagedGear = 0;
};

class PINKCAB_API FPinkCabVehicleControlRuntime
{
public:
    explicit FPinkCabVehicleControlRuntime(
        const FPinkCabVehicleControlRuntimeConfig& InConfig = {});

    void SetResponseConfig(const FPinkCabVehicleControlRuntimeConfig& InConfig);

    FPinkCabPreparedVehicleControlFrame PrepareInputFrame(
        const FPinkCabVehicleInputFrame& InputFrame,
        int32 WheelSteps,
        const TOptional<FPinkCabVehicleTelemetry>& Telemetry,
        FPinkCabCockpitState& Cockpit,
        float DeltaSeconds);

    const FPinkCabVehicleControlState& ResolveControlFrame(
        const FPinkCabVehicleInputFrame& InputFrame,
        float SteeringMouseX,
        float DeltaSeconds,
        FPinkCabCockpitState& Cockpit,
        FPinkCabVehicleHealthState& Health);

    FPinkCabVehicleControlOutput Update(
        const FPinkCabVehicleControlTickInput& Input,
        const FPinkCabVehicleTelemetry& Telemetry,
        FPinkCabCockpitState& Cockpit,
        FPinkCabVehicleHealthState& Health);

    void ApplyPhysicalControl(
        FName TargetId,
        bool bManipulationActive,
        float DeviceX,
        float DeviceY,
        float DeltaSeconds,
        FPinkCabCockpitState& Cockpit);

    void ApplySteering(float DriverMouseX, bool bGazeHeld, float DeltaSeconds);
    void ResetThrottleInputLatch() { bThrottleHeldLastFrame = false; }
    void ResetEngineTransition();
    bool ResetTransient(FPinkCabCockpitState& Cockpit);
    void ApplyHealthCapabilities(
        const FPinkCabVehicleHealthService& HealthService,
        const FPinkCabVehicleHealthState& Health);
    void ResetHandbrake(float LeverPosition, bool bLatched, FPinkCabCockpitState& Cockpit);
    void ForceGearState(int32 RequestedGear, int32 EngagedGear, FPinkCabCockpitState& Cockpit);

    const FPinkCabVehicleControlState& GetControlState() const { return ControlState; }
    FPinkCabVehicleControlState& GetMutableControlState() { return ControlState; }
    EPinkCabVehicleMotionMode GetMotionMode() const { return MotionClassifier.GetMode(); }
    float GetSteeringCommand() const { return SteeringController.GetSteering(); }
    int32 GetRequestedGear() const { return GearboxController.GetRequestedGear(); }
    int32 GetEngagedGear() const { return GearboxController.GetEngagedGear(); }
    FVector2D GetGearLeverCursor() const;
    EPinkCabGearEngagementResult GetLastGearResult() const { return GearboxController.GetLastResult(); }
    uint32 GetLaunchSerial() const { return LaunchController.GetLaunchSerial(); }
    bool RequiresThrottleDose() const { return LaunchController.RequiresThrottleDose(); }
    float GetThrottleTarget() const { return LaunchController.GetThrottleTarget(); }
    float GetBrakeTarget() const { return PedalDosingController.GetBrakeTarget(); }
    EPinkCabPedalWheelRecipient GetLastWheelRecipient() const { return LastWheelRecipient; }
    float GetHandbrakeCommand() const { return HandbrakeActuator.GetBrakeCommand(); }
    float GetHandbrakeLeverPosition() const { return HandbrakeActuator.GetLeverPosition(); }
    bool IsParkingHandbrakeLatched() const { return HandbrakeActuator.IsParkingLatched(); }
    float GetDisplayedClutchPedal() const { return DisplayedClutchPedal; }
    float GetDisplayedBrakePedal() const { return DisplayedBrakePedal; }
    float GetDisplayedThrottlePedal() const { return DisplayedThrottlePedal; }
    float GetDisplayedEngineRpm() const { return DisplayedEngineRpm; }
    float GetLastSpeedKmh() const { return LastSpeedKmh; }
    float GetLastEngineRpm() const { return LastEngineRpm; }
    float GetDrivetrainTorqueCapacity() const { return DrivetrainTorqueCapacity; }
    const FPinkCabCausalControlTelemetry& GetCausalControlTelemetry() const
    {
        return CausalControlTelemetry;
    }

private:
    void UpdateMotion(const TOptional<FPinkCabVehicleTelemetry>& Telemetry, float DeltaSeconds);
    void UpdateLaunchEdge(bool bThrottleHeld, EPinkCabVehicleMotionMode PreviousMotionMode);
    FPinkCabVehicleInputFrame ResolvePedalTargets(
        const FPinkCabVehicleInputFrame& InputFrame,
        int32 WheelSteps,
        FPinkCabCockpitState& Cockpit,
        float DeltaSeconds);
    FPinkCabGearEngagementContext BuildGearContext(
        const FPinkCabVehicleInputFrame& InputFrame,
        const FPinkCabVehicleHealthState& Health) const;
    FPinkCabDrivetrainConditionOutput ApplyDrivetrainCondition(
        FPinkCabVehicleInputFrame& EffectiveInput,
        float DeltaSeconds,
        FPinkCabCockpitState& Cockpit,
        FPinkCabVehicleHealthState& Health);
    void EvaluateGearbox(
        const FPinkCabVehicleInputFrame& EffectiveInput,
        FPinkCabCockpitState& Cockpit,
        FPinkCabVehicleHealthState& Health);

    FPinkCabVehicleControlRuntimeConfig Config;
    FPinkCabVehicleControlState ControlState;
    FPinkCabVehicleMotionClassifier MotionClassifier;
    FPinkCabSteeringController SteeringController;
    FPinkCabLaunchController LaunchController;
    FPinkCabPedalDosingController PedalDosingController;
    FPinkCabHandbrakeActuator HandbrakeActuator;
    FPinkCabGearboxController GearboxController;
    FPinkCabDrivetrainCondition DrivetrainCondition;
    FPinkCabCausalControlTelemetry CausalControlTelemetry;
    EPinkCabPedalWheelRecipient LastWheelRecipient = EPinkCabPedalWheelRecipient::None;
    float SmoothedClutch = 0.0f;
    float SmoothedBrake = 0.0f;
    float SmoothedThrottle = 0.0f;
    float DisplayedClutchPedal = 0.0f;
    float DisplayedBrakePedal = 0.0f;
    float DisplayedThrottlePedal = 0.0f;
    float LastSpeedKmh = 0.0f;
    float LastEngineRpm = 0.0f;
    float DisplayedEngineRpm = 0.0f;
    float DrivetrainTorqueCapacity = 1.0f;
    uint32 LastProcessedGearEventSerial = 0;
    bool bThrottleHeldLastFrame = false;
    bool bCausalPreparedFramePending = false;
};
