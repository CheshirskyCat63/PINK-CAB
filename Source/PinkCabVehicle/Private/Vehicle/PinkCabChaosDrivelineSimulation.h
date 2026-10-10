#pragma once
#include "ChaosWheeledVehicleMovementComponent.h"
#include "PinkCabChaosCommandChannel.h"
#include "Vehicle/PinkCabChaosNativeClutchJoint.h"

class FPinkCabChaosDrivelineSimulation final : public UChaosWheeledVehicleSimulation
{
public:
    explicit FPinkCabChaosDrivelineSimulation(TSharedRef<FPinkCabChaosCommandChannel, ESPMode::ThreadSafe> InChannel)
        : Channel(MoveTemp(InChannel)) {}
    virtual void TickVehicle(UWorld* WorldIn, float DeltaTime,
        const FChaosVehicleAsyncInput& InputData, FChaosVehicleAsyncOutput& OutputData,
        Chaos::FRigidBodyHandle_Internal* Handle) override;
    virtual void ApplyInput(const FControlInputs& Inputs, float DeltaTime) override;
    virtual void ProcessMechanicalSimulation(float DeltaTime) override;
    virtual void FillOutputState(FChaosVehicleAsyncOutput& Output) override;
    virtual void ApplyWheelFrictionForces(float DeltaTime) override;
private:
    void ApplyNativeClutchAtWheelBoundary(float DeltaTime);
    bool bClutchStepPending = false;
    TSharedRef<FPinkCabChaosCommandChannel, ESPMode::ThreadSafe> Channel;
    FPinkCabChaosCommandFrame Frame;
    FPinkCabChaosNativeClutchJoint ClutchJoint;
    FPinkCabDrivelineStepTelemetry Step;
    uint64 PhysicsStep = 0;
};
