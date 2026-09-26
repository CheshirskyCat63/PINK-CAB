#include "Vehicle/PinkCabCausalTelemetry.h"

namespace
{
const TCHAR* IgnitionToken(const EPinkCabCausalIgnitionState State)
{
    switch (State)
    {
    case EPinkCabCausalIgnitionState::Running: return TEXT("Running");
    case EPinkCabCausalIgnitionState::Stalled: return TEXT("Stalled");
    default: return TEXT("Off");
    }
}

const TCHAR* DrivePathToken(const EPinkCabCausalDriveTorquePath Path)
{
    switch (Path)
    {
    case EPinkCabCausalDriveTorquePath::ExternalPartialClutch: return TEXT("ExternalPartialClutch");
    case EPinkCabCausalDriveTorquePath::ChaosMechanical: return TEXT("ChaosMechanical");
    default: return TEXT("None");
    }
}
}

void FPinkCabCausalTelemetryFrame::RefreshDerivedFields()
{
    PermissionGatedAvailableEngineTorqueNm = bCombustionPermission
        ? FMath::Max(RequestedEngineTorqueAfterLimiterHealthNm, 0.0f)
        : 0.0f;
    ClutchSlipRpm = FMath::Abs(EngineRpm - ExpectedCoupledRpm);
}

FPinkCabCausalTelemetryTrace::FPinkCabCausalTelemetryTrace(const int32 InCapacity)
    : Capacity(FMath::Max(InCapacity, 1))
{
    Frames.Reserve(Capacity);
}

void FPinkCabCausalTelemetryTrace::Reset()
{
    Frames.Reset();
    Frames.Reserve(Capacity);
    DroppedFrameCount = 0;
    NextSequence = 0;
}

bool FPinkCabCausalTelemetryTrace::Record(FPinkCabCausalTelemetryFrame Frame)
{
    if (Frames.Num() >= Capacity)
    {
        ++DroppedFrameCount;
        return false;
    }

    Frame.Sequence = NextSequence++;
    Frame.RefreshDerivedFields();
    Frames.Add(MoveTemp(Frame));
    return true;
}

FString FPinkCabCausalTelemetryTrace::ToCsv() const
{
    FString Csv = TEXT(
        "sequence,timestamp_s,dt_s,ignition,combustion_permission,model_id,profile_id,"
        "profile_schema,calibration,profile_hash,raw_throttle,prepared_throttle,"
        "post_drivetrain_throttle,health_clamped_throttle,engine_throttle_pre_limiter,"
        "engine_throttle_final,engine_rpm,expected_coupled_rpm,clutch_coupling,clutch_slip_rpm,"
        "drivetrain_capacity,engine_health,requested_engine_torque_nm,permission_gated_engine_torque_nm,"
        "requested_gear,engaged_gear,chaos_current_gear,chaos_target_gear,effective_gear_ratio,"
        "final_drive,external_rear_torque_per_wheel_nm,drive_path,forward_speed_kmh,world_speed_mps,"
        "vehicle_mass_kg,translational_ke_j,raw_steering_mouse,steering_cursor,steering_target,"
        "final_steering,torque_control,target_rotation_control,stabilize_control,abs_configured,"
        "tc_configured,assist_contribution,wheel_count\n");

    for (const FPinkCabCausalTelemetryFrame& F : Frames)
    {
        Csv += FString::Printf(
            TEXT("%llu,%.9f,%.6f,%s,%d,%s,%s,%d,%d,%016llX,")
            TEXT("%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.3f,%.3f,%.6f,%.3f,")
            TEXT("%.6f,%.6f,%.3f,%.3f,%d,%d,%d,%d,%.6f,%.6f,%.3f,%s,")
            TEXT("%.6f,%.6f,%.3f,%.3f,%.6f,%.6f,%.6f,%.6f,%d,%d,%d,%d,%d,%.6f,%d\n"),
            static_cast<unsigned long long>(F.Sequence),
            F.TimestampSeconds,
            F.DeltaSeconds,
            IgnitionToken(F.IgnitionState),
            F.bCombustionPermission ? 1 : 0,
            *F.ModelId.ToString(),
            *F.ProfileId.ToString(),
            F.ProfileSchemaVersion,
            F.CalibrationVersion,
            static_cast<unsigned long long>(F.ProfileHash),
            F.Control.RawThrottle01,
            F.Control.PreparedThrottle01,
            F.Control.PostDrivetrainThrottle01,
            F.Actuation.HealthClampedControlThrottle01,
            F.Actuation.EngineThrottlePreLimiter01,
            F.Actuation.EngineThrottleFinal01,
            F.EngineRpm,
            F.ExpectedCoupledRpm,
            F.ClutchCoupling01,
            F.ClutchSlipRpm,
            F.DrivetrainTorqueCapacity01,
            F.EngineHealthFactor01,
            F.RequestedEngineTorqueAfterLimiterHealthNm,
            F.PermissionGatedAvailableEngineTorqueNm,
            F.RequestedGear,
            F.EngagedGear,
            F.ChaosCurrentGear,
            F.ChaosTargetGear,
            F.Actuation.EffectiveGearRatio,
            F.Actuation.ConfiguredFinalDriveRatio,
            F.Actuation.ExternalRearDriveTorquePerWheelNm,
            DrivePathToken(F.Actuation.DriveTorquePath),
            F.ForwardSpeedKmh,
            F.WorldSpeedMps,
            F.VehicleMassKg,
            F.TranslationalKineticEnergyJ,
            F.Control.RawSteeringMouseDelta,
            F.Control.SteeringVirtualCursor,
            F.Control.SteeringTarget,
            F.Control.FinalSteeringCommand,
            F.Actuation.bTorqueControlEnabled ? 1 : 0,
            F.Actuation.bTargetRotationControlEnabled ? 1 : 0,
            F.Actuation.bStabilizeControlEnabled ? 1 : 0,
            F.Actuation.bAnyAbsConfigured ? 1 : 0,
            F.Actuation.bAnyTractionControlConfigured ? 1 : 0,
            F.Actuation.AssistContribution,
            F.Wheels.Num());
    }
    return Csv;
}


FString FPinkCabCausalTelemetryTrace::ToWheelCsv() const
{
    FString Csv = TEXT(
        "sequence,timestamp_s,wheel_index,in_contact,wheel_rpm,steer_angle_deg,"
        "suspension_length,spring_force_n,slip_angle,slip_magnitude,drive_torque_nm,"
        "brake_torque_nm,abs_configured,abs_active,tc_configured,"
        "normal_load_available,normal_load_n,slip_ratio_available,slip_ratio,"
        "longitudinal_force_available,longitudinal_force_n,lateral_force_available,lateral_force_n\n");

    for (const FPinkCabCausalTelemetryFrame& Frame : Frames)
    {
        for (const FPinkCabCausalWheelTelemetry& Wheel : Frame.Wheels)
        {
            Csv += FString::Printf(
                TEXT("%llu,%.9f,%d,%d,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,%.6f,")
                TEXT("%d,%d,%d,%d,%.6f,%d,%.6f,%d,%.6f,%d,%.6f\n"),
                static_cast<unsigned long long>(Frame.Sequence),
                Frame.TimestampSeconds,
                Wheel.WheelIndex,
                Wheel.bInContact ? 1 : 0,
                Wheel.WheelRpm,
                Wheel.SteerAngleDeg,
                Wheel.NormalizedSuspensionLength,
                Wheel.SuspensionSpringForce,
                Wheel.SlipAngle,
                Wheel.SlipMagnitude,
                Wheel.DriveTorqueNm,
                Wheel.BrakeTorqueNm,
                Wheel.bABSConfigured ? 1 : 0,
                Wheel.bABSActivated ? 1 : 0,
                Wheel.bTractionControlConfigured ? 1 : 0,
                Wheel.bHasNormalLoad ? 1 : 0,
                Wheel.NormalLoadN,
                Wheel.bHasSlipRatio ? 1 : 0,
                Wheel.SlipRatio,
                Wheel.bHasLongitudinalForce ? 1 : 0,
                Wheel.LongitudinalForceN,
                Wheel.bHasLateralForce ? 1 : 0,
                Wheel.LateralForceN);
        }
    }
    return Csv;
}
