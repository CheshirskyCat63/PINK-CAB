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

private:
    void ApplyPinkCabMassProperties(FBodyInstance* Body);
    FPinkCabVehicleMassProperties MassProperties;
    FDelegateHandle MassRecalculationHandle;
    FPinkCabClutchDrivelineConfig ClutchConfig;
    FPinkCabEngineRpmEnvelope EngineRpmEnvelope;
};
