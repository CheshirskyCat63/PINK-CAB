#pragma once

#include "Vehicle/PinkCabVehicleDynamicsProvider.h"

class UChaosWheeledVehicleMovementComponent;

class PINKCAB_API FPinkCabChaosVehicleDynamicsProvider final : public IPinkCabVehicleDynamicsProvider
{
public:
    explicit FPinkCabChaosVehicleDynamicsProvider(
        UChaosWheeledVehicleMovementComponent* InMovement = nullptr);

    virtual bool ApplyControls(const FPinkCabVehicleControlState& Controls) override;
    virtual bool ReadTelemetry(FPinkCabVehicleTelemetry& OutTelemetry) const override;
    virtual EPinkCabMechanicalClutchCapability GetMechanicalClutchCapability() const override
    {
        return EPinkCabMechanicalClutchCapability::EmulatedNeutralGate;
    }

    UChaosWheeledVehicleMovementComponent* GetMovement() const { return Movement; }
    const FPinkCabVehicleControlState& GetLastControls() const { return LastControls; }

private:
    UChaosWheeledVehicleMovementComponent* Movement = nullptr;
    FPinkCabVehicleControlState LastControls;
};
