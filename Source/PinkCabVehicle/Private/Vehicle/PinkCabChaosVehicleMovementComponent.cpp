#include "Vehicle/PinkCabChaosVehicleMovementComponent.h"

#include "ChaosVehicleManagerAsyncCallback.h"
#include "EngineSystem.h"
#include "Physics/PhysicsInterfaceCore.h"
#include "SimpleVehicle.h"
#include "VehicleUtility.h"
#include "WheelSystem.h"

namespace
{
constexpr float RpmToRadPerSecond = 2.0f * PI / 60.0f;

float MeanDrivenWheelRpm(const Chaos::FSimpleWheeledVehicle& Vehicle)
{
    float Sum = 0.0f;
    int32 Count = 0;
    for (int32 Index = 0; Index < Vehicle.Wheels.Num(); ++Index)
    {
        const Chaos::FSimpleWheelSim& Wheel = Vehicle.Wheels[Index];
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
    void SetDrivelineCommand(const FPinkCabChaosDrivelineCommand& InCommand)
    {
        Command = InCommand;
        ClutchModel.SetConfig(Command.ClutchConfig);
    }

    virtual void ApplyInput(
        const FControlInputs& ControlInputs,
        const float DeltaTime) override
    {
        using namespace Chaos;

        // Preserve generic input bookkeeping/steering rates, but deliberately
        // bypass UChaosWheeledVehicleSimulation::ApplyInput because that path
        // injects engine braking directly at engine-enabled wheels. In PINK CAB
        // every engine/wheel torque exchange must pass through the clutch model.
        UChaosVehicleSimulation::ApplyInput(ControlInputs, DeltaTime);
        if (!PVehicle)
        {
            return;
        }

        if (PVehicle->HasEngine())
        {
            PVehicle->GetEngine().SetThrottle(
                Command.bCombustionAllowed
                    ? FMath::Clamp(Command.AuthoritativeEngineThrottle01, 0.0f, 1.0f)
                    : 0.0f);
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

            if ((ControlInputs.HandbrakeInput && Setup.HandbrakeEnabled)
                || ControlInputs.ParkingEnabled)
            {
                const float HandbrakeTorqueNm =
                    ControlInputs.ParkingEnabled
                        ? Setup.HandbrakeTorque
                        : Setup.HandbrakeTorque
                            * FMath::Clamp(ControlInputs.HandbrakeInput, 0.0f, 1.0f);
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
            // Preserve the accepted P01 free-running engine dynamics. The clutch
            // reaction is applied afterwards on the same physics step.
            Engine.SetEngineRPM(true, 0.0f);
            Engine.Simulate(DeltaTime);
        }
        else if (Command.EngagedGear == 0
            || Command.ClutchCoupling01 <= KINDA_SMALL_NUMBER)
        {
            // Ignition off/stalled with an open driveline has no combustion
            // source and therefore no synthetic idle.
            Engine.SetEngineOmega(0.0f);
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
                ? FMath::Max(Command.AvailableEngineTorqueNm, 0.0f)
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
                + Output.EngineReactionDeltaRpm * RpmToRadPerSecond);
        Engine.SetEngineOmega(EngineOmegaAfterReaction);

        for (int32 WheelIndex = 0; WheelIndex < PVehicle->Wheels.Num(); ++WheelIndex)
        {
            FSimpleWheelSim& Wheel = PVehicle->Wheels[WheelIndex];
            if (Wheel.Setup().EngineEnabled)
            {
                Wheel.SetDriveTorque(
                    TorqueMToCm(Output.RearAxleTorqueNm)
                    * Wheel.Setup().TorqueRatio);
            }
            else
            {
                Wheel.SetDriveTorque(0.0f);
            }
        }
    }

private:
    FPinkCabChaosDrivelineCommand Command;
    FPinkCabClutchDrivelineModel ClutchModel;
};

UPinkCabChaosVehicleMovementComponent::UPinkCabChaosVehicleMovementComponent(
    const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}

TUniquePtr<Chaos::FSimpleWheeledVehicle>
UPinkCabChaosVehicleMovementComponent::CreatePhysicsVehicle()
{
    TUniquePtr<FPinkCabChaosWheeledVehicleSimulation> Simulation =
        MakeUnique<FPinkCabChaosWheeledVehicleSimulation>();
    PinkCabSimulationPT = Simulation.Get();
    PinkCabSimulationPT->SetDrivelineCommand(PendingDrivelineCommand);
    VehicleSimulationPT = MoveTemp(Simulation);

    return UChaosVehicleMovementComponent::CreatePhysicsVehicle();
}

bool UPinkCabChaosVehicleMovementComponent::SetPinkCabDrivelineCommand(
    const FPinkCabChaosDrivelineCommand& InCommand)
{
    PendingDrivelineCommand = InCommand;

    FBodyInstance* Body = GetBodyInstance();
    if (!Body || !PinkCabSimulationPT)
    {
        return false;
    }

    return FPhysicsCommand::ExecuteWrite(
        Body->ActorHandle,
        [this, InCommand](const FPhysicsActorHandle&)
        {
            if (PinkCabSimulationPT)
            {
                PinkCabSimulationPT->SetDrivelineCommand(InCommand);
            }
        });
}
