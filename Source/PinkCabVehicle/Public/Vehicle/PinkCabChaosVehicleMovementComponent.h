#pragma once

#include "CoreMinimal.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "Vehicle/PinkCabClutchDrivelineModel.h"
#include "Vehicle/PinkCabEngineRpmEnvelope.h"
#include "PinkCabChaosVehicleMovementComponent.generated.h"

// P4 uses stock Chaos mechanical simulation. This thin component only retains
// PINKCAB profile data so cockpit/gameplay code does not own vehicle physics.
UCLASS(ClassGroup=(Physics), meta=(BlueprintSpawnableComponent))
class PINKCABVEHICLE_API UPinkCabChaosVehicleMovementComponent final
    : public UChaosWheeledVehicleMovementComponent
{
    GENERATED_BODY()

public:
    explicit UPinkCabChaosVehicleMovementComponent(
        const FObjectInitializer& ObjectInitializer);

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

private:
    FPinkCabClutchDrivelineConfig ClutchConfig;
    FPinkCabEngineRpmEnvelope EngineRpmEnvelope;
};
