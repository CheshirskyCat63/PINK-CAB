#pragma once

#include "CoreMinimal.h"
#include "Vehicle/PinkCabChaosVehicleDynamicsProvider.h"
#include "Vehicle/PinkCabCockpitState.h"
#include "Interaction/PinkCabSemanticInputRouter.h"
#include "WheeledVehiclePawn.h"
#include "PinkCabChaosTatraPawn.generated.h"

class UChaosWheeledVehicleMovementComponent;
class UCameraComponent;
class USceneComponent;
class USpringArmComponent;
class UPinkCabCockpitAssemblyComponent;
class UPinkCabCockpitInteractionComponent;
struct FPinkCabInteractionEvent;

UCLASS()
class PINKCAB_API APinkCabChaosTatraPawn : public AWheeledVehiclePawn
{
    GENERATED_BODY()

public:
    APinkCabChaosTatraPawn();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;

    UChaosWheeledVehicleMovementComponent* GetChaosMovement() const;
    IPinkCabVehicleDynamicsProvider& GetPinkCabDynamicsProvider() { return DynamicsProvider; }
    const FPinkCabCockpitState& GetCockpitState() const { return CockpitState; }
    bool ApplyCockpitInteraction(const FPinkCabInteractionEvent& Event);

    UPinkCabCockpitAssemblyComponent* GetCockpitAssembly() const { return CockpitAssembly; }
    UPinkCabCockpitInteractionComponent* GetCockpitInteraction() const { return CockpitInteraction; }
    UCameraComponent* GetDriverCamera() const { return DriverCamera; }

    static float IntegrateMouseSteering(
        float CurrentSteering,
        float DeltaX,
        bool bGazeHeld,
        float Gain = 0.025f);

    void ApplyMouseSteeringDelta(float DeltaX, bool bGazeHeld);
    float GetSteeringCommand() const { return SteeringCommand; }

private:
    UPROPERTY(VisibleAnywhere, Category = "PinkCab|Cockpit")
    TObjectPtr<UPinkCabCockpitAssemblyComponent> CockpitAssembly;

    UPROPERTY(VisibleAnywhere, Category = "PinkCab|Cockpit")
    TObjectPtr<UPinkCabCockpitInteractionComponent> CockpitInteraction;

    UPROPERTY(VisibleAnywhere, Category = "PinkCab|Cockpit")
    TObjectPtr<USceneComponent> DriverHeadRoot;

    UPROPERTY(VisibleAnywhere, Category = "PinkCab|Cockpit")
    TObjectPtr<UCameraComponent> DriverCamera;

    UPROPERTY(VisibleAnywhere, Category = "PinkCab|Camera")
    TObjectPtr<USpringArmComponent> CameraBoom;

    UPROPERTY(VisibleAnywhere, Category = "PinkCab|Camera")
    TObjectPtr<UCameraComponent> ChaseCamera;

    void SyncCockpitToChaos();

    FPinkCabChaosVehicleDynamicsProvider DynamicsProvider;
    FPinkCabVehicleControlState ControlState;
    FPinkCabCockpitState CockpitState;
    FPinkCabSemanticInputRouter InputRouter = FPinkCabSemanticInputRouter::CreateDefaults();

    float SteeringCommand = 0.0f;
    float MouseSteeringGain = 0.025f;
    float LookYaw = 0.0f;
    float LookPitch = 0.0f;
};
