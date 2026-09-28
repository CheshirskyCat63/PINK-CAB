#include "Vehicle/PinkCabChaosVehicleMovementComponent.h"

#include "ChaosVehicleManagerAsyncCallback.h"
#include "EngineSystem.h"
#include "Physics/PhysicsInterfaceCore.h"
#include "SimpleVehicle.h"
#include "VehicleUtility.h"
#include "WheelSystem.h"
#include "Vehicle/PinkCabEngineActuationResolver.h"

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

class FPinkCabChaosWheeledVehicleSimulation final
    : public UChaosWheeledVehicleSimulation
{
public:
    explicit FPinkCabChaosWheeledVehicleSimulation(
        FThreadSafeCounter64* InMechanicalIntegrationStepCounter,
        FThreadSafeCounter64* InMechanicalIntegrationDeltaMicros)
        : MechanicalIntegrationStepCounter(InMechanicalIntegrationStepCounter)
        , MechanicalIntegrationDeltaMicros(
            InMechanicalIntegrationDeltaMicros)
    {
    }

    void SetDrivelineCommand(const FPinkCabChaosDrivelineCommand& InCommand)
    {
        Command = InCommand;
        ClutchModel.SetConfig(Command.ClutchConfig);
    }

    void BeginEvidenceWindow(
        const int32 InSettleSteps,
        const int32 InSampleSteps)
    {
        EvidenceSettleStepsRemaining = FMath::Max(InSettleSteps, 0);
        EvidenceTargetSteps = FMath::Max(InSampleSteps, 1);
        EvidenceCompletedSteps = 0;
        EvidenceDrivenWheelTorqueSumNm = 0.0;
        EvidenceEngineRpmSum = 0.0;
        EvidenceDeltaSecondsSum = 0.0;
    }

    FPinkCabMechanicalEvidenceSnapshot ReadEvidenceWindow() const
    {
        FPinkCabMechanicalEvidenceSnapshot Result;
        Result.TargetSampleSteps = EvidenceTargetSteps;
        Result.CompletedSampleSteps = EvidenceCompletedSteps;
        if (EvidenceCompletedSteps > 0)
        {
            const double Denominator =
                static_cast<double>(EvidenceCompletedSteps);
            Result.MeanDrivenWheelTorqueNm = static_cast<float>(
                EvidenceDrivenWheelTorqueSumNm / Denominator);
            Result.MeanEngineRpm = static_cast<float>(
                EvidenceEngineRpmSum / Denominator);
            Result.MeanDeltaSeconds = static_cast<float>(
                EvidenceDeltaSecondsSum / Denominator);
        }
        Result.bComplete =
            EvidenceTargetSteps > 0
            && EvidenceCompletedSteps >= EvidenceTargetSteps;
        return Result;
    }

    virtual void ApplyInput(
        const FControlInputs& ControlInputs,
        const float DeltaTime) override
    {
        using namespace Chaos;

        // Preserve stock wheeled input semantics (including Ackermann steering
        // and input-rate bookkeeping), then overwrite only the torque channels
        // that P02 owns. This avoids rebuilding unrelated Chaos behavior.
        UChaosWheeledVehicleSimulation::ApplyInput(ControlInputs, DeltaTime);
        if (!PVehicle)
        {
            return;
        }

        PhysicsThreadActuation = {};
        if (PVehicle->HasEngine())
        {
            FSimpleEngineSim& Engine = PVehicle->GetEngine();
            const float EngineRpm = Engine.GetEngineRPM();

            FPinkCabEngineActuationInput ActuationInput;
            ActuationInput.bCombustionAllowed =
                Command.bCombustionAllowed;
            ActuationInput.HealthClampedControlThrottle01 =
                Command.HealthClampedControlThrottle01;
            ActuationInput.EngineRpm = EngineRpm;
            ActuationInput.MaxRpm = Engine.Setup().MaxRPM;
            ActuationInput.EngineTorqueCurveNm =
                Engine.GetTorqueFromRPM(EngineRpm, false);
            PhysicsThreadActuation =
                FPinkCabEngineActuationResolver::Resolve(
                    ActuationInput);

            Engine.SetThrottle(
                PhysicsThreadActuation.EngineThrottleFinal01);
        }

        for (int32 WheelIndex = 0; WheelIndex < PVehicle->Wheels.Num(); ++WheelIndex)
        {
            FSimpleWheelSim& Wheel = PVehicle->Wheels[WheelIndex];
            const FSimpleWheelConfig& Setup = Wheel.Setup();

            float BrakeTorqueNm = 0.0f;
            if (Setup.BrakeEnabled)
            {
                BrakeTorqueNm = Setup.MaxBrakeTorque
                    * FMath::Clamp(ControlInputs.BrakeInput, 0.0f, 1.0f);
            }

            if ((Command.Handbrake01 > KINDA_SMALL_NUMBER && Setup.HandbrakeEnabled)
                || ControlInputs.ParkingEnabled)
            {
                const float HandbrakeTorqueNm =
                    ControlInputs.ParkingEnabled
                        ? Setup.HandbrakeTorque
                        : Setup.HandbrakeTorque
                            * FMath::Clamp(Command.Handbrake01, 0.0f, 1.0f);
                BrakeTorqueNm = FMath::Max(BrakeTorqueNm, HandbrakeTorqueNm);
            }

            Wheel.SetBrakeTorque(
                TorqueMToCm(BrakeTorqueNm),
                false);
        }
    }

    virtual void ProcessMechanicalSimulation(const float DeltaTime) override
    {
        using namespace Chaos;

        if (!PVehicle || !PVehicle->HasEngine() || !PVehicle->HasTransmission())
        {
            return;
        }

        FSimpleEngineSim& Engine = PVehicle->GetEngine();
        FSimpleTransmissionSim& Transmission = PVehicle->GetTransmission();

        // The native simple transmission is kept neutral permanently. It remains
        // present for profile/output compatibility but never becomes a second
        // propulsion solver.
        Transmission.SetGear(0, true);

        if (Command.bCombustionAllowed)
        {
            // Match the accepted/native free-running engine path. The first
            // argument is FreeRunningIn: true keeps engine RPM independent of
            // wheel RPM. PINK CAB then applies clutch load explicitly below.
            Engine.SetEngineRPM(true, 0.0f);
            Engine.Simulate(DeltaTime);
        }

        const float DrivenWheelRpm = MeanDrivenWheelRpm(*PVehicle);
        const float ShaftEquivalentEngineRpm =
            FMath::Abs(Command.EffectiveGearRatio) > KINDA_SMALL_NUMBER
                ? DrivenWheelRpm * FMath::Abs(Command.EffectiveGearRatio)
                : 0.0f;

        FPinkCabClutchDrivelineInput Input;
        Input.DeltaSeconds = DeltaTime;
        Input.EngineRpm = FMath::Max(Engine.GetEngineRPM(), 0.0f);
        Input.ShaftEquivalentEngineRpm = ShaftEquivalentEngineRpm;
        Input.AvailableEngineTorqueNm =
            Command.bCombustionAllowed
                ? FMath::Max(
                    PhysicsThreadActuation
                        .RequestedEngineTorqueAfterLimiterHealthNm,
                    0.0f)
                : 0.0f;
        Input.EngineDragTorqueNm =
            FMath::Max(Input.EngineRpm * Command.EngineBrakeEffect, 0.0f);
        Input.ClutchCoupling01 = Command.ClutchCoupling01;
        Input.DrivetrainTorqueCapacity01 =
            Command.DrivetrainTorqueCapacity01;
        Input.EffectiveGearRatio = Command.EffectiveGearRatio;
        Input.TransmissionEfficiency = Command.TransmissionEfficiency;

        const FPinkCabClutchDrivelineOutput Output =
            ClutchModel.Step(Input);

        const float EngineOmegaAfterReaction = FMath::Max(
            0.0f,
            Engine.GetEngineOmega()
                + Output.EngineReactionDeltaRpm * PinkCabChaosRpmToRadPerSecond);
        Engine.SetEngineOmega(EngineOmegaAfterReaction);

        float DrivenWheelTorqueAbsSumNm = 0.0f;
        int32 DrivenWheelCount = 0;
        for (int32 WheelIndex = 0; WheelIndex < PVehicle->Wheels.Num(); ++WheelIndex)
        {
            FSimpleWheelSim& Wheel = PVehicle->Wheels[WheelIndex];
            const bool bDriven = Wheel.Setup().EngineEnabled;
            const float WheelDriveTorqueNm =
                bDriven
                    ? Output.RearAxleTorqueNm * Wheel.Setup().TorqueRatio
                    : 0.0f;
            Wheel.SetDriveTorque(TorqueMToCm(WheelDriveTorqueNm));
            if (bDriven)
            {
                DrivenWheelTorqueAbsSumNm +=
                    FMath::Abs(WheelDriveTorqueNm);
                ++DrivenWheelCount;
            }
        }

        if (EvidenceTargetSteps > 0
            && EvidenceCompletedSteps < EvidenceTargetSteps)
        {
            if (EvidenceSettleStepsRemaining > 0)
            {
                --EvidenceSettleStepsRemaining;
            }
            else
            {
                const float MeanDrivenWheelTorqueNm =
                    DrivenWheelCount > 0
                        ? DrivenWheelTorqueAbsSumNm
                            / static_cast<float>(DrivenWheelCount)
                        : 0.0f;
                EvidenceDrivenWheelTorqueSumNm +=
                    static_cast<double>(MeanDrivenWheelTorqueNm);
                EvidenceEngineRpmSum +=
                    static_cast<double>(Engine.GetEngineRPM());
                EvidenceDeltaSecondsSum +=
                    static_cast<double>(DeltaTime);
                ++EvidenceCompletedSteps;
            }
        }

        // Publish timing/state only after all engine/clutch/wheel work for this
        // ProcessMechanicalSimulation() invocation has completed. D3/D5 use
        // this read-only telemetry to prove the actual physics cadence rather
        // than assuming one automation callback equals one fixed simulation dt.
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

private:
    FThreadSafeCounter64* MechanicalIntegrationStepCounter = nullptr;
    FThreadSafeCounter64* MechanicalIntegrationDeltaMicros = nullptr;
    FPinkCabChaosDrivelineCommand Command;
    FPinkCabEngineActuationResult PhysicsThreadActuation;
    FPinkCabClutchDrivelineModel ClutchModel;
    int32 EvidenceSettleStepsRemaining = 0;
    int32 EvidenceTargetSteps = 0;
    int32 EvidenceCompletedSteps = 0;
    double EvidenceDrivenWheelTorqueSumNm = 0.0;
    double EvidenceEngineRpmSum = 0.0;
    double EvidenceDeltaSecondsSum = 0.0;
};

UPinkCabChaosVehicleMovementComponent::UPinkCabChaosVehicleMovementComponent(
    const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}

void UPinkCabChaosVehicleMovementComponent::ConfigurePinkCabClutch(
    const FPinkCabClutchDrivelineConfig& InConfig)
{
    ClutchConfig = InConfig;
    PendingDrivelineCommand.ClutchConfig = ClutchConfig;

    FBodyInstance* Body = GetBodyInstance();
    if (!Body || !PinkCabSimulationPT)
    {
        return;
    }

    FPhysicsCommand::ExecuteWrite(
        Body->ActorHandle,
        [this](const FPhysicsActorHandle&)
        {
            if (PinkCabSimulationPT)
            {
                PinkCabSimulationPT->SetDrivelineCommand(PendingDrivelineCommand);
            }
        });
}

bool UPinkCabChaosVehicleMovementComponent::BeginPinkCabMechanicalEvidenceWindow(
    const int32 SettleSteps,
    const int32 SampleSteps)
{
    FBodyInstance* Body = GetBodyInstance();
    if (!Body || !PinkCabSimulationPT)
    {
        return false;
    }

    return FPhysicsCommand::ExecuteWrite(
        Body->ActorHandle,
        [this, SettleSteps, SampleSteps](const FPhysicsActorHandle&)
        {
            if (PinkCabSimulationPT)
            {
                PinkCabSimulationPT->BeginEvidenceWindow(
                    SettleSteps,
                    SampleSteps);
            }
        });
}

bool UPinkCabChaosVehicleMovementComponent::ReadPinkCabMechanicalEvidenceWindow(
    FPinkCabMechanicalEvidenceSnapshot& OutSnapshot)
{
    FBodyInstance* Body = GetBodyInstance();
    if (!Body || !PinkCabSimulationPT)
    {
        return false;
    }

    return FPhysicsCommand::ExecuteWrite(
        Body->ActorHandle,
        [this, &OutSnapshot](const FPhysicsActorHandle&)
        {
            if (PinkCabSimulationPT)
            {
                OutSnapshot =
                    PinkCabSimulationPT->ReadEvidenceWindow();
            }
        });
}

TUniquePtr<Chaos::FSimpleWheeledVehicle>
UPinkCabChaosVehicleMovementComponent::CreatePhysicsVehicle()
{
    TUniquePtr<FPinkCabChaosWheeledVehicleSimulation> Simulation =
        MakeUnique<FPinkCabChaosWheeledVehicleSimulation>(
            &MechanicalIntegrationStepCounter,
            &MechanicalIntegrationDeltaMicros);
    PinkCabSimulationPT = Simulation.Get();
    PinkCabSimulationPT->SetDrivelineCommand(PendingDrivelineCommand);
    VehicleSimulationPT = MoveTemp(Simulation);

    return UChaosVehicleMovementComponent::CreatePhysicsVehicle();
}

bool UPinkCabChaosVehicleMovementComponent::SetPinkCabDrivelineCommand(
    const FPinkCabChaosDrivelineCommand& InCommand)
{
    PendingDrivelineCommand = InCommand;
    PendingDrivelineCommand.ClutchConfig = ClutchConfig;
    const FPinkCabChaosDrivelineCommand Command =
        PendingDrivelineCommand;

    FBodyInstance* Body = GetBodyInstance();
    if (!Body || !PinkCabSimulationPT)
    {
        // The command remains authoritative and will be installed when the
        // physics representation is created. Pre-physics calls are valid.
        return true;
    }

    return FPhysicsCommand::ExecuteWrite(
        Body->ActorHandle,
        [this, Command](const FPhysicsActorHandle&)
        {
            if (PinkCabSimulationPT)
            {
                PinkCabSimulationPT->SetDrivelineCommand(Command);
            }
        });
}
