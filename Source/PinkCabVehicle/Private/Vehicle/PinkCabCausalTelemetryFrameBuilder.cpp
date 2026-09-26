#include "Vehicle/PinkCabCausalTelemetryFrameBuilder.h"

FPinkCabCausalTelemetryFrame FPinkCabCausalTelemetryFrameBuilder::Build(
    const FPinkCabCausalTelemetryBuildInput& Input)
{
    FPinkCabCausalTelemetryFrame Frame;
    Frame.TimestampSeconds = Input.TimestampSeconds;
    Frame.DeltaSeconds = Input.DeltaSeconds;
    Frame.IgnitionState = Input.IgnitionState;
    Frame.bCombustionPermission =
        Input.IgnitionState == EPinkCabCausalIgnitionState::Running;

    Frame.ModelId = Input.ModelId;
    Frame.ProfileId = Input.ProfileId;
    Frame.ProfileSchemaVersion = Input.ProfileSchemaVersion;
    Frame.CalibrationVersion = Input.CalibrationVersion;
    Frame.ProfileHash = Input.ProfileHash;

    Frame.Control = Input.Control;
    Frame.Actuation = Input.Vehicle.CausalActuation;

    Frame.EngineRpm = Input.Vehicle.EngineRpm;
    Frame.ExpectedCoupledRpm = Input.Control.ExpectedCoupledRpm;
    Frame.ClutchCoupling01 = Input.Controls.ClutchCoupling;
    Frame.DrivetrainTorqueCapacity01 =
        Input.Controls.DrivetrainTorqueCapacity;
    Frame.EngineHealthFactor01 =
        FMath::Clamp(Input.EngineHealthFactor01, 0.0f, 1.0f);
    Frame.RequestedEngineTorqueAfterLimiterHealthNm =
        Input.Vehicle.CausalActuation.RequestedEngineTorqueAfterLimiterHealthNm;

    Frame.RequestedGear = Input.Controls.RequestedGear;
    Frame.EngagedGear = Input.Controls.EngagedGear;
    Frame.ChaosCurrentGear = Input.Vehicle.CurrentGear;
    Frame.ChaosTargetGear = Input.Vehicle.TargetGear;

    Frame.ForwardSpeedKmh = Input.Vehicle.SpeedKmh;
    Frame.WorldSpeedMps = FMath::Max(Input.WorldSpeedMps, 0.0f);
    Frame.VehicleMassKg = FMath::Max(Input.VehicleMassKg, 0.0f);
    Frame.TranslationalKineticEnergyJ =
        0.5
        * static_cast<double>(Frame.VehicleMassKg)
        * static_cast<double>(Frame.WorldSpeedMps)
        * static_cast<double>(Frame.WorldSpeedMps);

    Frame.Wheels = Input.Vehicle.CausalWheels;
    Frame.RefreshDerivedFields();
    return Frame;
}
