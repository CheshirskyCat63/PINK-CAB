#include "Vehicle/PinkCabChaosTatraPawn.h"

#include "Camera/CameraComponent.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "Cockpit/PinkCabCockpitAssemblyComponent.h"
#include "Cockpit/PinkCabCockpitInteractionComponent.h"
#include "Cockpit/PinkCabCockpitPresentationState.h"
#include "Cockpit/PinkCabCockpitServiceBridge.h"
#include "Cockpit/PinkCabCockpitVisualDriverComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputCoreTypes.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/CoreDelegates.h"
#include "Interaction/PinkCabInteractionModel.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "Persistence/PinkCabVehicleSnapshot.h"
#include "Vehicle/PinkCabChaosLoadBridge.h"
#include "Vehicle/PinkCabVehicleVisualShellComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Vehicle/PinkCabChaosWheelFront.h"
#include "Vehicle/PinkCabChaosWheelRear.h"
#include "Vehicle/PinkCabChaosPhysicalProfile.h"
#include "Vehicle/PinkCabChaosCockpitBridge.h"
#include "Vehicle/PinkCabCockpitInteractionRouter.h"
#include "Vehicle/PinkCabVehicleInputFrame.h"

APinkCabChaosTatraPawn::APinkCabChaosTatraPawn()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bTickEvenWhenPaused = true;
    AutoPossessPlayer = EAutoReceiveInput::Player0;

    USkeletalMeshComponent* VehicleMesh = GetMesh();
    if (USkeletalMesh* VehicleAsset = Cast<USkeletalMesh>(PrototypeVisualProfile.VehicleMeshPath.TryLoad()))
    {
        VehicleMesh->SetSkeletalMesh(VehicleAsset);
    }
    if (UPhysicsAsset* PhysicsAsset = Cast<UPhysicsAsset>(PrototypeVisualProfile.PhysicsAssetPath.TryLoad()))
    {
        VehicleMesh->SetPhysicsAsset(PhysicsAsset, false);
    }

    VehicleMesh->SetCollisionProfileName(TEXT("Vehicle"));
    VehicleMesh->SetSimulatePhysics(true);
    VehicleMesh->SetOwnerNoSee(true);

    VehicleVisualShell = CreateDefaultSubobject<UPinkCabVehicleVisualShellComponent>(TEXT("VehicleVisualShell"));
    VehicleVisualShell->SetupAttachment(VehicleMesh);

    CockpitAssembly = CreateDefaultSubobject<UPinkCabCockpitAssemblyComponent>(TEXT("CockpitAssembly"));
    CockpitAssembly->SetupAttachment(VehicleMesh);
    CockpitAssembly->SetRelativeTransform(PrototypeVisualProfile.CockpitRootTransform);

    CockpitInteraction = CreateDefaultSubobject<UPinkCabCockpitInteractionComponent>(TEXT("CockpitInteraction"));
    CockpitVisualDriver = CreateDefaultSubobject<UPinkCabCockpitVisualDriverComponent>(TEXT("CockpitVisualDriver"));

    PrototypeDriverVisual = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("PrototypeDriverVisual"));
    PrototypeDriverVisual->SetupAttachment(VehicleMesh);
    PrototypeDriverVisual->SetRelativeTransform(PrototypeVisualProfile.DriverTransform);
    PrototypeDriverVisual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    PrototypeDriverVisual->SetGenerateOverlapEvents(false);
    PrototypeDriverVisual->SetCastShadow(false);
    PrototypeDriverVisual->SetOwnerNoSee(true);
    if (USkeletalMesh* DriverAsset = Cast<USkeletalMesh>(PrototypeVisualProfile.DriverMeshPath.TryLoad()))
    {
        PrototypeDriverVisual->SetSkeletalMesh(DriverAsset);
    }

    DriverHeadRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DriverHeadRoot"));
    DriverHeadRoot->SetupAttachment(CockpitAssembly);
    DriverHeadRoot->SetRelativeLocation(FVector(-15.0f, -38.0f, 128.0f));

    DriverCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("DriverCamera"));
    DriverCamera->SetupAttachment(DriverHeadRoot);
    DriverCamera->SetFieldOfView(86.0f);
    DriverCamera->bUsePawnControlRotation = false;
    DriverCamera->SetAutoActivate(true);
    CockpitAssembly->RegisterExternalSlot(EPinkCabCockpitSlot::DriverCamera, DriverCamera);

    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(VehicleMesh);
    CameraBoom->TargetArmLength = 520.0f;
    CameraBoom->SocketOffset = FVector(0.0f, 0.0f, 150.0f);
    CameraBoom->bDoCollisionTest = false;
    CameraBoom->bInheritPitch = false;
    CameraBoom->bInheritRoll = false;

    ChaseCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ChaseCamera"));
    ChaseCamera->SetupAttachment(CameraBoom);
    ChaseCamera->SetAutoActivate(false);

    UChaosWheeledVehicleMovementComponent* Movement = GetChaosMovement();
    check(Movement);
    DynamicsProvider = FPinkCabChaosVehicleDynamicsProvider(Movement);

    const FPinkCabChaosPhysicalProfile Profile =
        FPinkCabChaosPhysicalProfile::ForVariant(EPinkCabCalibrationVariant::Nominal);
    Profile.ApplyToMovement(*Movement);

    VehicleLoadState.SetFuelMassKg(TatraProfile.FullFuelMassKg);
    VehicleLoadState.SetCrew(TatraProfile.HeroineMassKg, TatraProfile.DaughterMassKg);
    SyncLoadToChaos();

    Movement->WheelSetups.SetNum(4);
    Movement->WheelSetups[0].WheelClass = UPinkCabChaosWheelFront::StaticClass();
    Movement->WheelSetups[0].BoneName = PrototypeVisualProfile.WheelBones[0];
    Movement->WheelSetups[1].WheelClass = UPinkCabChaosWheelFront::StaticClass();
    Movement->WheelSetups[1].BoneName = PrototypeVisualProfile.WheelBones[1];
    Movement->WheelSetups[2].WheelClass = UPinkCabChaosWheelRear::StaticClass();
    Movement->WheelSetups[2].BoneName = PrototypeVisualProfile.WheelBones[2];
    Movement->WheelSetups[3].WheelClass = UPinkCabChaosWheelRear::StaticClass();
    Movement->WheelSetups[3].BoneName = PrototypeVisualProfile.WheelBones[3];
}

void APinkCabChaosTatraPawn::BeginPlay()
{
    Super::BeginPlay();
    DynamicsProvider = FPinkCabChaosVehicleDynamicsProvider(GetChaosMovement());
    DriverCamera->SetActive(true);
    ChaseCamera->SetActive(false);
    SyncLoadToChaos();
    SyncCockpitToChaos();
    ApplicationWillDeactivateHandle = FCoreDelegates::ApplicationWillDeactivateDelegate.AddUObject(
        this, &APinkCabChaosTatraPawn::HandleApplicationWillDeactivate);
}

void APinkCabChaosTatraPawn::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (ApplicationWillDeactivateHandle.IsValid())
    {
        FCoreDelegates::ApplicationWillDeactivateDelegate.Remove(ApplicationWillDeactivateHandle);
        ApplicationWillDeactivateHandle.Reset();
    }
    ResetTransientCockpitInput();
    Super::EndPlay(EndPlayReason);
}

void APinkCabChaosTatraPawn::HandleApplicationWillDeactivate()
{
    ResetTransientCockpitInput();
}

void APinkCabChaosTatraPawn::ResetTransientCockpitInput()
{
    if (!CockpitInteraction)
    {
        return;
    }
    TArray<FPinkCabInteractionEvent> ReleaseEvents;
    CockpitInteraction->ResetTransientInputState(&ReleaseEvents);
    for (const FPinkCabInteractionEvent& ReleaseEvent : ReleaseEvents)
    {
        ApplyCockpitInteraction(ReleaseEvent);
    }
}

bool APinkCabChaosTatraPawn::ApplyCockpitInteraction(const FPinkCabInteractionEvent& Event)
{
    if (!FPinkCabCockpitInteractionRouter::Apply(Event, CockpitState))
    {
        return false;
    }
    SyncCockpitToChaos();
    return true;
}

void APinkCabChaosTatraPawn::SyncCockpitToChaos()
{
    if (UChaosWheeledVehicleMovementComponent* Movement = GetChaosMovement())
    {
        ApplyHealthToControls();
        FPinkCabChaosCockpitBridge::Apply(CockpitState, *Movement, ControlState, DynamicsProvider);
        if (!VehicleHealthService.HasCapability(VehicleHealthState, EPinkCabVehicleCapability::RunEngine))
        {
            Movement->EnableMechanicalSim(false);
        }
    }
}

FName APinkCabChaosTatraPawn::GetVehicleVisualProfileId() const
{
    return VehicleVisualShell ? VehicleVisualShell->GetProfileId() : NAME_None;
}

bool APinkCabChaosTatraPawn::ApplyVehicleVisualProfile(const FPinkCabVehicleVisualProfile& Profile)
{
    if (!VehicleVisualShell || !Profile.IsValid() || !VehicleVisualShell->ApplyProfile(Profile))
    {
        return false;
    }
    CockpitAssembly->SetRelativeTransform(Profile.CockpitRootTransform);
    DriverHeadRoot->SetRelativeTransform(Profile.DriverHeadTransform);
    if (USkeletalMeshComponent* VehicleMesh = GetMesh())
    {
        VehicleMesh->SetVisibility(!(Profile.HasExteriorAsset() && Profile.bHidePhysicsChassisWhenExteriorPresent), true);
    }
    return true;
}

bool APinkCabChaosTatraPawn::SyncLoadToChaos()
{
    UChaosWheeledVehicleMovementComponent* Movement = GetChaosMovement();
    return Movement && FPinkCabChaosLoadBridge::Apply(VehicleLoadState, TatraProfile, *Movement);
}

void APinkCabChaosTatraPawn::ApplyHealthToControls()
{
    VehicleHealthService.ApplyCapabilitiesToControls(VehicleHealthState, ControlState);
}

bool APinkCabChaosTatraPawn::SetFuelMassKg(const float MassKg, const float LongitudinalCm)
{
    if (!FMath::IsFinite(MassKg) || !FMath::IsFinite(LongitudinalCm) || MassKg < 0.0f)
    {
        return false;
    }
    const FPinkCabVehicleLoadState Before = VehicleLoadState;
    VehicleLoadState.SetFuelMassKg(MassKg, LongitudinalCm);
    if (SyncLoadToChaos()) return true;
    VehicleLoadState = Before;
    SyncLoadToChaos();
    return false;
}

bool APinkCabChaosTatraPawn::TrySetFarePassengerGroup(
    const FPinkCabStableId& GroupId,
    TConstArrayView<FPinkCabVehicleLoadItem> Items)
{
    const FPinkCabVehicleLoadState Before = VehicleLoadState;
    if (!VehicleLoadState.TrySetFarePassengerGroup(GroupId, Items)) return false;
    if (SyncLoadToChaos()) return true;
    VehicleLoadState = Before;
    SyncLoadToChaos();
    return false;
}

bool APinkCabChaosTatraPawn::RemoveFarePassengerGroup(const FPinkCabStableId& GroupId)
{
    const FPinkCabVehicleLoadState Before = VehicleLoadState;
    if (!VehicleLoadState.RemoveFarePassengerGroup(GroupId)) return false;
    if (SyncLoadToChaos()) return true;
    VehicleLoadState = Before;
    SyncLoadToChaos();
    return false;
}

bool APinkCabChaosTatraPawn::SetVehicleDamageProfile(const FPinkCabVehicleDamageProfile& Profile)
{
    if (Profile.GetProfileId().IsNone()) return false;
    VehicleDamageProfile = Profile;
    return true;
}

bool APinkCabChaosTatraPawn::ApplyAuthoredVehicleHit(FName ZoneId, float CollisionSeverity)
{
    FPinkCabVehicleHitEvent Event;
    if (!VehicleDamageProfile.ResolveFunctionalHit(ZoneId, CollisionSeverity, Event)) return false;
    return ApplyVehicleHit(Event);
}

bool APinkCabChaosTatraPawn::ApplyVehicleHit(const FPinkCabVehicleHitEvent& Event)
{
    if (!VehicleHealthService.ApplyHit(VehicleHealthState, Event)) return false;
    ApplyHealthToControls();
    DynamicsProvider.ApplyControls(ControlState);
    if (!VehicleHealthService.HasCapability(VehicleHealthState, EPinkCabVehicleCapability::RunEngine))
    {
        if (UChaosWheeledVehicleMovementComponent* Movement = GetChaosMovement())
        {
            Movement->EnableMechanicalSim(false);
        }
    }
    return true;
}

bool APinkCabChaosTatraPawn::CaptureVehicleSnapshot(FPinkCabVehicleSnapshot& OutSnapshot) const
{
    return FPinkCabVehicleSnapshotCodec::Capture(VehicleHealthState, VehicleLoadState, OutSnapshot);
}

bool APinkCabChaosTatraPawn::RestoreVehicleSnapshot(const FPinkCabVehicleSnapshot& Snapshot)
{
    FPinkCabVehicleHealthState RestoredHealth;
    FPinkCabVehicleLoadState RestoredLoad;
    if (!FPinkCabVehicleSnapshotCodec::Restore(Snapshot, RestoredHealth, RestoredLoad)) return false;

    const FPinkCabVehicleHealthState BeforeHealth = VehicleHealthState;
    const FPinkCabVehicleLoadState BeforeLoad = VehicleLoadState;
    VehicleHealthState = RestoredHealth;
    VehicleLoadState = RestoredLoad;
    if (!SyncLoadToChaos())
    {
        VehicleHealthState = BeforeHealth;
        VehicleLoadState = BeforeLoad;
        SyncLoadToChaos();
        return false;
    }
    ApplyHealthToControls();
    SyncCockpitToChaos();
    return true;
}

float APinkCabChaosTatraPawn::IntegrateMouseSteering(
    const float CurrentSteering,
    const float DeltaX,
    const bool bGazeHeld,
    const float Gain)
{
    return bGazeHeld ? CurrentSteering : FMath::Clamp(CurrentSteering + DeltaX * Gain, -1.0f, 1.0f);
}

void APinkCabChaosTatraPawn::ApplyMouseSteeringDelta(const float DeltaX, const bool bGazeHeld)
{
    SteeringCommand = IntegrateMouseSteering(SteeringCommand, DeltaX, bGazeHeld, MouseSteeringGain);
    ControlState.SetSteering(SteeringCommand);
}

void APinkCabChaosTatraPawn::ApplyVehicleInputFrame(
    const FPinkCabVehicleInputFrame& InputFrame,
    const float MouseDeltaX)
{
    if (CockpitInteraction)
    {
        CockpitInteraction->SetGazeHeld(InputFrame.bGazeHeld);
    }
    ApplyMouseSteeringDelta(MouseDeltaX, InputFrame.bGazeHeld);
    ControlState = InputFrame.ToControlState(
        SteeringCommand,
        CockpitState.GetHandbrakeAmount());
    ApplyHealthToControls();
    DynamicsProvider.ApplyControls(ControlState);
}

void APinkCabChaosTatraPawn::Tick(const float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    APlayerController* PC = Cast<APlayerController>(GetController());
    if (!PC || UGameplayStatics::IsGamePaused(this))
    {
        ResetTransientCockpitInput();
        return;
    }

    float MouseX = 0.0f;
    float MouseY = 0.0f;
    PC->GetInputMouseDelta(MouseX, MouseY);
    const FPinkCabVehicleInputFrame InputFrame = FPinkCabVehicleInputFrame::FromRouter(
        InputRouter,
        [PC](const FKey& Key) { return PC->IsInputKeyDown(Key); });

    const auto IsActionHeld = [this, PC](const EPinkCabSemanticAction Action)
    {
        const FKey Key = InputRouter.GetKeyForAction(Action);
        return Key.IsValid() && PC->IsInputKeyDown(Key);
    };
    const FKey WheelKey = InputRouter.GetKeyForAction(EPinkCabSemanticAction::Wheel);
    const float WheelAxis = WheelKey.IsValid() ? PC->GetInputAnalogKeyState(WheelKey) : 0.0f;

    FPinkCabCockpitInteractionFrame InteractionFrame;
    InteractionFrame.bGazeHeld = InputFrame.bGazeHeld;
    InteractionFrame.bQuickRecall1Held = IsActionHeld(EPinkCabSemanticAction::QuickRecall1);
    InteractionFrame.bQuickRecall2Held = IsActionHeld(EPinkCabSemanticAction::QuickRecall2);
    InteractionFrame.bQuickRecall3Held = IsActionHeld(EPinkCabSemanticAction::QuickRecall3);
    InteractionFrame.bQuickRecall4Held = IsActionHeld(EPinkCabSemanticAction::QuickRecall4);
    InteractionFrame.bGripHeld = IsActionHeld(EPinkCabSemanticAction::Grip);
    InteractionFrame.bMomentaryHeld = IsActionHeld(EPinkCabSemanticAction::MomentaryPress);
    InteractionFrame.WheelSteps = WheelAxis > 0.0f ? 1 : (WheelAxis < 0.0f ? -1 : 0);
    InteractionFrame.NowSeconds = FPlatformTime::Seconds();
    if (DriverCamera)
    {
        InteractionFrame.GazeOrigin = DriverCamera->GetComponentLocation();
        InteractionFrame.GazeForward = DriverCamera->GetForwardVector();
    }

    TArray<FPinkCabInteractionEvent> InteractionEvents;
    CockpitInteraction->ProcessFrame(InteractionFrame, CockpitAssembly, InteractionEvents);
    for (const FPinkCabInteractionEvent& InteractionEvent : InteractionEvents)
    {
        ApplyCockpitInteraction(InteractionEvent);
    }

    ApplyVehicleInputFrame(InputFrame, MouseX);
    const bool bGazeHeld = CockpitInteraction->IsGazeHeld();

    if (bGazeHeld)
    {
        LookYaw = FMath::Clamp(LookYaw + MouseX * 0.45f, -110.0f, 110.0f);
        LookPitch = FMath::Clamp(LookPitch - MouseY * 0.35f, -45.0f, 35.0f);
    }
    else
    {
        LookYaw = FMath::FInterpTo(LookYaw, 0.0f, DeltaSeconds, 4.0f);
        LookPitch = FMath::FInterpTo(LookPitch, 0.0f, DeltaSeconds, 4.0f);
    }

    DriverHeadRoot->SetRelativeRotation(FRotator(LookPitch, LookYaw, 0.0f));

    FPinkCabCockpitPresentationState Presentation;
    Presentation.Steering = ControlState.Steering;
    Presentation.Clutch = ControlState.Clutch;
    Presentation.Brake = ControlState.Brake;
    Presentation.Throttle = ControlState.Throttle;
    Presentation.SelectedGear = CockpitState.GetSelectedGear();
    Presentation.bIgnitionRunning = CockpitState.GetIgnitionState() == EPinkCabIgnitionState::Running;
    Presentation.Handbrake = CockpitState.GetHandbrakeAmount();
    Presentation.bHandbrakeEngaged = CockpitState.IsHandbrakeEngaged();
    FPinkCabCockpitServiceSources ServiceSources;
    ServiceSources.Taximeter = CockpitTaximeterSource;
    ServiceSources.CockpitState = &CockpitState;
    ServiceSources.RouteProgress01 = CockpitRouteProgress01;
    ServiceSources.bRadioAvailable = bCockpitRadioAvailable;
    ServiceSources.bMirrorsAvailable = bCockpitMirrorsAvailable;
    FPinkCabCockpitServiceBridge::ApplyToPresentation(
        FPinkCabCockpitServiceBridge::Read(ServiceSources), Presentation);
    Presentation.bTurnSignalLeft = CockpitState.GetTurnSignalDirection() < 0;
    Presentation.bTurnSignalRight = CockpitState.GetTurnSignalDirection() > 0;
    Presentation.bHornActive = CockpitState.IsHornActive();
    Presentation.bLightsOn = CockpitState.GetLightMode() > 0;
    Presentation.bWipersOn = CockpitState.GetWiperMode() > 0;
    Presentation.bWasherActive = CockpitState.IsWasherActive();
    FPinkCabVehicleTelemetry Telemetry;
    if (DynamicsProvider.ReadTelemetry(Telemetry))
    {
        Presentation.SpeedKmh = Telemetry.SpeedKmh;
        Presentation.EngineRpm = Telemetry.EngineRpm;
    }
    CockpitVisualDriver->Apply(*CockpitAssembly, Presentation);

}

UChaosWheeledVehicleMovementComponent* APinkCabChaosTatraPawn::GetChaosMovement() const
{
    return Cast<UChaosWheeledVehicleMovementComponent>(GetVehicleMovementComponent());
}
