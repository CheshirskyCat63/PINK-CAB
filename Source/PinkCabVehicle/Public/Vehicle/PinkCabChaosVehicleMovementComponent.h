#pragma once

#include "CoreMinimal.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "Vehicle/PinkCabClutchDrivelineModel.h"
#include "Vehicle/PinkCabEngineRpmEnvelope.h"
#include "Vehicle/PinkCabVehicleMassProperties.h"
#include "PinkCabChaosVehicleMovementComponent.generated.h"

// Native Chaos owns mechanical simulation. This component applies versioned mass
// properties and retains profile data; it does not implement a second dynamics solver.
UCLASS(ClassGroup=(Physics), meta=(BlueprintSpawnableComponent))
class PINKCABVEHICLE_API UPinkCabChaosVehicleMovementComponent final
    : public UChaosWheeledVehicleMovementComponent
{
    GENERATED_BODY()

public:
    explicit UPinkCabChaosVehicleMovementComponent(
        const FObjectInitializer& ObjectInitializer);

    // Native float handbrake input; no boolean quantization or custom brake forces.
    void SetPinkCabHandbrakeInput(float Value);
    float GetPinkCabHandbrakeInput() const { return AnalogHandbrakeCommand; }

    bool ConfigurePinkCabMass(const FPinkCabVehicleMassProperties& InProperties);
    const FPinkCabVehicleMassProperties& GetPinkCabMassProperties() const { return MassProperties; }

    // Read-only native suspension result in chassis component space (cm).
    bool GetWheelPresentationCenter(int32 WheelIndex, FVector& OutCenter);

    void ConfigurePinkCabClutch(const FPinkCabClutchDrivelineConfig& InConfig)
    {
        ClutchConfig = InConfig;
    }

    void ConfigurePinkCabEngineRpmEnvelope(const FPinkCabEngineRpmEnvelope& InEnvelope)
    {
        EngineRpmEnvelope = InEnvelope;
    }

    const FPinkCabEngineRpmEnvelope& GetPinkCabEngineRpmEnvelope() const
    {
        return EngineRpmEnvelope;
    }

    const FPinkCabClutchDrivelineConfig& GetPinkCabClutchConfig() const
    {
        return ClutchConfig;
    }

protected:
    virtual void SetupVehicleMass() override;
    virtual void UpdateState(float DeltaTime) override;
    virtual void ProcessSleeping(const FControlInputs& Inputs) override;
    virtual void ClearRawInput() override;

private:
    void ApplyPinkCabMassProperties(FBodyInstance* Body);
    float AnalogHandbrakeCommand = 0.0f;
    bool bHandbrakeWakePending = false;
    FPinkCabVehicleMassProperties MassProperties;
    FDelegateHandle MassRecalculationHandle;
    FPinkCabClutchDrivelineConfig ClutchConfig;
    FPinkCabEngineRpmEnvelope EngineRpmEnvelope;
};
