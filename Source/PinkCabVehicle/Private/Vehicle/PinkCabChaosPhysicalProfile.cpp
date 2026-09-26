#include "Vehicle/PinkCabChaosPhysicalProfile.h"

#include "ChaosVehicleWheel.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "Vehicle/PinkCabTatraProfile.h"

namespace
{
template <typename T>
TPinkCabPhysicalParameter<T> P(T Value, EPinkCabPhysicalParameterAuthority Authority)
{
    return TPinkCabPhysicalParameter<T>{MoveTemp(Value), Authority};
}

bool HasAuthority(const EPinkCabPhysicalParameterAuthority Authority)
{
    return Authority != EPinkCabPhysicalParameterAuthority::Unspecified;
}


template <int32 N>
bool AllAuthoritiesSpecified(
    const EPinkCabPhysicalParameterAuthority (&Authorities)[N])
{
    for (const EPinkCabPhysicalParameterAuthority Authority : Authorities)
    {
        if (!HasAuthority(Authority))
        {
            return false;
        }
    }
    return true;
}

bool HasWheelProvenance(const FPinkCabChaosWheelPhysicalProfile& W)
{
    const EPinkCabPhysicalParameterAuthority Authorities[] = {
        W.WheelRadiusCm.Authority,
        W.WheelWidthCm.Authority,
        W.WheelMassKg.Authority,
        W.CorneringStiffness.Authority,
        W.FrictionForceMultiplier.Authority,
        W.SideSlipModifier.Authority,
    };
    return AllAuthoritiesSpecified(Authorities);
}

bool HasWheelProvenanceTail(const FPinkCabChaosWheelPhysicalProfile& W)
{
    const EPinkCabPhysicalParameterAuthority Authorities[] = {
        W.SlipThreshold.Authority,
        W.SkidThreshold.Authority,
        W.MaxSteerAngleDeg.Authority,
        W.MaxBrakeTorqueNm.Authority,
        W.MaxHandBrakeTorqueNm.Authority,
        W.SpringRate.Authority,
        W.SpringPreload.Authority,
        W.SuspensionMaxRaiseCm.Authority,
        W.SuspensionMaxDropCm.Authority,
        W.SuspensionDampingRatio.Authority,
        W.WheelLoadRatio.Authority,
        W.RollbarScaling.Authority,
        W.bABSEnabled.Authority,
        W.bTractionControlEnabled.Authority,
        W.bAffectedBySteering.Authority,
        W.bAffectedByEngine.Authority,
        W.bAffectedByBrake.Authority,
        W.bAffectedByHandbrake.Authority,
    };
    return AllAuthoritiesSpecified(Authorities);
}

float VariantFriction(
    const EPinkCabCalibrationVariant Variant,
    const bool bFront)
{
    // Front axle stays planted/readable. Rear axle intentionally has a much
    // smaller friction budget so RWD torque can produce progressive wheelspin
    // instead of converting every extra Nm into rocket-like linear launch.
    if (bFront)
    {
        return Variant == EPinkCabCalibrationVariant::Low ? 1.80f
            : Variant == EPinkCabCalibrationVariant::High ? 2.20f : 2.00f;
    }
    return Variant == EPinkCabCalibrationVariant::Low ? 0.44f
        : Variant == EPinkCabCalibrationVariant::High ? 0.56f : 0.50f;
}

float VariantSpring(const EPinkCabCalibrationVariant Variant)
{
    return Variant == EPinkCabCalibrationVariant::Low ? 150.0f
        : Variant == EPinkCabCalibrationVariant::High ? 195.0f : 170.0f;
}
FPinkCabChaosWheelPhysicalProfile MakeWheel(
    const bool bFront,
    const EPinkCabCalibrationVariant Variant)
{
    using A = EPinkCabPhysicalParameterAuthority;
    FPinkCabChaosWheelPhysicalProfile W;
    W.WheelRadiusCm = P(32.13f, A::Source);
    W.WheelWidthCm = P(20.5f, A::Source);
    W.WheelMassKg = P(20.0f, A::Calibration);
    W.CorneringStiffness = P(1000.0f, A::Calibration);
    W.FrictionForceMultiplier = P(VariantFriction(Variant, bFront), A::Calibration);
    W.SideSlipModifier = P(1.0f, A::Calibration);
    W.SlipThreshold = P(20.0f, A::Calibration);
    W.SkidThreshold = P(20.0f, A::Calibration);
    W.MaxSteerAngleDeg = P(bFront ? 41.0f : 0.0f, A::DesignTarget);
    W.MaxBrakeTorqueNm = P(bFront ? 2400.0f : 2200.0f, A::Calibration);
    W.MaxHandBrakeTorqueNm = P(bFront ? 0.0f : 1700.0f, A::Calibration);
    W.SpringRate = P(VariantSpring(Variant), A::Calibration);
    W.SpringPreload = P(32.0f, A::Calibration);
    // Keep the last pre-integration executable suspension seed:
    // 160 mm total front travel and 180 mm total rear travel.
    W.SuspensionMaxRaiseCm = P(bFront ? 8.0f : 9.0f, A::Calibration);
    W.SuspensionMaxDropCm = P(bFront ? 8.0f : 9.0f, A::Calibration);
    W.SuspensionDampingRatio = P(0.38f, A::Calibration);
    W.WheelLoadRatio = P(0.38f, A::Calibration);
    W.RollbarScaling = P(0.08f, A::Calibration);
    W.bABSEnabled = P(false, A::DesignTarget);
    W.bTractionControlEnabled = P(false, A::DesignTarget);
    W.bAffectedBySteering = P(bFront, A::DesignTarget);
    W.bAffectedByEngine = P(!bFront, A::DesignTarget);
    W.bAffectedByBrake = P(true, A::DesignTarget);
    W.bAffectedByHandbrake = P(!bFront, A::DesignTarget);
    return W;
}
} // namespace

FPinkCabChaosPhysicalProfile FPinkCabChaosPhysicalProfile::ForVariant(
    const EPinkCabCalibrationVariant Variant)
{
    using A = EPinkCabPhysicalParameterAuthority;
    FPinkCabChaosPhysicalProfile R;
    R.ModelId = FName(TEXT("TATRA_613"));
    R.ProfileId = FName(TEXT("PINKCAB_TATRA613_CHAOS"));
    R.SchemaVersion = 1;
    R.CalibrationVersion = 3;
    R.UnitSystemId = FName(TEXT("PINKCAB_PHYSICS_UNITS_V1"));
    R.ProvenanceSetId = FName(TEXT("PINKCAB_TATRA613_BASELINE_2026_09_26"));
    R.CompatibilityId = FName(TEXT("PINKCAB_CHAOS_PROFILE_V1"));
    R.MigrationId = FName(TEXT("PINKCAB_TATRA613_PROFILE_V1"));
    R.CalibrationVariant = Variant;
    const FPinkCabTatraProfile Tatra = FPinkCabTatraProfile::Canonical();
    R.ReferenceMassKg = P(Tatra.GetReferenceCrewMassKg(), A::DesignTarget);
    R.WheelbaseMm = P(2980.0f, A::Source);
    R.FrontTrackMm = P(1520.0f, A::Source);
    R.RearTrackMm = P(1520.0f, A::Source);
    R.MaxPowerHp = P(250.0f, A::DesignTarget);
    R.MaxTorqueNm = P(260.0f, A::DesignTarget);
    R.TerminalTargetKmh = P(195.0f, A::DesignTarget);
    R.bRearWheelDrive = P(true, A::DesignTarget);
    R.EngineMaxRpm = P(8500.0f, A::Calibration);
    R.EngineIdleRpm = P(925.0f, A::Calibration);
    R.EngineBrakeEffect = P(0.15f, A::Calibration);
    R.EngineRevUpMOI = P(0.17f, A::Calibration);
    R.EngineRevDownRate = P(1800.0f, A::Calibration);
    // Supercharged/high-rev design target: strong low/mid response and a broad
    // compressor-fed plateau with roughly 250 hp still available at the 8500 rpm
    // redline. This changes engine character only; pedal, gearbox, steering and
    // tire-control semantics remain untouched.
    R.NormalizedTorqueCurve = P(TArray<FVector2D>{
        FVector2D(0.0, 0.85), FVector2D(800.0, 0.92), FVector2D(2000.0, 0.95),
        FVector2D(3500.0, 0.98), FVector2D(5000.0, 1.00), FVector2D(6500.0, 0.97),
        FVector2D(7500.0, 0.90), FVector2D(8500.0, 0.81)}, A::Calibration);
    R.bUseAutomaticGears = P(false, A::Calibration);
    R.bUseAutoReverse = P(false, A::Calibration);
    R.FinalDriveRatio = P(3.2f, A::Calibration);
    R.ForwardGearRatios = P(TArray<float>{4.6f, 2.2f, 1.5f, 1.1f, 0.85f}, A::Calibration);
    R.ReverseGearRatios = P(TArray<float>{4.6f}, A::Calibration);
    R.SteeringAngleRatio = P(0.72f, A::Calibration);
    R.FrontWheel = MakeWheel(true, Variant);
    R.RearWheel = MakeWheel(false, Variant);
    return R;
}


bool FPinkCabChaosPhysicalProfile::HasCompleteProvenance() const
{
    const EPinkCabPhysicalParameterAuthority Authorities[] = {
        ReferenceMassKg.Authority,
        WheelbaseMm.Authority,
        FrontTrackMm.Authority,
        RearTrackMm.Authority,
        MaxPowerHp.Authority,
        MaxTorqueNm.Authority,
        TerminalTargetKmh.Authority,
        bRearWheelDrive.Authority,
        EngineMaxRpm.Authority,
        EngineIdleRpm.Authority,
        EngineBrakeEffect.Authority,
        EngineRevUpMOI.Authority,
        EngineRevDownRate.Authority,
        NormalizedTorqueCurve.Authority,
        bUseAutomaticGears.Authority,
        bUseAutoReverse.Authority,
        FinalDriveRatio.Authority,
        ForwardGearRatios.Authority,
        ReverseGearRatios.Authority,
        SteeringAngleRatio.Authority,
    };
    return HasValidEnvelope()
        && AllAuthoritiesSpecified(Authorities)
        && HasWheelProvenance(FrontWheel)
        && HasWheelProvenanceTail(FrontWheel)
        && HasWheelProvenance(RearWheel)
        && HasWheelProvenanceTail(RearWheel);
}

void FPinkCabChaosPhysicalProfile::ApplyToMovement(
    UChaosWheeledVehicleMovementComponent& Movement) const
{
    Movement.Mass = ReferenceMassKg.Value;
    Movement.DifferentialSetup.DifferentialType = EVehicleDifferential::RearWheelDrive;
    Movement.EngineSetup.MaxTorque = MaxTorqueNm.Value;
    Movement.EngineSetup.MaxRPM = EngineMaxRpm.Value;
    Movement.EngineSetup.EngineIdleRPM = EngineIdleRpm.Value;
    Movement.EngineSetup.EngineBrakeEffect = EngineBrakeEffect.Value;
    Movement.EngineSetup.EngineRevUpMOI = EngineRevUpMOI.Value;
    Movement.EngineSetup.EngineRevDownRate = EngineRevDownRate.Value;
    FRichCurve* TorqueCurve = Movement.EngineSetup.TorqueCurve.GetRichCurve();
    TorqueCurve->Reset();
    for (const FVector2D& Key : NormalizedTorqueCurve.Value)
    {
        TorqueCurve->AddKey(static_cast<float>(Key.X), static_cast<float>(Key.Y));
    }

    Movement.TransmissionSetup.bUseAutomaticGears = bUseAutomaticGears.Value;
    Movement.TransmissionSetup.bUseAutoReverse = bUseAutoReverse.Value;
    // PINK CAB owns a physical H-pattern manual gearbox. Chaos' arcade
    // Reverse-As-Brake path otherwise forces +1 whenever throttle is applied
    // while reverse is selected, overriding the driver's engaged R gear.
    Movement.bReverseAsBrake = false;
    Movement.bThrottleAsBrake = false;
    Movement.TransmissionSetup.FinalRatio = FinalDriveRatio.Value;
    Movement.TransmissionSetup.ForwardGearRatios = ForwardGearRatios.Value;
    Movement.TransmissionSetup.ReverseGearRatios = ReverseGearRatios.Value;
    Movement.SteeringSetup.SteeringType = ESteeringType::Ackermann;
    Movement.SteeringSetup.AngleRatio = SteeringAngleRatio.Value;

    Movement.TorqueControl.Enabled = false;
    Movement.TargetRotationControl.Enabled = false;
    Movement.StabilizeControl.Enabled = false;
}

void FPinkCabChaosPhysicalProfile::ApplyToFrontWheel(UChaosVehicleWheel& Wheel) const
{
    const FPinkCabChaosWheelPhysicalProfile& W = FrontWheel;
    Wheel.AxleType = EAxleType::Front;
    Wheel.MaxSteerAngle = W.MaxSteerAngleDeg.Value;
    Wheel.WheelRadius = W.WheelRadiusCm.Value;
    Wheel.WheelWidth = W.WheelWidthCm.Value;
    Wheel.WheelMass = W.WheelMassKg.Value;
    Wheel.CorneringStiffness = W.CorneringStiffness.Value;
    Wheel.FrictionForceMultiplier = W.FrictionForceMultiplier.Value;
    Wheel.SideSlipModifier = W.SideSlipModifier.Value;
    Wheel.SlipThreshold = W.SlipThreshold.Value;
    Wheel.SkidThreshold = W.SkidThreshold.Value;
    Wheel.MaxBrakeTorque = W.MaxBrakeTorqueNm.Value;
    Wheel.MaxHandBrakeTorque = W.MaxHandBrakeTorqueNm.Value;
    Wheel.SpringRate = W.SpringRate.Value;
    Wheel.SpringPreload = W.SpringPreload.Value;
    Wheel.SuspensionMaxRaise = W.SuspensionMaxRaiseCm.Value;
    Wheel.SuspensionMaxDrop = W.SuspensionMaxDropCm.Value;
    Wheel.SuspensionDampingRatio = W.SuspensionDampingRatio.Value;
    Wheel.WheelLoadRatio = W.WheelLoadRatio.Value;
    Wheel.RollbarScaling = W.RollbarScaling.Value;
    Wheel.bABSEnabled = W.bABSEnabled.Value;
    Wheel.bTractionControlEnabled = W.bTractionControlEnabled.Value;
    Wheel.bAffectedBySteering = W.bAffectedBySteering.Value;
    Wheel.bAffectedByEngine = W.bAffectedByEngine.Value;
    Wheel.bAffectedByBrake = W.bAffectedByBrake.Value;
    Wheel.bAffectedByHandbrake = W.bAffectedByHandbrake.Value;
}
void FPinkCabChaosPhysicalProfile::ApplyToRearWheel(UChaosVehicleWheel& Wheel) const
{
    const FPinkCabChaosWheelPhysicalProfile& W = RearWheel;
    Wheel.AxleType = EAxleType::Rear;
    Wheel.MaxSteerAngle = W.MaxSteerAngleDeg.Value;
    Wheel.WheelRadius = W.WheelRadiusCm.Value;
    Wheel.WheelWidth = W.WheelWidthCm.Value;
    Wheel.WheelMass = W.WheelMassKg.Value;
    Wheel.CorneringStiffness = W.CorneringStiffness.Value;
    Wheel.FrictionForceMultiplier = W.FrictionForceMultiplier.Value;
    Wheel.SideSlipModifier = W.SideSlipModifier.Value;
    Wheel.SlipThreshold = W.SlipThreshold.Value;
    Wheel.SkidThreshold = W.SkidThreshold.Value;
    Wheel.MaxBrakeTorque = W.MaxBrakeTorqueNm.Value;
    Wheel.MaxHandBrakeTorque = W.MaxHandBrakeTorqueNm.Value;
    Wheel.SpringRate = W.SpringRate.Value;
    Wheel.SpringPreload = W.SpringPreload.Value;
    Wheel.SuspensionMaxRaise = W.SuspensionMaxRaiseCm.Value;
    Wheel.SuspensionMaxDrop = W.SuspensionMaxDropCm.Value;
    Wheel.SuspensionDampingRatio = W.SuspensionDampingRatio.Value;
    Wheel.WheelLoadRatio = W.WheelLoadRatio.Value;
    Wheel.RollbarScaling = W.RollbarScaling.Value;
    Wheel.bABSEnabled = W.bABSEnabled.Value;
    Wheel.bTractionControlEnabled = W.bTractionControlEnabled.Value;
    Wheel.bAffectedBySteering = W.bAffectedBySteering.Value;
    Wheel.bAffectedByEngine = W.bAffectedByEngine.Value;
    Wheel.bAffectedByBrake = W.bAffectedByBrake.Value;
    Wheel.bAffectedByHandbrake = W.bAffectedByHandbrake.Value;
}
