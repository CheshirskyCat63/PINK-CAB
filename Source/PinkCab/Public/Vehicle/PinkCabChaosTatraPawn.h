#pragma once

#include "CoreMinimal.h"
#include "Vehicle/PinkCabChaosVehicleDynamicsProvider.h"
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

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;

    UChaosWheeledVehicleMovementComponent* GetChaosMovement() const;

    static float IntegrateMouseSteering(
        float CurrentSteering,
        float DeltaX,
        bool bGazeHeld,
        float Gain = 0.025f);

    void ApplyMouseSteeringDelta(float DeltaX, bool bGazeHeld);
    float GetSteeringCommand() const { return SteeringCommand; }

private:
    UPROPERTY(VisibleAnywhere, Category = "PinkCab|Camera")
    TObjectPtr<USpringArmComponent> CameraBoom;

    UPROPERTY(VisibleAnywhere, Category = "PinkCab|Camera")
    TObjectPtr<UCameraComponent> ChaseCamera;

    FPinkCabChaosVehicleDynamicsProvider DynamicsProvider;
    FPinkCabVehicleControlState ControlState;

    float SteeringCommand = 0.0f;
    float MouseSteeringGain = 0.025f;
    float LookYaw = 0.0f;
    float LookPitch = 0.0f;
};
