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
        || !Native->bMechanicalSimEnabled
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
struct FNativeWheelResponse
{
    double ShaftOmega = 0.0;
    double WheelWorkJ = 0.0;
    double TorqueFactor = 1.0;
    double PredictedOmega[4] = {};
    bool bValid = false;
};

FNativeWheelResponse EvaluateNativeWheels(const Chaos::FSimpleWheeledVehicle& Vehicle,
    const FWheelState& State, double ClutchTorque, double Ratio, double Efficiency, double Dt)
{
    FNativeWheelResponse Out;
    double Weight = 0.0, BeforeShaft = 0.0;
    for (const auto& Wheel : Vehicle.Wheels)
    {
        if (!Wheel.EngineEnabled) continue;
        Weight += Wheel.Setup().TorqueRatio;
        BeforeShaft += Wheel.Setup().TorqueRatio * Wheel.GetAngularVelocity() * Ratio;
    }
    if (Weight <= 0.0 || Vehicle.Wheels.Num() > 4) return Out;
    BeforeShaft /= Weight;
    Out.TorqueFactor = ClutchTorque * BeforeShaft < 0.0 ? 1.0 / Efficiency : Efficiency;
    for (int32 DirectionPass = 0; DirectionPass < 2; ++DirectionPass)
    {
        Out.ShaftOmega = Out.WheelWorkJ = 0.0;
        for (int32 Index = 0; Index < Vehicle.Wheels.Num(); ++Index)
        {
            const auto& Original = Vehicle.Wheels[Index];
            if (!State.LocalWheelVelocity.IsValidIndex(Index)) return Out;
            // Pure native component evaluation. No force reaches a scene and no
            // actual wheel/chassis state is advanced during these candidates.
            auto Wheel = Original;
            Wheel.SetVehicleGroundSpeed(FRotator(0, Wheel.GetSteeringAngle(), 0)
                .UnrotateVector(State.LocalWheelVelocity[Index]));
            const double Share = Original.EngineEnabled ? Original.Setup().TorqueRatio / Weight : 0.0;
            const double WheelTorque = ClutchTorque * Ratio * Out.TorqueFactor * Share;
            Wheel.SetDriveTorque(Chaos::TorqueMToCm(static_cast<float>(WheelTorque)));
            Wheel.Simulate(static_cast<float>(Dt));
            Out.PredictedOmega[Index] = Wheel.GetAngularVelocity();
            Out.ShaftOmega += Share * Wheel.GetAngularVelocity() * Ratio;
            // Native wheel Simulate advances angle using its end-step omega.
            // Match that implicit power port; pre-step wheel spin is not clutch work.
            Out.WheelWorkJ += WheelTorque * Dt * Wheel.GetAngularVelocity();
        }
        const double Direction = ClutchTorque * Out.ShaftOmega;
        const double Factor = Direction < 0.0 ? 1.0 / Efficiency : Efficiency;
        if (Factor == Out.TorqueFactor) break;
        Out.TorqueFactor = Factor;
    }
    Out.bValid = FMath::IsFinite(Out.ShaftOmega) && FMath::IsFinite(Out.WheelWorkJ);
    return Out;
}

struct FNativeCoupledClutch
{
    FPinkCabNativeClutchResult Connection;
    FNativeWheelResponse Wheels;
    double ResidualNm = 0.0;
    bool bValid = false;
};

FNativeCoupledClutch SolveNativeCoupledClutch(FPinkCabChaosNativeClutchJoint& Joint,
    const FPinkCabChaosCommandFrame& Frame, const Chaos::FSimpleWheeledVehicle& Vehicle,
    const FWheelState& State, double EngineOmega, double Ratio, double Efficiency, double Dt)
{
    FNativeCoupledClutch Out;
    const auto& Config = Frame.ClutchConfig;
    const double Capacity = Config.MaxClutchTorqueNm * Frame.Controls.ClutchCoupling * Frame.Controls.DrivetrainTorqueCapacity;
    double Lower = -Capacity, Upper = Capacity;
    // Bounded interface convergence, not a replacement tyre/engine equation.
    // Each candidate uses only native wheel and native joint responses. All
    // candidates start from the same state; only the final torque is scattered.
    for (int32 Iteration = 0; Iteration < 24; ++Iteration)
    {
        const double Candidate = 0.5 * (Lower + Upper);
        Out.Wheels = EvaluateNativeWheels(Vehicle, State, Candidate, Ratio, Efficiency, Dt);
        if (!Out.Wheels.bValid) return Out;
        Out.Connection = Joint.Solve(EngineOmega, Out.Wheels.ShaftOmega, Config.EngineEffectiveInertia,
            1.0, Capacity, Config.SynchronizationTimeSeconds, Dt, true);
        if (!Out.Connection.bValid) return Out;
        Out.ResidualNm = Out.Connection.TransferredTorqueNm - Candidate;
        const double RoundoffNm = 4.0 * FLT_EPSILON * Config.EngineEffectiveInertia
            * FMath::Max(1.0, FMath::Max(FMath::Abs(EngineOmega), FMath::Abs(Out.Wheels.ShaftOmega))) / Dt;
        if (FMath::Abs(Out.ResidualNm) <= FMath::Max(RoundoffNm, 32.0 * FLT_EPSILON * FMath::Max(1.0, Capacity)))
        {
            Out.Wheels = EvaluateNativeWheels(Vehicle, State, Out.Connection.TransferredTorqueNm, Ratio, Efficiency, Dt);
            Out.bValid = Out.Wheels.bValid;
            return Out;
        }
        if (Out.ResidualNm > 0.0) Lower = Candidate; else Upper = Candidate;
    }
    return Out;
}
}

void FPinkCabChaosDrivelineSimulation::ProcessMechanicalSimulation(float DeltaTime)
{
    if (!PVehicle || !PVehicle->HasEngine() || !PVehicle->HasTransmission() || DeltaTime <= 0) return;
    auto& Engine = PVehicle->GetEngine();
    auto& Transmission = PVehicle->GetTransmission();
    for (auto& Wheel : PVehicle->Wheels) Wheel.SetDriveTorque(0.0f);
    if (!Frame.bValid) { Engine.StopEngine(); return; }
    if (Frame.Controls.IsCombustionAllowed()) Engine.StartEngine(); else Engine.StopEngine();
    Transmission.SetGear(Frame.Controls.EngagedGear, true);
    Transmission.Simulate(DeltaTime);
    const double Ratio = Transmission.GetGearRatio(Transmission.GetCurrentGear());
    Step.EffectiveGearRatio = Ratio;
    Engine.SetEngineRPM(true, 0.0f);
    Engine.Simulate(DeltaTime);
    Step.EngineOmegaBefore = Engine.GetEngineOmega();
    Step.EngineOmegaAfter = Step.EngineOmegaBefore;
    if (FMath::IsNearlyZero(Ratio)) return;
    bClutchStepPending = true;
}

void FPinkCabChaosDrivelineSimulation::ApplyWheelFrictionForces(float DeltaTime)
{
    if (bClutchStepPending) ApplyNativeClutchAtWheelBoundary(DeltaTime);
    bClutchStepPending = false;
    UChaosWheeledVehicleSimulation::ApplyWheelFrictionForces(DeltaTime);
}

void FPinkCabChaosDrivelineSimulation::ApplyNativeClutchAtWheelBoundary(float DeltaTime)
{
    if (!PVehicle || !Frame.bValid) return;
    auto& Engine = PVehicle->GetEngine();
    auto& Transmission = PVehicle->GetTransmission();
    const double Ratio = Transmission.GetGearRatio(Transmission.GetCurrentGear());
    const double Efficiency = Transmission.Setup().TransmissionEfficiency;
    if (!FMath::IsFinite(Efficiency) || Efficiency <= 0.0 || Efficiency > 1.0) return;
    const auto Coupled = SolveNativeCoupledClutch(ClutchJoint, Frame, *PVehicle,
        WheelState, Step.EngineOmegaBefore, Ratio, Efficiency, DeltaTime);
    Step.NativeCouplingResidualNm = Coupled.ResidualNm;
    Step.bNativeResponseConverged = Coupled.bValid;
    if (!ensureMsgf(Coupled.bValid, TEXT("PINKCAB_CLUTCH_NATIVE_INTERFACE_NOT_CONVERGED residual_nm=%.8f sequence=%llu"),
        Coupled.ResidualNm, Step.CommandSequence)) return;
    const auto& Result = Coupled.Connection;
    Engine.SetEngineOmega(static_cast<float>(Result.EngineOmega));
    Transmission.SetEngineRPM(Chaos::OmegaToRPM(static_cast<float>(Result.EngineOmega)));
    double Weight = 0.0;
    for (const auto& Wheel : PVehicle->Wheels) if (Wheel.EngineEnabled) Weight += Wheel.Setup().TorqueRatio;
    const double TotalWheelTorque = Result.TransferredTorqueNm * Ratio * Coupled.Wheels.TorqueFactor;
    for (auto& Wheel : PVehicle->Wheels)
        if (Wheel.EngineEnabled) Wheel.SetDriveTorque(Chaos::TorqueMToCm(static_cast<float>(TotalWheelTorque * Wheel.Setup().TorqueRatio / Weight)));
    const double EngineDeltaJ = 0.5 * Frame.ClutchConfig.EngineEffectiveInertia
        * (Result.EngineOmega * Result.EngineOmega - Step.EngineOmegaBefore * Step.EngineOmegaBefore);
    Step.ConnectionEnergyDeltaJ = EngineDeltaJ + Coupled.Wheels.WheelWorkJ;
    Step.GearLossJ = -Step.ConnectionEnergyDeltaJ - Result.DissipatedEnergyJ;
    Step.bNativeJointApplied = true;
    Step.EngineOmegaAfter = Result.EngineOmega;
    Step.ShaftOmega = Coupled.Wheels.ShaftOmega;
    Step.PredictedShaftOmega = Coupled.Wheels.ShaftOmega;
    Step.TransferredTorqueNm = Result.TransferredTorqueNm;
    Step.CapacityNm = Frame.ClutchConfig.MaxClutchTorqueNm * Frame.Controls.ClutchCoupling * Frame.Controls.DrivetrainTorqueCapacity;
    Step.DissipatedEnergyJ = Result.DissipatedEnergyJ;
    Step.MomentumResidual = Result.MomentumResidual;
}
