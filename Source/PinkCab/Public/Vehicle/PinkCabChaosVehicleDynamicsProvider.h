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

    UChaosWheeledVehicleMovementComponent* GetMovement() const { return Movement; }

private:
    UChaosWheeledVehicleMovementComponent* Movement = nullptr;
    FPinkCabVehicleControlState LastControls;
};
