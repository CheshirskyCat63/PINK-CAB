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
#include "Interaction/PinkCabInteractionModel.h"
#include "PhysicsEngine/PhysicsAsset.h"
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
    SyncCockpitToChaos();
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
        FPinkCabChaosCockpitBridge::Apply(CockpitState, *Movement, ControlState, DynamicsProvider);
    }
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

void APinkCabChaosTatraPawn::Tick(const float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    APlayerController* PC = Cast<APlayerController>(GetController());
    if (!PC)
    {
        return;
    }

    float MouseX = 0.0f;
    float MouseY = 0.0f;
    PC->GetInputMouseDelta(MouseX, MouseY);
    const FPinkCabVehicleInputFrame InputFrame = FPinkCabVehicleInputFrame::FromRouter(
        InputRouter,
        [PC](const FKey& Key) { return PC->IsInputKeyDown(Key); });

    CockpitInteraction->SetGazeHeld(InputFrame.bGazeHeld);
    const EPinkCabSemanticAction QuickActions[] = {
        EPinkCabSemanticAction::QuickRecall1,
        EPinkCabSemanticAction::QuickRecall2,
        EPinkCabSemanticAction::QuickRecall3,
        EPinkCabSemanticAction::QuickRecall4};
    for (int32 Index = 0; Index < UE_ARRAY_COUNT(QuickActions); ++Index)
    {
        const FKey Key = InputRouter.GetKeyForAction(QuickActions[Index]);
        CockpitInteraction->SetQuickSlotHeld(Index + 1, Key.IsValid() && PC->IsInputKeyDown(Key));
    }
    CockpitInteraction->SetCandidateTarget(CockpitInteraction->GetCurrentQuickTargetId());

    const FKey AttentionKey = InputRouter.GetKeyForAction(EPinkCabSemanticAction::Attention);
    if (AttentionKey.IsValid() && PC->WasInputKeyJustPressed(AttentionKey))
    {
        CockpitInteraction->CommitAttention();
    }

    const FKey GoKey = InputRouter.GetKeyForAction(EPinkCabSemanticAction::Go);
    FPinkCabInteractionEvent InteractionEvent;
    if (GoKey.IsValid() && PC->WasInputKeyJustPressed(GoKey) && CockpitInteraction->BeginGo(InteractionEvent))
    {
        ApplyCockpitInteraction(InteractionEvent);
    }
    if (GoKey.IsValid() && PC->WasInputKeyJustReleased(GoKey) && CockpitInteraction->EndGo(InteractionEvent))
    {
        ApplyCockpitInteraction(InteractionEvent);
    }

    const FKey WheelKey = InputRouter.GetKeyForAction(EPinkCabSemanticAction::Wheel);
    const float WheelAxis = WheelKey.IsValid() ? PC->GetInputAnalogKeyState(WheelKey) : 0.0f;
    const int32 WheelSteps = WheelAxis > 0.0f ? 1 : (WheelAxis < 0.0f ? -1 : 0);
    if (CockpitInteraction->BuildWheelEvent(WheelSteps, InteractionEvent))
    {
        ApplyCockpitInteraction(InteractionEvent);
    }

    const bool bGazeHeld = CockpitInteraction->IsGazeHeld();
    ApplyMouseSteeringDelta(MouseX, bGazeHeld);

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

    ControlState = InputFrame.ToControlState(
        SteeringCommand,
        CockpitState.IsHandbrakeEngaged() ? 1.0f : 0.0f);
    DynamicsProvider.ApplyControls(ControlState);

    FPinkCabCockpitPresentationState Presentation;
    Presentation.Steering = ControlState.Steering;
    Presentation.Clutch = ControlState.Clutch;
    Presentation.Brake = ControlState.Brake;
    Presentation.Throttle = ControlState.Throttle;
    Presentation.SelectedGear = CockpitState.GetSelectedGear();
    Presentation.bIgnitionRunning = CockpitState.GetIgnitionState() == EPinkCabIgnitionState::Running;
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

    if (PC->WasInputKeyJustPressed(EKeys::R))
    {
        const FVector ResetLocation = GetActorLocation() + FVector(0.0f, 0.0f, 80.0f);
        const FRotator ResetRotation(0.0f, GetActorRotation().Yaw, 0.0f);
        SetActorTransform(FTransform(ResetRotation, ResetLocation), false, nullptr, ETeleportType::TeleportPhysics);
        GetMesh()->SetPhysicsLinearVelocity(FVector::ZeroVector);
        GetMesh()->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
    }
}

UChaosWheeledVehicleMovementComponent* APinkCabChaosTatraPawn::GetChaosMovement() const
{
    return Cast<UChaosWheeledVehicleMovementComponent>(GetVehicleMovementComponent());
}
