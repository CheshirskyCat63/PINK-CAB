#pragma once

#include "CoreMinimal.h"
#include "Cockpit/PinkCabPrototypeVisualProfile.h"
#include "Vehicle/PinkCabChaosVehicleDynamicsProvider.h"
#include "Vehicle/PinkCabCockpitState.h"
#include "Vehicle/PinkCabVehicleDamageProfile.h"
#include "Vehicle/PinkCabVehicleHealthService.h"
#include "Vehicle/PinkCabVehicleLoadState.h"
#include "Vehicle/PinkCabSteeringController.h"
#include "Vehicle/PinkCabVehicleMotionClassifier.h"
#include "Vehicle/PinkCabLaunchController.h"
#include "Vehicle/PinkCabPedalDosingController.h"
#include "Vehicle/PinkCabHandbrakeActuator.h"
#include "Vehicle/PinkCabGearboxController.h"
#include "Vehicle/PinkCabDrivetrainCondition.h"
#include "Vehicle/PinkCabVehicleHealthBinding.h"
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
class UPinkCabVehicleVisualShellComponent;
struct FPinkCabInteractionEvent;
struct FPinkCabVehicleInputFrame;
struct FPinkCabVehicleSnapshot;
struct FPinkCabVehicleVisualProfile;
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
    UPinkCabVehicleVisualShellComponent* GetVehicleVisualShell() const { return VehicleVisualShell; }
    FName GetVehicleVisualProfileId() const;
    bool ApplyVehicleVisualProfile(const FPinkCabVehicleVisualProfile& Profile);
    const FPinkCabVehicleLoadState& GetVehicleLoadState() const { return VehicleLoadState; }

    bool SetFuelMassKg(float MassKg, float LongitudinalCm = 0.0f);
    bool TrySetFarePassengerGroup(
        const FPinkCabStableId& GroupId,
        TConstArrayView<FPinkCabVehicleLoadItem> Items);
    bool RemoveFarePassengerGroup(const FPinkCabStableId& GroupId);
    bool SetVehicleDamageProfile(const FPinkCabVehicleDamageProfile& Profile);
    FName GetVehicleDamageProfileId() const { return VehicleDamageProfile.GetProfileId(); }
    bool ApplyAuthoredVehicleHit(FName ZoneId, float CollisionSeverity);
    bool ApplyVehicleHit(const FPinkCabVehicleHitEvent& Event);
    bool IsVehicleTerminal() const { return VehicleHealthService.IsTerminal(GetVehicleHealthState()); }
    bool CaptureVehicleSnapshot(FPinkCabVehicleSnapshot& OutSnapshot) const;
    bool RestoreVehicleSnapshot(const FPinkCabVehicleSnapshot& Snapshot);

    void ApplyMouseSteeringDelta(float DeltaX, bool bGazeHeld, float DeltaSeconds = 1.0f / 60.0f);
    void ApplyVehicleInputFrame(const FPinkCabVehicleInputFrame& InputFrame, float MouseDeltaX, float DeltaSeconds = 1.0f / 60.0f);
    void ApplyPhysicalControlMouseDelta(
        FName TargetId,
        bool bGripHeld,
        float MouseDeltaX,
        float MouseDeltaY,
        float DeltaSeconds = 1.0f / 60.0f);
    float GetSteeringCommand() const { return SteeringCommand; }
    EPinkCabVehicleMotionMode GetMotionMode() const { return MotionClassifier.GetMode(); }
    const FPinkCabVehicleHealthState& GetVehicleHealthState() const { return VehicleHealthBinding.Get(); }
    FPinkCabVehicleHealthState& GetMutableVehicleHealthState() { return VehicleHealthBinding.GetMutable(); }
    void BindVehicleHealthState(FPinkCabVehicleHealthState& ExternalHealth) { VehicleHealthBinding.Bind(ExternalHealth); }
    void UnbindVehicleHealthState() { VehicleHealthBinding.UnbindPreservingState(); }
    bool IsVehicleHealthStateBound() const { return VehicleHealthBinding.IsBound(); }
    void ResetTransientCockpitInput();

    void SetCockpitTaximeterSource(const FPinkCabTaximeter* InTaximeter) { CockpitTaximeterSource = InTaximeter; }
    void SetCockpitRouteProgress(TOptional<float> InRouteProgress01) { CockpitRouteProgress01 = InRouteProgress01; }
    void SetCockpitRadioAvailable(bool bAvailable) { bCockpitRadioAvailable = bAvailable; }
    void SetCockpitMirrorsAvailable(bool bAvailable) { bCockpitMirrorsAvailable = bAvailable; }

private:
    UPROPERTY(VisibleAnywhere, Category = "PinkCab|Visual")
    TObjectPtr<UPinkCabVehicleVisualShellComponent> VehicleVisualShell;

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
    bool SyncLoadToChaos();
    void ApplyHealthToControls();
    void HandleApplicationWillDeactivate();

    FPinkCabPrototypeVisualProfile PrototypeVisualProfile =
        FPinkCabPrototypeVisualProfile::EpicSportsCarManny();
    FPinkCabTatraProfile TatraProfile = FPinkCabTatraProfile::Canonical();
    FPinkCabVehicleLoadState VehicleLoadState;
    FPinkCabVehicleDamageProfile VehicleDamageProfile{TEXT("PinkCab.Damage.Fallback")};
    FPinkCabVehicleHealthService VehicleHealthService;
    FPinkCabChaosVehicleDynamicsProvider DynamicsProvider;
    FPinkCabVehicleControlState ControlState;
    FPinkCabCockpitState CockpitState;
    FPinkCabSemanticInputRouter InputRouter = FPinkCabSemanticInputRouter::CreateDefaults();
    FPinkCabVehicleMotionClassifier MotionClassifier;
    FPinkCabSteeringController SteeringController;
    FPinkCabLaunchController LaunchController;
    FPinkCabPedalDosingController PedalDosingController;
    FPinkCabHandbrakeActuator HandbrakeActuator;
    FPinkCabGearboxController GearboxController;
    FPinkCabDrivetrainCondition DrivetrainCondition;
    FPinkCabVehicleHealthBinding VehicleHealthBinding;
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
    float LastSpeedKmh = 0.0f;
    float LastEngineRpm = 0.0f;
    float DrivetrainTorqueCapacity = 1.0f;
    uint32 LastProcessedGearEventSerial = 0;
    bool bThrottleHeldLastFrame = false;
    float LookYaw = 0.0f;
    float LookPitch = 0.0f;
    FDelegateHandle ApplicationWillDeactivateHandle;
};
