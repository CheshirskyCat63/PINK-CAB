#include "PinkCabChaosVehicleSimulation.h"

#include "EngineSystem.h"
#include "SimpleVehicle.h"
#include "VehicleUtility.h"
#include "WheelSystem.h"

namespace
{
constexpr float PinkCabChaosRpmToRadPerSecond = 2.0f * PI / 60.0f;

float MeanDrivenWheelRpm(Chaos::FSimpleWheeledVehicle& Vehicle)
{
    float Sum = 0.0f;
    int32 Count = 0;
    for (int32 Index = 0; Index < Vehicle.Wheels.Num(); ++Index)
    {
        Chaos::FSimpleWheelSim& Wheel = Vehicle.Wheels[Index];
        if (!Wheel.Setup().EngineEnabled)
        {
            continue;
        }
        Sum += FMath::Abs(Wheel.GetWheelRPM());
        ++Count;
    }
    return Count > 0 ? Sum / static_cast<float>(Count) : 0.0f;
}
}

FPinkCabChaosWheeledVehicleSimulation::FPinkCabChaosWheeledVehicleSimulation(
    FThreadSafeCounter64* InMechanicalIntegrationStepCounter,
    FThreadSafeCounter64* InMechanicalIntegrationDeltaMicros)
    : MechanicalIntegrationStepCounter(InMechanicalIntegrationStepCounter)
    , MechanicalIntegrationDeltaMicros(InMechanicalIntegrationDeltaMicros)
{
}

void FPinkCabChaosWheeledVehicleSimulation::SetDrivelineCommand(
    const FPinkCabChaosDrivelineCommand& InCommand)
{
    Command = InCommand;
    ClutchModel.SetConfig(Command.ClutchConfig);
}

void FPinkCabChaosWheeledVehicleSimulation::ApplyInput(
    const FControlInputs& ControlInputs,
    const float DeltaTime)
{
    using namespace Chaos;

    UChaosWheeledVehicleSimulation::ApplyInput(ControlInputs, DeltaTime);
    if (!PVehicle)
    {
        return;
    }

    PhysicsThreadActuation = {};
    bLastParkingEnabled = ControlInputs.ParkingEnabled;
    LastMaxAppliedWheelBrakeTorqueNm = 0.0f;
    if (PVehicle->HasEngine())
    {
        FSimpleEngineSim& Engine = PVehicle->GetEngine();
        const float EngineRpm = Engine.GetEngineRPM();

        FPinkCabEngineActuationInput ActuationInput;
        ActuationInput.bCombustionAllowed = Command.bCombustionAllowed;
        ActuationInput.HealthClampedControlThrottle01 =
            Command.HealthClampedControlThrottle01;
        ActuationInput.EngineRpm = EngineRpm;
        ActuationInput.RpmEnvelope = Command.EngineRpmEnvelope;
        ActuationInput.EngineTorqueCurveNm =
            Engine.GetTorqueFromRPM(EngineRpm, false);
        PhysicsThreadActuation =
            FPinkCabEngineActuationResolver::Resolve(ActuationInput);
        Engine.SetThrottle(PhysicsThreadActuation.EngineThrottleFinal01);
    }

    for (int32 WheelIndex = 0;
         WheelIndex < PVehicle->Wheels.Num();
         ++WheelIndex)
    {
        FSimpleWheelSim& Wheel = PVehicle->Wheels[WheelIndex];
        const FSimpleWheelConfig& Setup = Wheel.Setup();

        float BrakeTorqueNm = 0.0f;
        if (Setup.BrakeEnabled)
        {
            // PinkCab owns service-brake response before the Chaos bridge.
            // ControlInputs.BrakeInput is native Chaos bookkeeping and can
            // retain an independently interpolated stale value after the
            // authoritative PinkCab command has reached zero. Never let that
            // secondary state author wheel torque.
            BrakeTorqueNm = Setup.MaxBrakeTorque
                * FMath::Clamp(Command.ServiceBrake01, 0.0f, 1.0f);
        }

        if ((Command.Handbrake01 > KINDA_SMALL_NUMBER
                && Setup.HandbrakeEnabled)
            || ControlInputs.ParkingEnabled)
        {
            const float HandbrakeTorqueNm =
                ControlInputs.ParkingEnabled
                    ? Setup.HandbrakeTorque
                    : Setup.HandbrakeTorque
                        * FMath::Clamp(Command.Handbrake01, 0.0f, 1.0f);
            BrakeTorqueNm =
                FMath::Max(BrakeTorqueNm, HandbrakeTorqueNm);
        }

        LastMaxAppliedWheelBrakeTorqueNm =
            FMath::Max(LastMaxAppliedWheelBrakeTorqueNm, BrakeTorqueNm);
        Wheel.SetBrakeTorque(TorqueMToCm(BrakeTorqueNm), false);
    }
}

void FPinkCabChaosWheeledVehicleSimulation::ProcessMechanicalSimulation(
    const float DeltaTime)
{
    using namespace Chaos;

    if (!PVehicle || !PVehicle->HasEngine() || !PVehicle->HasTransmission())
    {
        return;
    }

    FSimpleEngineSim& Engine = PVehicle->GetEngine();
    FSimpleTransmissionSim& Transmission = PVehicle->GetTransmission();
    Transmission.SetGear(0, true);

    const float EngineRpmBeforeNative =
        Engine.GetEngineRPM();
    const float EngineOmegaBeforeNative =
        Engine.GetEngineOmega();
    LastEngineInputStateErrorRpm = FMath::Abs(
        EngineRpmBeforeNative
        - EngineOmegaBeforeNative / PinkCabChaosRpmToRadPerSecond);
    AdvanceAcceptedNativeEngine(Transmission, DeltaTime);
    const float EngineOmegaAfterNative =
        Engine.GetEngineOmega();

    const float ObservedFreeEngineNetTorqueNm =
        Command.bCombustionAllowed
            && DeltaTime > KINDA_SMALL_NUMBER
            && Command.ClutchConfig.EngineEffectiveInertia
                > KINDA_SMALL_NUMBER
        ? Command.ClutchConfig.EngineEffectiveInertia
            * (EngineOmegaAfterNative - EngineOmegaBeforeNative)
            / DeltaTime
        : 0.0f;

    const FPinkCabClutchDrivelineOutput Output =
        SolveDrivelineStep(
            EngineRpmBeforeNative,
            ObservedFreeEngineNetTorqueNm,
            DeltaTime);
    ApplyEngineReaction(Engine, Output);

    const FDrivenWheelTorqueStats WheelStats =
        ApplyDrivenWheelTorque(Output);
    AccumulateEvidenceStep(
        Engine,
        Output,
        ObservedFreeEngineNetTorqueNm,
        WheelStats,
        DeltaTime);
    PublishMechanicalStep(DeltaTime);
}

void FPinkCabChaosWheeledVehicleSimulation::AdvanceAcceptedNativeEngine(
    Chaos::FSimpleTransmissionSim& Transmission,
    const float DeltaTime)
{
    // The native mechanical step must run even with ignition Off/Stalled:
    // Chaos advances wheel rotational state here, which is required for
    // gravity coast and shaft back-drive. Native transmission remains neutral,
    // so this cannot become a second propulsion path.
    Chaos::FSimpleEngineSim& Engine = PVehicle->GetEngine();
    const float PreservedEngineOmega = Engine.GetEngineOmega();

    Transmission.SetGear(0, true);
    UChaosWheeledVehicleSimulation::ProcessMechanicalSimulation(DeltaTime);
    Transmission.SetGear(0, true);

    if (!Command.bCombustionAllowed)
    {
        // Zero-throttle Chaos still maintains its own idle model. Ignition Off
        // must not receive that synthetic combustion state, so preserve the
        // pre-step engine omega and let the PinkCab clutch reaction below be
        // the only way the stopped engine can be mechanically back-driven.
        Engine.SetEngineOmega(PreservedEngineOmega);
    }
}

FPinkCabClutchDrivelineOutput
FPinkCabChaosWheeledVehicleSimulation::SolveDrivelineStep(
    const float EngineRpmBeforeNative,
    const float ObservedFreeEngineNetTorqueNm,
    const float DeltaTime)
{
    const float DrivenWheelRpm = MeanDrivenWheelRpm(*PVehicle);
    const float ShaftEquivalentEngineRpm =
        FMath::Abs(Command.EffectiveGearRatio) > KINDA_SMALL_NUMBER
            ? DrivenWheelRpm * FMath::Abs(Command.EffectiveGearRatio)
            : 0.0f;

    FPinkCabClutchDrivelineInput Input;
    Input.DeltaSeconds = DeltaTime;
    Input.EngineRpm =
        FMath::Max(EngineRpmBeforeNative, 0.0f);
    Input.ShaftEquivalentEngineRpm = ShaftEquivalentEngineRpm;
    if (Command.bCombustionAllowed)
    {
        // P01 native Chaos is the engine authority. Convert the actual free
        // engine angular-momentum change from this same physics step into the
        // net engine torque consumed by the clutch predictor. This avoids
        // duplicating/approximating the accepted engine torque/drag model.
        Input.AvailableEngineTorqueNm =
            FMath::Max(ObservedFreeEngineNetTorqueNm, 0.0f);
        Input.EngineDragTorqueNm =
            FMath::Max(-ObservedFreeEngineNetTorqueNm, 0.0f);
    }
    else
    {
        // Native combustion evolution is intentionally disabled when Off or
        // Stalled. Mechanical engine drag remains a P02 driveline load and can
        // still be back-driven through the same clutch path.
        Input.AvailableEngineTorqueNm = 0.0f;
        Input.EngineDragTorqueNm =
            FMath::Max(
                EngineRpmBeforeNative
                    * Command.EngineBrakeEffect,
                0.0f);
    }
    Input.ClutchCoupling01 = Command.ClutchCoupling01;
    Input.DrivetrainTorqueCapacity01 =
        Command.DrivetrainTorqueCapacity01;
    Input.EffectiveGearRatio = Command.EffectiveGearRatio;
    Input.TransmissionEfficiency = Command.TransmissionEfficiency;
    return ClutchModel.Step(Input);
}

void FPinkCabChaosWheeledVehicleSimulation::ApplyEngineReaction(
    Chaos::FSimpleEngineSim& Engine,
    const FPinkCabClutchDrivelineOutput& Output)
{
    const float EngineOmegaAfterReaction = FMath::Max(
        0.0f,
        Engine.GetEngineOmega()
            + Output.EngineReactionDeltaRpm
                * PinkCabChaosRpmToRadPerSecond);
    Engine.SetEngineOmega(EngineOmegaAfterReaction);
}

FPinkCabChaosWheeledVehicleSimulation::FDrivenWheelTorqueStats
FPinkCabChaosWheeledVehicleSimulation::ApplyDrivenWheelTorque(
    const FPinkCabClutchDrivelineOutput& Output)
{
    using namespace Chaos;

    FDrivenWheelTorqueStats Stats;
    const float InitialTransmittedClutchTorqueNm = FMath::Clamp(
        Output.RequestedClutchTorqueNm,
        -Output.TorqueCapacityNm,
        Output.TorqueCapacityNm);
    const float InitialRearAxleTorqueNm =
        InitialTransmittedClutchTorqueNm
        * Command.EffectiveGearRatio
        * FMath::Clamp(Command.TransmissionEfficiency, 0.0f, 1.0f);

    for (int32 WheelIndex = 0;
         WheelIndex < PVehicle->Wheels.Num();
         ++WheelIndex)
    {
        FSimpleWheelSim& Wheel = PVehicle->Wheels[WheelIndex];
        const bool bDriven = Wheel.Setup().EngineEnabled;
        const float WheelDriveTorqueNm =
            bDriven
                ? Output.RearAxleTorqueNm * Wheel.Setup().TorqueRatio
                : 0.0f;
        const float InitialWheelDriveTorqueNm =
            bDriven
                ? InitialRearAxleTorqueNm * Wheel.Setup().TorqueRatio
                : 0.0f;
        Wheel.SetDriveTorque(TorqueMToCm(WheelDriveTorqueNm));
        if (bDriven)
        {
            Stats.AbsTorqueSumNm += FMath::Abs(WheelDriveTorqueNm);
            Stats.InitialAbsTorqueSumNm +=
                FMath::Abs(InitialWheelDriveTorqueNm);
            Stats.SignedTorqueSumNm += WheelDriveTorqueNm;
            Stats.AbsWheelRpmSum += FMath::Abs(Wheel.GetWheelRPM());
            ++Stats.DrivenWheelCount;
        }
    }
    return Stats;
}

void FPinkCabChaosWheeledVehicleSimulation::PublishMechanicalStep(
    const float DeltaTime)
{
    if (MechanicalIntegrationDeltaMicros)
    {
        MechanicalIntegrationDeltaMicros->Set(
            FMath::RoundToInt64(
                FMath::Max(DeltaTime, 0.0f) * 1000000.0));
    }
    if (MechanicalIntegrationStepCounter)
    {
        MechanicalIntegrationStepCounter->Increment();
    }
}
