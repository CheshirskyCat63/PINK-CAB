#pragma once
#include "ChaosWheeledVehicleMovementComponent.h"

// One native simulation instance. Only the engine-to-wheel connection differs;
// wheel friction, suspension, steering and chassis integration stay inherited.
class FPinkCabChaosDrivelineSimulation final : public UChaosWheeledVehicleSimulation
{
public:
    virtual void ApplyInput(const FControlInputs& Inputs, float DeltaTime) override;
};
