#pragma once

#include "CoreMinimal.h"
#include "Cockpit/PinkCabPrototypeVisualProfile.h"
#include "Vehicle/PinkCabChaosVehicleDynamicsProvider.h"
#include "Vehicle/PinkCabCockpitState.h"
#include "Interaction/PinkCabSemanticInputRouter.h"
#include "WheeledVehiclePawn.h"
#include "PinkCabChaosTatraPawn.generated.h"

class UChaosWheeledVehicleMovementComponent;
class UCameraComponent;
class USceneComponent;
class USkeletalMeshComponent;
class USpringArmComponent;
class UPinkCabCockpitAssemblyComponent;
class UPinkCabCockpitInteractionComponent;
class UPinkCabCockpitVisualDriverComponent;
struct FPinkCabInteractionEvent;
struct FPinkCabVehicleInputFrame;
class FPinkCabTaximeter;

UCLASS()
class PINKCAB_API APinkCabChaosTatraPawn : public AWheeledVehiclePawn
{
    GENERATED_BODY()

public:
    APinkCabChaosTatraPawn();

    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void Tick(float DeltaSeconds) override;
    UChaosWheeledVehicleMovementComponent* GetChaosMovement() const;
    IPinkCabVehicleDynamicsProvider& GetPinkCabDynamicsProvider() { return DynamicsProvider; }
    const FPinkCabCockpitState& GetCockpitState() const { return CockpitState; }
    bool ApplyCockpitInteraction(const FPinkCabInteractionEvent& Event);

    UPinkCabCockpitAssemblyComponent* GetCockpitAssembly() const { return CockpitAssembly; }
    UPinkCabCockpitInteractionComponent* GetCockpitInteraction() const { return CockpitInteraction; }
    UPinkCabCockpitVisualDriverComponent* GetCockpitVisualDriver() const { return CockpitVisualDriver; }
    FName GetPrototypeVisualProfileId() const { return PrototypeVisualProfile.ProfileId; }
    USkeletalMeshComponent* GetPrototypeDriverVisual() const { return PrototypeDriverVisual; }
    UCameraComponent* GetDriverCamera() const { return DriverCamera; }

    static float IntegrateMouseSteering(
        float CurrentSteering,
        float DeltaX,
        bool bGazeHeld,
        float Gain = 0.025f);
    static float SteeringGainForSpeed(float SpeedKmh);

    void ApplyMouseSteeringDelta(float DeltaX, bool bGazeHeld);
    void ApplyVehicleInputFrame(const FPinkCabVehicleInputFrame& InputFrame, float MouseDeltaX);
    float GetSteeringCommand() const { return SteeringCommand; }
    void ResetTransientCockpitInput();

    void SetCockpitTaximeterSource(const FPinkCabTaximeter* InTaximeter) { CockpitTaximeterSource = InTaximeter; }
    void SetCockpitRouteProgress(TOptional<float> InRouteProgress01) { CockpitRouteProgress01 = InRouteProgress01; }
    void SetCockpitRadioAvailable(bool bAvailable) { bCockpitRadioAvailable = bAvailable; }
    void SetCockpitMirrorsAvailable(bool bAvailable) { bCockpitMirrorsAvailable = bAvailable; }

private:
    UPROPERTY(VisibleAnywhere, Category = "PinkCab|Cockpit")
    TObjectPtr<UPinkCabCockpitAssemblyComponent> CockpitAssembly;

    UPROPERTY(VisibleAnywhere, Category = "PinkCab|Cockpit")
    TObjectPtr<UPinkCabCockpitInteractionComponent> CockpitInteraction;

    UPROPERTY(VisibleAnywhere, Category = "PinkCab|Cockpit")
    TObjectPtr<UPinkCabCockpitVisualDriverComponent> CockpitVisualDriver;

    UPROPERTY(VisibleAnywhere, Category = "PinkCab|PrototypeVisual")
    TObjectPtr<USkeletalMeshComponent> PrototypeDriverVisual;

    UPROPERTY(VisibleAnywhere, Category = "PinkCab|Cockpit")
    TObjectPtr<USceneComponent> DriverHeadRoot;

    UPROPERTY(VisibleAnywhere, Category = "PinkCab|Cockpit")
    TObjectPtr<UCameraComponent> DriverCamera;

    UPROPERTY(VisibleAnywhere, Category = "PinkCab|Camera")
    TObjectPtr<USpringArmComponent> CameraBoom;

    UPROPERTY(VisibleAnywhere, Category = "PinkCab|Camera")
    TObjectPtr<UCameraComponent> ChaseCamera;

    void SyncCockpitToChaos();
    void HandleApplicationWillDeactivate();

    FPinkCabPrototypeVisualProfile PrototypeVisualProfile =
        FPinkCabPrototypeVisualProfile::EpicSportsCarManny();
    FPinkCabChaosVehicleDynamicsProvider DynamicsProvider;
    FPinkCabVehicleControlState ControlState;
    FPinkCabCockpitState CockpitState;
    FPinkCabSemanticInputRouter InputRouter = FPinkCabSemanticInputRouter::CreateDefaults();
    const FPinkCabTaximeter* CockpitTaximeterSource = nullptr;
    TOptional<float> CockpitRouteProgress01;
    bool bCockpitRadioAvailable = false;
    bool bCockpitMirrorsAvailable = true;

    UPROPERTY(EditAnywhere, Category = "PinkCab|Input|Pedals", meta=(ClampMin="0.05"))
    float ClutchPressSeconds = 0.16f;
    UPROPERTY(EditAnywhere, Category = "PinkCab|Input|Pedals", meta=(ClampMin="0.05"))
    float BrakePressSeconds = 0.20f;
    UPROPERTY(EditAnywhere, Category = "PinkCab|Input|Pedals", meta=(ClampMin="0.05"))
    float BrakeReleaseSeconds = 0.28f;
    UPROPERTY(EditAnywhere, Category = "PinkCab|Input|Pedals", meta=(ClampMin="0.05"))
    float ThrottlePressSeconds = 0.35f;
    UPROPERTY(EditAnywhere, Category = "PinkCab|Input|Pedals", meta=(ClampMin="0.05"))
    float ThrottleReleaseSeconds = 0.22f;

    float SmoothedClutch = 0.0f;
    float SmoothedBrake = 0.0f;
    float SmoothedThrottle = 0.0f;
    float SteeringCommand = 0.0f;
    float MouseSteeringGain = 0.010f;
    float LookYaw = 0.0f;
    float LookPitch = 0.0f;
    FDelegateHandle ApplicationWillDeactivateHandle;
};
