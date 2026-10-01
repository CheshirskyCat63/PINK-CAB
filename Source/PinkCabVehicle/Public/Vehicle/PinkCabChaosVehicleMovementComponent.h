#pragma once

#include "CoreMinimal.h"
#include "HAL/ThreadSafeCounter64.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "Vehicle/PinkCabClutchDrivelineModel.h"
#include "Vehicle/PinkCabEngineRpmEnvelope.h"
#include "PinkCabChaosVehicleMovementComponent.generated.h"

class FPinkCabChaosWheeledVehicleSimulation;

struct PINKCABVEHICLE_API FPinkCabMechanicalEvidenceSnapshot
{
    int32 CompletedSampleSteps = 0;
    float TargetSampleSeconds = 0.0f;
    float CompletedSampleSeconds = 0.0f;
    float MeanDrivenWheelTorqueNm = 0.0f;
    float MeanInitialDrivenWheelTorqueNm = 0.0f;
    float MeanSignedDrivenWheelTorqueNm = 0.0f;
    float MeanDrivenWheelRpm = 0.0f;
    float MeanEngineRpm = 0.0f;
    float MeanObservedFreeEngineNetTorqueNm = 0.0f;
    float MeanRequestedClutchTorqueNm = 0.0f;
    float MeanTransmittedClutchTorqueNm = 0.0f;
    float MeanClutchSlipRpm = 0.0f;
    float MeanAppliedWheelBrakeTorqueNm = 0.0f;
    float MeanDeltaSeconds = 0.0f;
    bool bAnyParkingEnabled = false;
    bool bComplete = false;
};

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
    // Service brake is already prepared/smoothed by the PinkCab control
    // runtime. Carry that authoritative demand to the physics thread so native
    // Chaos input interpolation cannot become a second brake-state owner.
    float ServiceBrake01 = 0.0f;
    float EffectiveGearRatio = 0.0f;
    float TransmissionEfficiency = 1.0f;
    float EngineBrakeEffect = 0.0f;
    float Handbrake01 = 0.0f;
    FPinkCabEngineRpmEnvelope EngineRpmEnvelope;
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
    void ConfigurePinkCabEngineRpmEnvelope(
        const FPinkCabEngineRpmEnvelope& InEnvelope);
    const FPinkCabEngineRpmEnvelope& GetPinkCabEngineRpmEnvelope() const
    {
        return EngineRpmEnvelope;
    }
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

    float GetPinkCabLastMechanicalIntegrationDeltaSeconds() const
    {
        return static_cast<float>(
            MechanicalIntegrationDeltaMicros.GetValue())
            / 1000000.0f;
    }

    bool BeginPinkCabMechanicalEvidenceWindow(
        float SettleSeconds,
        float SampleSeconds);
    bool ReadPinkCabMechanicalEvidenceWindow(
        FPinkCabMechanicalEvidenceSnapshot& OutSnapshot);


protected:
    virtual TUniquePtr<Chaos::FSimpleWheeledVehicle> CreatePhysicsVehicle() override;

private:
    FThreadSafeCounter64 MechanicalIntegrationStepCounter;
    FThreadSafeCounter64 MechanicalIntegrationDeltaMicros;
    FPinkCabClutchDrivelineConfig ClutchConfig;
    FPinkCabEngineRpmEnvelope EngineRpmEnvelope;
    FPinkCabChaosDrivelineCommand PendingDrivelineCommand;
    FPinkCabChaosWheeledVehicleSimulation* PinkCabSimulationPT = nullptr;
};
