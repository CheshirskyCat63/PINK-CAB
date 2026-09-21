#include "Runtime/PinkCabChaosTatraPawn.h"
#include "Runtime/PinkCabDriverUiComponent.h"

#include "Camera/CameraComponent.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "Cockpit/PinkCabCockpitAssemblyComponent.h"
#include "Cockpit/PinkCabCockpitInteractionComponent.h"
#include "Cockpit/PinkCabCockpitPresentationState.h"
#include "Cockpit/PinkCabCockpitServiceBridge.h"
#include "Cockpit/PinkCabCockpitVisualDriverComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/SkyLightComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/SkinnedAsset.h"
#include "EngineUtils.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "GameFramework/Actor.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Math/RotationMatrix.h"
#include "HAL/PlatformTime.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Interaction/PinkCabInteractionModel.h"
#include "Interaction/PinkCabPhysicalInputConvention.h"
#include "Interaction/PinkCabPlayerInputAdapter.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "PhysicsEngine/BodyInstance.h"
#include "PhysicsEngine/BodySetup.h"
#include "Vehicle/PinkCabVehicleStateSnapshot.h"
#include "Vehicle/PinkCabChaosLoadBridge.h"
#include "Runtime/PinkCabVehicleVisualShellComponent.h"
#include "Runtime/PinkCabVehicleVisualProfile.h"
#include "UObject/ConstructorHelpers.h"
#include "Vehicle/PinkCabChaosWheelFront.h"
#include "Vehicle/PinkCabChaosWheelRear.h"
#include "Vehicle/PinkCabChaosPhysicalProfile.h"
#include "Vehicle/PinkCabChaosCockpitBridge.h"
#include "Vehicle/PinkCabCockpitInteractionRouter.h"
#include "Vehicle/PinkCabVehicleInputFrame.h"
#include "Vehicle/PinkCabVehicleInputResponse.h"

namespace
{

void ConfigurePhysicsVehicleMesh(
    USkeletalMeshComponent& VehicleMesh,
    const FPinkCabPrototypeVisualProfile& PrototypeVisualProfile)
{
    if (USkeletalMesh* VehicleAsset = Cast<USkeletalMesh>(
            PrototypeVisualProfile.VehicleMeshPath.TryLoad()))
    {
        VehicleMesh.SetSkeletalMesh(VehicleAsset);
    }
    if (UPhysicsAsset* PhysicsAsset = Cast<UPhysicsAsset>(
            PrototypeVisualProfile.PhysicsAssetPath.TryLoad()))
    {
        VehicleMesh.SetPhysicsAsset(PhysicsAsset, false);
    }
    VehicleMesh.SetCollisionProfileName(TEXT("Vehicle"));
    VehicleMesh.SetSimulatePhysics(true);
    VehicleMesh.SetOwnerNoSee(true);
}

FVector ResolveWheelBoneLocalPosition(
    const USkeletalMeshComponent& Mesh,
    const FName BoneName)
{
    const USkinnedAsset* Asset = Mesh.GetSkinnedAsset();
    if (!Asset || BoneName.IsNone())
    {
        return FVector::ZeroVector;
    }

    const FVector BonePosition =
        Asset->GetComposedRefPoseMatrix(BoneName).GetOrigin() * Mesh.GetRelativeScale3D();
    FMatrix RootBodyMatrix = FMatrix::Identity;
    if (const FBodyInstance* BodyInstance = Mesh.GetBodyInstance())
    {
        if (BodyInstance->BodySetup.IsValid())
        {
            RootBodyMatrix = Asset->GetComposedRefPoseMatrix(BodyInstance->BodySetup->BoneName);
        }
    }
    return RootBodyMatrix.InverseTransformPosition(BonePosition);
}

const FPinkCabVehiclePresentationPart* FindPresentationPart(
    const FPinkCabVehicleVisualProfile& Profile,
    const FName PartId)
{
    return Profile.PresentationParts.FindByPredicate([PartId](const FPinkCabVehiclePresentationPart& Part)
    {
        return Part.PartId == PartId;
    });
}

bool BindChaosWheelsToTatraGeometry(
    UChaosWheeledVehicleMovementComponent& Movement,
    const USkeletalMeshComponent& Mesh,
    const FPinkCabVehicleVisualProfile& TatraVisual)
{
    if (Movement.WheelSetups.Num() != 4)
    {
        return false;
    }

    const FName PartIds[4] = { TEXT("WheelFL"), TEXT("WheelFR"), TEXT("WheelRL"), TEXT("WheelRR") };
    for (int32 Index = 0; Index < 4; ++Index)
    {
        FChaosWheelSetup& Setup = Movement.WheelSetups[Index];
        const FPinkCabVehiclePresentationPart* Part = FindPresentationPart(TatraVisual, PartIds[Index]);
        if (!Part || !Setup.WheelClass)
        {
            return false;
        }

        const UChaosVehicleWheel* WheelDefaults = Setup.WheelClass.GetDefaultObject();
        if (!WheelDefaults)
        {
            return false;
        }

        const FVector BaseRestPosition =
            ResolveWheelBoneLocalPosition(Mesh, Setup.BoneName) + WheelDefaults->Offset;
        Setup.AdditionalOffset = Part->LocalTransform.GetLocation() - BaseRestPosition;
    }
    return true;
}

}

APinkCabChaosTatraPawn::APinkCabChaosTatraPawn()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bTickEvenWhenPaused = true;
    AutoPossessPlayer = EAutoReceiveInput::Player0;

    DriverUi = CreateDefaultSubobject<UPinkCabDriverUiComponent>(TEXT("DriverUi"));

    USkeletalMeshComponent* VehicleMesh = GetMesh();
    ConfigurePhysicsVehicleMesh(*VehicleMesh, PrototypeVisualProfile);

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
    DriverCamera->SetRelativeRotation(FRotator(-4.0f, 0.0f, 0.0f));
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

    const FPinkCabChaosPhysicalProfile Profile = FPinkCabChaosPhysicalProfile::ForVariant(
        EPinkCabCalibrationVariant::Nominal);
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

    if (!BindChaosWheelsToTatraGeometry(
            *Movement, *VehicleMesh, FPinkCabVehicleVisualProfile::Tatra613Donor()))
    {
        UE_LOG(LogTemp, Error, TEXT("PinkCab Tatra wheel geometry contract could not be bound; disabling Chaos wheel setups."));
        Movement->WheelSetups.Reset();
    }
}

void APinkCabChaosTatraPawn::BeginPlay()
{
    Super::BeginPlay();
    DynamicsProvider = FPinkCabChaosVehicleDynamicsProvider(GetChaosMovement());
    EnsurePlayableLighting();
    ApplyVehicleVisualProfile(FPinkCabVehicleVisualProfile::Tatra613Donor());
    AlignInitialPresentationToGround();
    DriverCamera->SetActive(true);
    ChaseCamera->SetActive(false);
    SyncLoadToChaos();
    FPinkCabVehicleControlRuntimeConfig ControlConfig;
    ControlConfig.ClutchPressSeconds = ClutchPressSeconds;
    ControlConfig.BrakePressSeconds = BrakePressSeconds;
    ControlConfig.BrakeReleaseSeconds = BrakeReleaseSeconds;
    ControlConfig.ThrottlePressSeconds = ThrottlePressSeconds;
    ControlConfig.ThrottleReleaseSeconds = ThrottleReleaseSeconds;
    VehicleControlRuntime.SetResponseConfig(ControlConfig);
    SyncCockpitToChaos();

    APlayerController* PC = Cast<APlayerController>(GetController());
    if (DriverUi)
    {
        DriverUi->BeginRuntime(PC);
    }
    SetSystemMenuOpen(true);

    bPackagedGateTelemetryEnabled =
        FParse::Param(FCommandLine::Get(), TEXT("PinkCabGateTelemetry"));
    if (bPackagedGateTelemetryEnabled)
    {
        PackagedGateStartLocation = GetActorLocation();
        NextPackagedGateTelemetrySeconds = 0.0;
        UE_LOG(LogTemp, Display, TEXT("PINKCAB_GATE_BEGIN source=packaged_runtime_input_probe"));
    }
}

void APinkCabChaosTatraPawn::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    ResetTransientCockpitInput();
    if (DriverUi)
    {
        DriverUi->EndRuntime(Cast<APlayerController>(GetController()));
    }
    Super::EndPlay(EndPlayReason);
}

void APinkCabChaosTatraPawn::Tick(const float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    APlayerController* PC = Cast<APlayerController>(GetController());
    if (!PC)
    {
        ResetTransientCockpitInput();
        return;
    }

    EmitPackagedGateTelemetry(FPlatformTime::Seconds());

    FPinkCabPlayerInputSample PlayerInput;
    if (!BeginDriverFrame(*PC, PlayerInput))
    {
        return;
    }
    EPinkCabPedalWheelRecipient WheelRecipient = EPinkCabPedalWheelRecipient::None;
    FPinkCabVehicleInputFrame InputFrame =
        PrepareVehicleFrame(PlayerInput, DeltaSeconds, WheelRecipient);
    const bool bPhysicalManipulationActive =
        ProcessCockpitFrame(*PC, PlayerInput, WheelRecipient, DeltaSeconds);
    InputFrame.bSteeringHeld = bPhysicalManipulationActive;
    const float SteeringMouseX = FPinkCabPhysicalInputConvention::SteeringRight(PlayerInput.DeviceX);
    ApplyVehicleInputFrame(
        InputFrame,
        bPhysicalManipulationActive ? 0.0f : SteeringMouseX,
        DeltaSeconds);
    UpdateDriverLook(PlayerInput, bPhysicalManipulationActive, DeltaSeconds);
    const FPinkCabCockpitPresentationState Presentation = BuildCockpitPresentation(DeltaSeconds);
    CockpitVisualDriver->Apply(*CockpitAssembly, Presentation);
    UpdateDriverUiState(Presentation);
    EmitPackagedGateTelemetry(FPlatformTime::Seconds());
}

void APinkCabChaosTatraPawn::EmitPackagedGateTelemetry(const double NowSeconds)
{
    if (!bPackagedGateTelemetryEnabled || NowSeconds < NextPackagedGateTelemetrySeconds)
    {
        return;
    }
    NextPackagedGateTelemetrySeconds = NowSeconds + 0.10;

    FPinkCabVehicleTelemetry Telemetry;
    const bool bHasTelemetry = DynamicsProvider.ReadTelemetry(Telemetry);
    const FVector Location = GetActorLocation();
    const float DistanceCm = FVector::Dist2D(Location, PackagedGateStartLocation);
    const FName TargetId =
        CockpitInteraction ? CockpitInteraction->GetCurrentTargetId() : NAME_None;
    const bool bGrip = CockpitInteraction && CockpitInteraction->IsGripActive();
    const bool bManipulation =
        CockpitInteraction && CockpitInteraction->IsManipulationActive();
    const FVector2D Cursor = GetGearLeverVisualCursor();
    const UChaosWheeledVehicleMovementComponent* Movement = GetChaosMovement();

    UE_LOG(
        LogTemp,
        Display,
        TEXT("PINKCAB_GATE_STATE menu=%d ignition=%d requested=%d engaged=%d throttle=%.3f brake=%.3f clutch=%.3f handbrake=%.3f steering=%.3f speed=%.3f dist=%.1f gearx=%.3f geary=%.3f target=%s grip=%d manipulation=%d camera=%d wheels=%d"),
        IsSystemMenuOpen() ? 1 : 0,
        CockpitState.GetIgnitionState() == EPinkCabIgnitionState::Running ? 1 : 0,
        GetRequestedGear(),
        GetEngagedGear(),
        bHasTelemetry ? Telemetry.NormalizedThrottle : 0.0f,
        bHasTelemetry ? Telemetry.NormalizedBrake : 0.0f,
        bHasTelemetry ? Telemetry.NormalizedClutch : 0.0f,
        bHasTelemetry ? Telemetry.NormalizedHandbrake : 0.0f,
        bHasTelemetry ? Telemetry.NormalizedSteering : GetSteeringCommand(),
        bHasTelemetry ? Telemetry.SpeedKmh : 0.0f,
        DistanceCm,
        Cursor.X,
        Cursor.Y,
        *TargetId.ToString(),
        bGrip ? 1 : 0,
        bManipulation ? 1 : 0,
        DriverCamera && DriverCamera->IsActive() ? 1 : 0,
        Movement ? Movement->GetNumWheels() : 0);
}

UChaosWheeledVehicleMovementComponent* APinkCabChaosTatraPawn::GetChaosMovement() const
{
    return Cast<UChaosWheeledVehicleMovementComponent>(GetVehicleMovementComponent());
}
