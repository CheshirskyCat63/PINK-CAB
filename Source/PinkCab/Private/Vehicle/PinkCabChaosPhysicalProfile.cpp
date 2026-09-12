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

bool HasWheelProvenance(const FPinkCabChaosWheelPhysicalProfile& W)
{
    return HasAuthority(W.WheelRadiusCm.Authority)
        && HasAuthority(W.WheelWidthCm.Authority)
        && HasAuthority(W.WheelMassKg.Authority)
        && HasAuthority(W.CorneringStiffness.Authority)
        && HasAuthority(W.FrictionForceMultiplier.Authority)
        && HasAuthority(W.SideSlipModifier.Authority);
}
bool HasWheelProvenanceTail(const FPinkCabChaosWheelPhysicalProfile& W)
{
    return HasAuthority(W.SlipThreshold.Authority)
        && HasAuthority(W.SkidThreshold.Authority)
        && HasAuthority(W.MaxSteerAngleDeg.Authority)
        && HasAuthority(W.MaxBrakeTorqueNm.Authority)
        && HasAuthority(W.MaxHandBrakeTorqueNm.Authority)
        && HasAuthority(W.SpringRate.Authority)
        && HasAuthority(W.SpringPreload.Authority)
        && HasAuthority(W.SuspensionMaxRaiseCm.Authority)
        && HasAuthority(W.SuspensionMaxDropCm.Authority)
        && HasAuthority(W.SuspensionDampingRatio.Authority)
        && HasAuthority(W.WheelLoadRatio.Authority)
        && HasAuthority(W.RollbarScaling.Authority)
        && HasAuthority(W.bABSEnabled.Authority)
        && HasAuthority(W.bTractionControlEnabled.Authority)
        && HasAuthority(W.bAffectedBySteering.Authority)
        && HasAuthority(W.bAffectedByEngine.Authority)
        && HasAuthority(W.bAffectedByBrake.Authority)
        && HasAuthority(W.bAffectedByHandbrake.Authority);
}

float VariantFriction(const EPinkCabCalibrationVariant Variant)
{
    return Variant == EPinkCabCalibrationVariant::Low ? 1.8f
        : Variant == EPinkCabCalibrationVariant::High ? 2.2f : 2.0f;
}

float VariantSpring(const EPinkCabCalibrationVariant Variant)
{
    return Variant == EPinkCabCalibrationVariant::Low ? 225.0f
        : Variant == EPinkCabCalibrationVariant::High ? 275.0f : 250.0f;
}
FPinkCabChaosWheelPhysicalProfile MakeWheel(
    const bool bFront,
    const EPinkCabCalibrationVariant Variant)
{
    using A = EPinkCabPhysicalParameterAuthority;
    FPinkCabChaosWheelPhysicalProfile W;
    W.WheelRadiusCm = P(34.0f, A::Calibration);
    W.WheelWidthCm = P(18.5f, A::Calibration);
    W.WheelMassKg = P(20.0f, A::Calibration);
    W.CorneringStiffness = P(1000.0f, A::Calibration);
    W.FrictionForceMultiplier = P(VariantFriction(Variant), A::Calibration);
    W.SideSlipModifier = P(1.0f, A::Calibration);
    W.SlipThreshold = P(20.0f, A::Calibration);
    W.SkidThreshold = P(20.0f, A::Calibration);
    W.MaxSteerAngleDeg = P(bFront ? 41.0f : 0.0f, A::DesignTarget);
    W.MaxBrakeTorqueNm = P(bFront ? 2400.0f : 2200.0f, A::Calibration);
    W.MaxHandBrakeTorqueNm = P(bFront ? 0.0f : 1700.0f, A::Calibration);
    W.SpringRate = P(VariantSpring(Variant), A::Calibration);
    W.SpringPreload = P(50.0f, A::Calibration);
    W.SuspensionMaxRaiseCm = P(10.0f, A::Calibration);
    W.SuspensionMaxDropCm = P(10.0f, A::Calibration);
    W.SuspensionDampingRatio = P(0.5f, A::Calibration);
    W.WheelLoadRatio = P(0.5f, A::Calibration);
    W.RollbarScaling = P(0.15f, A::Calibration);
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
    const FPinkCabTatraProfile Tatra = FPinkCabTatraProfile::Canonical();
    R.ReferenceMassKg = P(Tatra.GetReferenceCrewMassKg(), A::DesignTarget);
    R.WheelbaseMm = P(2750.0f, A::Source);
    R.FrontTrackMm = P(1485.0f, A::Source);
    R.RearTrackMm = P(1400.0f, A::Source);
    R.MaxPowerHp = P(180.0f, A::DesignTarget);
    R.MaxTorqueNm = P(240.0f, A::DesignTarget);
    R.TerminalTargetKmh = P(195.0f, A::DesignTarget);
    R.bRearWheelDrive = P(true, A::DesignTarget);
    R.EngineMaxRpm = P(6000.0f, A::Calibration);
    R.EngineIdleRpm = P(750.0f, A::Calibration);
    R.EngineBrakeEffect = P(0.15f, A::Calibration);
    R.NormalizedTorqueCurve = P(TArray<FVector2D>{
        FVector2D(0.0, 0.65), FVector2D(800.0, 0.72), FVector2D(2000.0, 0.90),
        FVector2D(3500.0, 1.00), FVector2D(5000.0, 0.85), FVector2D(6000.0, 0.65)}, A::Calibration);
    R.bUseAutomaticGears = P(true, A::Calibration);
    R.bUseAutoReverse = P(true, A::Calibration);
    R.FinalDriveRatio = P(3.2f, A::Calibration);
    R.ForwardGearRatios = P(TArray<float>{3.8f, 2.2f, 1.5f, 1.1f, 0.85f}, A::Calibration);
    R.ReverseGearRatios = P(TArray<float>{3.5f}, A::Calibration);
    R.SteeringAngleRatio = P(0.72f, A::Calibration);
    R.FrontWheel = MakeWheel(true, Variant);
    R.RearWheel = MakeWheel(false, Variant);
    return R;
}

bool FPinkCabChaosPhysicalProfile::HasCompleteProvenance() const
{
    return HasAuthority(ReferenceMassKg.Authority)
        && HasAuthority(WheelbaseMm.Authority)
        && HasAuthority(FrontTrackMm.Authority)
        && HasAuthority(RearTrackMm.Authority)
        && HasAuthority(MaxPowerHp.Authority)
        && HasAuthority(MaxTorqueNm.Authority)
        && HasAuthority(TerminalTargetKmh.Authority)
        && HasAuthority(bRearWheelDrive.Authority)
        && HasAuthority(EngineMaxRpm.Authority)
        && HasAuthority(EngineIdleRpm.Authority)
        && HasAuthority(EngineBrakeEffect.Authority)
        && HasAuthority(NormalizedTorqueCurve.Authority)
        && HasAuthority(bUseAutomaticGears.Authority)
        && HasAuthority(bUseAutoReverse.Authority)
        && HasAuthority(FinalDriveRatio.Authority)
        && HasAuthority(ForwardGearRatios.Authority)
        && HasAuthority(ReverseGearRatios.Authority)
        && HasAuthority(SteeringAngleRatio.Authority)
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
    FRichCurve* TorqueCurve = Movement.EngineSetup.TorqueCurve.GetRichCurve();
    TorqueCurve->Reset();
    for (const FVector2D& Key : NormalizedTorqueCurve.Value)
    {
        TorqueCurve->AddKey(static_cast<float>(Key.X), static_cast<float>(Key.Y));
    }

    Movement.TransmissionSetup.bUseAutomaticGears = bUseAutomaticGears.Value;
    Movement.TransmissionSetup.bUseAutoReverse = bUseAutoReverse.Value;
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
