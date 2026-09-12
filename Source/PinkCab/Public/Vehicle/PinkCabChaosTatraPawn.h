#pragma once

#include "CoreMinimal.h"
#include "WheeledVehiclePawn.h"
#include "PinkCabChaosTatraPawn.generated.h"

class UChaosWheeledVehicleMovementComponent;
class UCameraComponent;
class USpringArmComponent;

UCLASS()
class PINKCAB_API APinkCabChaosTatraPawn : public AWheeledVehiclePawn
{
    GENERATED_BODY()

public:
    APinkCabChaosTatraPawn();

    UChaosWheeledVehicleMovementComponent* GetChaosMovement() const;

private:
    UPROPERTY(VisibleAnywhere, Category = "PinkCab|Camera")
    TObjectPtr<USpringArmComponent> CameraBoom;

    UPROPERTY(VisibleAnywhere, Category = "PinkCab|Camera")
    TObjectPtr<UCameraComponent> ChaseCamera;
};
