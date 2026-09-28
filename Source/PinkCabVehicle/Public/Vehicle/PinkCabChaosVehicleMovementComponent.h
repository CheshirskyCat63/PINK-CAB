#pragma once

#include "CoreMinimal.h"
#include "HAL/ThreadSafeCounter64.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "Vehicle/PinkCabClutchDrivelineModel.h"
#include "PinkCabChaosVehicleMovementComponent.generated.h"

class FPinkCabChaosWheeledVehicleSimulation;

struct PINKCABVEHICLE_API FPinkCabChaosDrivelineCommand
{
    bool bCombustionAllowed = false;
    int32 EngagedGear = 0;
    float ClutchCoupling01 = 0.0f;
    float DrivetrainTorqueCapacity01 = 1.0f;
    // Game-thread command carries only driver/health demand. RPM-sensitive
    // limiter and torque-curve resolution happen on the physics thread against
    // the exact engine state consumed by the same mechanical step.
    float HealthClampedControlThrottle01 = 0.0f;
    float EffectiveGearRatio = 0.0f;
    float TransmissionEfficiency = 1.0f;
    float EngineBrakeEffect = 0.0f;
    float Handbrake01 = 0.0f;
    FPinkCabClutchDrivelineConfig ClutchConfig;
};

UCLASS(ClassGroup=(Physics), meta=(BlueprintSpawnableComponent))
class PINKCABVEHICLE_API UPinkCabChaosVehicleMovementComponent final
    : public UChaosWheeledVehicleMovementComponent
{
    GENERATED_BODY()

public:
    explicit UPinkCabChaosVehicleMovementComponent(
        const FObjectInitializer& ObjectInitializer);

    void ConfigurePinkCabClutch(
        const FPinkCabClutchDrivelineConfig& InConfig);
    const FPinkCabClutchDrivelineConfig& GetPinkCabClutchConfig() const
    {
        return ClutchConfig;
    }

    bool SetPinkCabDrivelineCommand(
        const FPinkCabChaosDrivelineCommand& InCommand);

    const FPinkCabChaosDrivelineCommand& GetPendingPinkCabDrivelineCommand() const
    {
        return PendingDrivelineCommand;
    }

    int64 GetPinkCabMechanicalIntegrationStepCount() const
    {
        return MechanicalIntegrationStepCounter.GetValue();
    }

protected:
    virtual TUniquePtr<Chaos::FSimpleWheeledVehicle> CreatePhysicsVehicle() override;

private:
    FThreadSafeCounter64 MechanicalIntegrationStepCounter;
    FPinkCabClutchDrivelineConfig ClutchConfig;
    FPinkCabChaosDrivelineCommand PendingDrivelineCommand;
    FPinkCabChaosWheeledVehicleSimulation* PinkCabSimulationPT = nullptr;
};
