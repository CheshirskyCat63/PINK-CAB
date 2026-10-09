#include "PinkCabChaosDrivelineSimulation.h"
#include "SimpleVehicle.h"

void FPinkCabChaosDrivelineSimulation::ApplyInput(const FControlInputs& Inputs, float DeltaTime)
{
    if (!PVehicle || !PVehicle->HasTransmission()
        || PVehicle->GetTransmission().GetCurrentGear() != 0)
    {
        UChaosWheeledVehicleSimulation::ApplyInput(Inputs, DeltaTime);
        return;
    }

    // Native ApplyInput also generates engine braking. In neutral the driven
    // axles are mechanically disconnected, but service and hand brakes remain.
    // Scope this per-instance eligibility to the native input calculation only;
    // never alter shared wheel configuration or the permanent driven-axle layout.
    TArray<bool, TInlineAllocator<8>> Driven;
    Driven.Reserve(PVehicle->Wheels.Num());
    for (auto& Wheel : PVehicle->Wheels)
    {
        Driven.Add(Wheel.EngineEnabled);
        Wheel.EngineEnabled = false;
    }
    UChaosWheeledVehicleSimulation::ApplyInput(Inputs, DeltaTime);
    for (int32 Index = 0; Index < PVehicle->Wheels.Num(); ++Index)
    {
        PVehicle->Wheels[Index].EngineEnabled = Driven[Index];
    }
}
