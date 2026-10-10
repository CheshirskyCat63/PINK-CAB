#include "Vehicle/PinkCabChaosVehicleDynamicsProvider.h"
#include "Vehicle/PinkCabChaosEngineAdapter.h"
#include "Vehicle/PinkCabChaosVehicleMovementComponent.h"

#include "PinkCabChaosDrivelineSimulation.h"
#include "SimpleVehicle.h"
#include "ChaosVehicleWheel.h"
#include "ChaosWheeledVehicleMovementComponent.h"

namespace
{
void PopulateActuationTelemetry(
    UChaosWheeledVehicleMovementComponent& Movement,
    const FPinkCabVehicleControlState& Controls,
    FPinkCabCausalActuationTelemetry& Out)
{
    Out = {};
    Out.HealthClampedControlThrottle01 = FMath::Clamp(Controls.Throttle, 0.0f, 1.0f);
    Out.EngineThrottlePreLimiter01 = Controls.GetResolvedEngineThrottlePreLimiter01();
    Out.EngineThrottleFinal01 = Controls.GetResolvedEngineThrottle01();
    Out.ChaosThrottleInput01 = FPinkCabChaosEngineAdapter::ToChaosThrottleInput(
        Out.EngineThrottleFinal01);
    Out.EngineTorqueCurveNm = Controls.GetResolvedEngineTorqueCurveNm();
    Out.RequestedEngineTorqueAfterLimiterHealthNm = Controls.GetAvailableEngineTorqueNm();
    Out.EffectiveGearRatio = Controls.EngagedGear != 0
        ? Movement.TransmissionSetup.GetGearRatio(Controls.EngagedGear)
        : 0.0f;
    Out.ConfiguredFinalDriveRatio = Movement.TransmissionSetup.FinalRatio;
    Out.TransmissionEfficiency = Movement.TransmissionSetup.TransmissionEfficiency;
    Out.ExternalRearDriveTorquePerWheelNm = 0.0f;
    Out.DriveTorquePath = Controls.EngagedGear != 0
        ? EPinkCabCausalDriveTorquePath::NativeConstraintClutch
        : EPinkCabCausalDriveTorquePath::None;
    Out.bTorqueControlEnabled = Movement.TorqueControl.Enabled;
    Out.bTargetRotationControlEnabled = Movement.TargetRotationControl.Enabled;
    Out.bStabilizeControlEnabled = Movement.StabilizeControl.Enabled;
    Out.AssistContribution = 0.0f;
}

void PopulateConfiguredAssistFlags(
    const UChaosWheeledVehicleMovementComponent& Movement,
    FPinkCabCausalActuationTelemetry& Out)
{
    Out.bAnyAbsConfigured = false;
    Out.bAnyTractionControlConfigured = false;
    for (const UChaosVehicleWheel* Wheel : Movement.Wheels)
    {
        if (!Wheel)
        {
            continue;
        }
        Out.bAnyAbsConfigured |= Wheel->bABSEnabled;
        Out.bAnyTractionControlConfigured |= Wheel->bTractionControlEnabled;
    }
}
}

FPinkCabChaosVehicleDynamicsProvider::FPinkCabChaosVehicleDynamicsProvider(
    UChaosWheeledVehicleMovementComponent* InMovement)
    : Movement(InMovement)
{
}

EPinkCabMechanicalClutchCapability FPinkCabChaosVehicleDynamicsProvider::GetMechanicalClutchCapability() const
{
    const auto* Native = Cast<UPinkCabChaosVehicleMovementComponent>(Movement);
    // Availability of the configured extension, not proof of clutch acceptance.
    if (!IsValid(Native) || !Native->IsPhysicsStateCreated()
        || !Native->GetPinkCabClutchConfig().IsValid()
        || Native->Wheels.IsEmpty() || Native->Wheels.Num() != Native->WheelSetups.Num())
    {
        return EPinkCabMechanicalClutchCapability::Unsupported;
    }
    for (const auto& Wheel : Native->Wheels)
        if (!IsValid(Wheel.Get())) return EPinkCabMechanicalClutchCapability::Unsupported;
    return EPinkCabMechanicalClutchCapability::NativeConstraintExtension;
}

bool FPinkCabChaosVehicleDynamicsProvider::ApplyControls(
    const FPinkCabVehicleControlState& Controls)
{
    UPinkCabChaosVehicleMovementComponent* NativeInput = Cast<UPinkCabChaosVehicleMovementComponent>(Movement);
    if (!NativeInput)
    {
        return false;
    }

    LastControls = Controls;
    PopulateActuationTelemetry(*Movement, Controls, LastCausalActuation);

    Movement->SetUseAutomaticGears(false);
    Movement->SetSteeringInput(Controls.Steering);
    Movement->SetThrottleInput(
        Controls.IsCombustionAllowed()
            ? LastCausalActuation.ChaosThrottleInput01
            : 0.0f);
    Movement->SetBrakeInput(FMath::Clamp(Controls.Brake, 0.0f, 1.0f));
    Movement->SetHandbrakeInput(false); // Only the semantic adapter writes raw native controls.
    NativeInput->SetPinkCabHandbrakeInput(Controls.Handbrake);

    // Validated H-pattern engagement is independent of clutch pressure.
    // One complete command carries that pressure to the native mechanical step.
    Movement->SetTargetGear(Controls.EngagedGear, true);
    NativeInput->SetPinkCabControlState(Controls);

    PopulateConfiguredAssistFlags(*Movement, LastCausalActuation);
    return true;
}


namespace
{
struct FPinkCabReflectedAxle
{
    double Weight = 0.0;
    double Omega = 0.0;
    double InertiaKgM2 = 0.0;
};

bool GatherReflectedAxle(const Chaos::FSimpleWheeledVehicle& Vehicle,
    double Ratio, FPinkCabReflectedAxle& Out)
{
    double Weight = 0.0, WeightedOmega = 0.0, InverseShaftInertia = 0.0;
    for (const auto& Wheel : Vehicle.Wheels)
    {
        if (!Wheel.EngineEnabled) continue;
        const double Portion = Wheel.Setup().TorqueRatio;
        const double InertiaKgM2 = Wheel.Inertia / 10000.0;
        if (Portion <= 0 || InertiaKgM2 <= 0) continue;
        Weight += Portion;
        WeightedOmega += Portion * Wheel.GetAngularVelocity();
        InverseShaftInertia += Portion * Portion / InertiaKgM2;
    }
    if (Weight <= 0 || InverseShaftInertia <= 0) return false;
    Out.Weight = Weight;
    Out.Omega = Ratio * WeightedOmega / Weight;
    Out.InertiaKgM2 = Weight * Weight / (InverseShaftInertia * Ratio * Ratio);
    return true;
}

struct FPinkCabClutchTransmissionResult
{
    FPinkCabNativeClutchResult Clutch;
    double TorqueFactor = 1.0;
};

FPinkCabClutchTransmissionResult SolveNativeTransmissionConnection(
    FPinkCabChaosNativeClutchJoint& Joint, const FPinkCabChaosCommandFrame& Frame,
    double EngineOmega, const FPinkCabReflectedAxle& Axle, double Efficiency, double Dt)
{
    FPinkCabClutchTransmissionResult Out;
    if (!FMath::IsFinite(Efficiency) || Efficiency <= 0.0 || Efficiency > 1.0) return Out;
    const auto& Config = Frame.ClutchConfig;
    const double Capacity = Config.MaxClutchTorqueNm * Frame.Controls.ClutchCoupling
        * Frame.Controls.DrivetrainTorqueCapacity;
    Out.TorqueFactor = (EngineOmega - Axle.Omega) * Axle.Omega < 0.0 ? 1.0 / Efficiency : Efficiency;
    auto Solve = [&]()
    {
        return Joint.Solve(EngineOmega, Axle.Omega, Config.EngineEffectiveInertia,
            Axle.InertiaKgM2 / Out.TorqueFactor, Capacity, Config.SynchronizationTimeSeconds, Dt);
    };
    Out.Clutch = Solve();
    if (!Out.Clutch.bValid) return Out;
    const double Impulse = Out.Clutch.TransferredTorqueNm * Dt;
    const double ShaftWork = Axle.Omega * Impulse
        + 0.5 * Impulse * Impulse * Out.TorqueFactor / Axle.InertiaKgM2;
    const double CorrectFactor = ShaftWork < 0.0 ? 1.0 / Efficiency : Efficiency;
    // At a zero crossing, choose loss direction by the complete impulse work.
    // Re-evaluate the SAME native joint before scattering any real wheel torque.
    if (CorrectFactor != Out.TorqueFactor)
    {
        Out.TorqueFactor = CorrectFactor;
        Out.Clutch = Solve();
    }
    return Out;
}

// Native gearbox efficiency removes energy in BOTH power directions. Reflect
// that load before the joint solve; never damp only the output reaction torque.
double ScatterNativeWheelTorque(Chaos::FSimpleWheeledVehicle& Vehicle,
    double TotalTorqueNm, double Weight, double Dt)
{
    double EnergyDelta = 0.0;
    for (auto& Wheel : Vehicle.Wheels)
    {
        if (!Wheel.EngineEnabled) continue;
        const double TorqueNm = TotalTorqueNm * Wheel.Setup().TorqueRatio / Weight;
        const double Impulse = TorqueNm * Dt;
        EnergyDelta += Wheel.GetAngularVelocity() * Impulse
            + 0.5 * Impulse * Impulse / (Wheel.Inertia / 10000.0);
        Wheel.SetDriveTorque(Chaos::TorqueMToCm(static_cast<float>(TorqueNm)));
    }
    return EnergyDelta;
}
}

// Physics-thread implementation of this provider. Native Chaos alone integrates
// road contacts/chassis/wheels; all clutch positions use one native torque path.
void FPinkCabChaosDrivelineSimulation::ProcessMechanicalSimulation(float DeltaTime)
{
    if (!PVehicle || !PVehicle->HasEngine() || !PVehicle->HasTransmission() || DeltaTime <= 0) return;
    auto& Engine = PVehicle->GetEngine();
    auto& Transmission = PVehicle->GetTransmission();
    for (auto& Wheel : PVehicle->Wheels) Wheel.SetDriveTorque(0.0f);
    if (!Frame.bValid)
    {
        Engine.StopEngine();
        return;
    }
    const bool Running = Frame.Controls.IsCombustionAllowed();
    if (Running) Engine.StartEngine(); else Engine.StopEngine();
    Transmission.SetGear(Frame.Controls.EngagedGear, true);
    Transmission.Simulate(DeltaTime);
    const double Ratio = Transmission.GetGearRatio(Transmission.GetCurrentGear());
    Step.EffectiveGearRatio = Ratio;
    Engine.SetEngineRPM(true, 0.0f);
    // Preserve native free-rev/drag response. The joint supplies the missing load
    // reaction; stock direct RPM-to-wheel locking is not run as a second path.
    Engine.Simulate(DeltaTime);
    Step.EngineOmegaBefore = Engine.GetEngineOmega();
    Step.EngineOmegaAfter = Step.EngineOmegaBefore;
    if (FMath::IsNearlyZero(Ratio)) return;
    FPinkCabReflectedAxle Axle;
    if (!GatherReflectedAxle(*PVehicle, Ratio, Axle)) return;
    const double Weight = Axle.Weight;
    const double ShaftOmega = Axle.Omega;
    const auto& Config = Frame.ClutchConfig;
    const double Capacity = Config.MaxClutchTorqueNm * Frame.Controls.ClutchCoupling
        * Frame.Controls.DrivetrainTorqueCapacity;
    const double Efficiency = Transmission.Setup().TransmissionEfficiency;
    const auto Connection = SolveNativeTransmissionConnection(ClutchJoint, Frame,
        Step.EngineOmegaBefore, Axle, Efficiency, DeltaTime);
    const auto& Result = Connection.Clutch;
    if (!Result.bValid) return;
    Engine.SetEngineOmega(static_cast<float>(Result.EngineOmega));
    Transmission.SetEngineRPM(Chaos::OmegaToRPM(static_cast<float>(Result.EngineOmega)));
    // Match the native mechanical boundary: wheel storage is kg*cm^2/s^2,
    // while transmission and public wheel telemetry are N*m. This is units, not boost.
    const double WheelTorque = Transmission.GetTransmissionTorque(static_cast<float>(Result.TransferredTorqueNm))
        * Connection.TorqueFactor / Efficiency;
    const double WheelEnergyDelta = ScatterNativeWheelTorque(*PVehicle, WheelTorque, Weight, DeltaTime);
    const double EngineEnergyDelta = 0.5 * Config.EngineEffectiveInertia
        * (Result.EngineOmega * Result.EngineOmega - Step.EngineOmegaBefore * Step.EngineOmegaBefore);
    Step.ConnectionEnergyDeltaJ = EngineEnergyDelta + WheelEnergyDelta;
    Step.GearLossJ = -Step.ConnectionEnergyDeltaJ - Result.DissipatedEnergyJ;
    Step.bNativeJointApplied = true;
    Step.EngineOmegaAfter = Result.EngineOmega;
    Step.ShaftOmega = ShaftOmega;
    Step.TransferredTorqueNm = Result.TransferredTorqueNm;
    Step.CapacityNm = Capacity;
    Step.DissipatedEnergyJ = Result.DissipatedEnergyJ;
    Step.MomentumResidual = Result.MomentumResidual;
}
