#include "PinkCabChaosDrivelineSimulation.h"
#include "SimpleVehicle.h"

void FPinkCabChaosDrivelineSimulation::TickVehicle(UWorld* WorldIn, float DeltaTime,
    const FChaosVehicleAsyncInput& InputData, FChaosVehicleAsyncOutput& OutputData,
    Chaos::FRigidBodyHandle_Internal* Handle)
{
    // One coherent local command for this entire physics step. Chassis/trace data
    // stays in the native packet. Never pair a new clutch with an older throttle.
    Frame = Channel->ReadCommand();
    FChaosVehicleAsyncInput EffectiveInput(InputData);
    if (Frame.Sequence != 0) EffectiveInput.PhysicsInputs.NetworkInputs = Frame.Native;
    Step = {};
    Step.CommandSequence = Frame.Sequence;
    Step.PhysicsStep = ++PhysicsStep;
    UChaosWheeledVehicleSimulation::TickVehicle(WorldIn, DeltaTime, EffectiveInput, OutputData, Handle);
}

void FPinkCabChaosDrivelineSimulation::ApplyInput(const FControlInputs& Inputs, float DeltaTime)
{
    if (!PVehicle) return;
    // Engine resistance now reaches the wheels through the same native joint
    // as combustion. The stock direct-wheel engine brake would bypass it.
    TArray<bool, TInlineAllocator<8>> Driven;
    Driven.Reserve(PVehicle->Wheels.Num());
    for (auto& Wheel : PVehicle->Wheels)
    {
        Driven.Add(Wheel.EngineEnabled);
        Wheel.EngineEnabled = false;
    }
    UChaosWheeledVehicleSimulation::ApplyInput(Inputs, DeltaTime);
    for (int32 Index = 0; Index < PVehicle->Wheels.Num(); ++Index)
        PVehicle->Wheels[Index].EngineEnabled = Driven[Index];
    const auto& Prepared = Frame.Native.VehicleInputs;
    Step.bCommandMatchesPreparedFrame = Inputs.SteeringInput == Prepared.SteeringInput
        && Inputs.ThrottleInput == Prepared.ThrottleInput && Inputs.BrakeInput == Prepared.BrakeInput
        && Inputs.HandbrakeInput == Prepared.HandbrakeInput && Inputs.TransmissionType == Prepared.TransmissionType;
    Step.Steering = Inputs.SteeringInput;
    Step.Throttle = Inputs.ThrottleInput;
    Step.Brake = Inputs.BrakeInput;
    Step.Handbrake = Inputs.HandbrakeInput;
    Step.Coupling = Frame.Controls.ClutchCoupling;
    Step.Gear = Frame.Controls.EngagedGear;
    Step.bCombustionAllowed = Frame.Controls.IsCombustionAllowed();
}

void FPinkCabChaosDrivelineSimulation::FillOutputState(FChaosVehicleAsyncOutput& Output)
{
    UChaosWheeledVehicleSimulation::FillOutputState(Output);
    if (PVehicle && PVehicle->HasEngine() && Frame.bValid)
        Output.VehicleSimOutput.EngineRPM = Chaos::OmegaToRPM(PVehicle->GetEngine().GetEngineOmega());
    Step.NativeGear = Output.VehicleSimOutput.CurrentGear;
    if (PVehicle)
    {
        Step.NativeWheelCount = PVehicle->Wheels.Num();
        for (int32 Index = 0; Index < FMath::Min(Step.NativeWheelCount, 4); ++Index)
        {
            Step.NativeWheelDriveNm[Index] = Chaos::TorqueCmToM(PVehicle->Wheels[Index].GetDriveTorque());
            Step.NativeWheelOmegaRad[Index] = PVehicle->Wheels[Index].GetAngularVelocity();
        }
    }
    Channel->PublishFeedback(Step);
}
