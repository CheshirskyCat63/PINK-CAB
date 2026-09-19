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

void APinkCabChaosTatraPawn::EnsurePlayableLighting()
{
    UWorld* World = GetWorld();
    if (!World) return;
    const FName RigTag(TEXT("PinkCab.PlayableLighting"));
    for (TActorIterator<AActor> It(World); It; ++It)
    {
        if (It->ActorHasTag(RigTag)) return;
    }

    AActor* Rig = World->SpawnActor<AActor>();
    if (!Rig) return;
    Rig->Tags.Add(RigTag);

    USceneComponent* Root = NewObject<USceneComponent>(Rig, TEXT("SkyRigRoot"));
    Rig->AddInstanceComponent(Root);
    Rig->SetRootComponent(Root);
    Root->RegisterComponent();

    USkyAtmosphereComponent* Atmosphere = NewObject<USkyAtmosphereComponent>(Rig, TEXT("PlayableSkyAtmosphere"));
    Rig->AddInstanceComponent(Atmosphere);
    Atmosphere->SetupAttachment(Root);
    Atmosphere->RegisterComponent();

    UDirectionalLightComponent* Sun = NewObject<UDirectionalLightComponent>(Rig, TEXT("PlayableSun"));
    Rig->AddInstanceComponent(Sun);
    Sun->SetupAttachment(Root);
    Sun->SetMobility(EComponentMobility::Movable);
    Sun->SetIntensity(4.0f);
    Sun->SetAtmosphereSunLight(true);
    Sun->SetRelativeRotation(FRotator(-32.0f, -28.0f, 0.0f));
    Sun->RegisterComponent();

    USkyLightComponent* SkyLight = NewObject<USkyLightComponent>(Rig, TEXT("PlayableSkyLight"));
    Rig->AddInstanceComponent(SkyLight);
    SkyLight->SetupAttachment(Root);
    SkyLight->SetMobility(EComponentMobility::Movable);
    SkyLight->SetIntensity(0.8f);
    SkyLight->SetRealTimeCapture(true);
    SkyLight->RegisterComponent();
}

void APinkCabChaosTatraPawn::SetSystemMenuOpen(const bool bOpen)
{
    ResetTransientCockpitInput();
    if (DriverUi)
    {
        if (APlayerController* PC = Cast<APlayerController>(GetController()))
        {
            DriverUi->SetSystemMenuOpen(*PC, bOpen);
        }
    }
}

bool APinkCabChaosTatraPawn::IsSystemMenuOpen() const
{
    return DriverUi && DriverUi->IsSystemMenuOpen();
}

void APinkCabChaosTatraPawn::ResetTransientCockpitInput()
{
    if (DriverUi)
    {
        if (APlayerController* PC = Cast<APlayerController>(GetController()))
        {
            DriverUi->ResetTransientInput(*PC);
        }
    }
    VehicleControlRuntime.ResetThrottleInputLatch();
    bGearLeverDragging = false;
    GearLeverCursor = UPinkCabCockpitVisualDriverComponent::GearCursorForGear(
        CockpitState.GetSelectedGear());
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
    const bool bControlsChanged = VehicleControlRuntime.ResetTransient(CockpitState);
    GearLeverCursor = UPinkCabCockpitVisualDriverComponent::GearCursorForGear(
        CockpitState.GetSelectedGear());
    if (bControlsChanged)
    {
        SyncCockpitToChaos();
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
        VehicleControlRuntime.ApplyHealthCapabilities(
            VehicleHealthService, GetVehicleHealthState());
        FPinkCabChaosCockpitBridge::Apply(
            CockpitState,
            *Movement,
            VehicleControlRuntime.GetMutableControlState(),
            DynamicsProvider);
        if (!VehicleHealthService.HasCapability(
                GetVehicleHealthState(), EPinkCabVehicleCapability::RunEngine))
        {
            Movement->EnableMechanicalSim(false);
        }
    }
}

FName APinkCabChaosTatraPawn::GetVehicleVisualProfileId() const
{
    return VehicleVisualShell ? VehicleVisualShell->GetProfileId() : NAME_None;
}

bool APinkCabChaosTatraPawn::ConfigureSourceSteeringVisual(
    const FPinkCabVehicleVisualProfile& Profile)
{
    if (!VehicleVisualShell || !CockpitVisualDriver) return false;

    CockpitVisualDriver->SetSteeringVisualComponent(nullptr);
    if (SourceSteeringPivot)
    {
        SourceSteeringPivot->DestroyComponent();
        SourceSteeringPivot = nullptr;
    }

    if (Profile.SteeringPresentationPartId.IsNone())
    {
        return true;
    }

    UStaticMeshComponent* SourceSteering =
        VehicleVisualShell->GetPresentationPartComponent(Profile.SteeringPresentationPartId);
    UStaticMesh* SteeringMesh = SourceSteering ? SourceSteering->GetStaticMesh() : nullptr;
    if (!SourceSteering || !SteeringMesh)
    {
        return false;
    }

    // Derive the steering-column pivot from the actual visible source wheel,
    // not camera/head-space or guessed scene coordinates. The preserved Tatra
    // wheel is a thin disc; its thinnest local bounds axis is the column axis.
    const FBoxSphereBounds LocalBounds = SteeringMesh->GetBounds();
    const FVector Extent = LocalBounds.BoxExtent;
    FVector LocalAxis = FVector::ForwardVector;
    if (Extent.Y <= Extent.X && Extent.Y <= Extent.Z)
    {
        LocalAxis = FVector::RightVector;
    }
    else if (Extent.Z <= Extent.X && Extent.Z <= Extent.Y)
    {
        LocalAxis = FVector::UpVector;
    }

    const FTransform SteeringWorld = SourceSteering->GetComponentTransform();
    const FVector PivotWorld = SteeringWorld.TransformPosition(LocalBounds.Origin);
    const FVector AxisWorld =
        SteeringWorld.TransformVectorNoScale(LocalAxis).GetSafeNormal();
    const FTransform ShellWorld = VehicleVisualShell->GetComponentTransform();
    const FVector PivotLocal = ShellWorld.InverseTransformPosition(PivotWorld);
    const FVector AxisLocal =
        ShellWorld.InverseTransformVectorNoScale(AxisWorld).GetSafeNormal();
    if (AxisLocal.IsNearlyZero())
    {
        return false;
    }

    USceneComponent* Pivot = NewObject<USceneComponent>(this);
    if (!Pivot) return false;
    AddInstanceComponent(Pivot);
    Pivot->SetupAttachment(VehicleVisualShell);
    Pivot->SetMobility(EComponentMobility::Movable);
    Pivot->SetRelativeLocation(PivotLocal);
    Pivot->SetRelativeRotation(FRotationMatrix::MakeFromX(AxisLocal).Rotator());
    Pivot->RegisterComponent();

    // Keep the source wheel exactly where the artist put it; only its parent
    // changes so subsequent rotation occurs around the wheel hub/column.
    SourceSteering->AttachToComponent(Pivot, FAttachmentTransformRules::KeepWorldTransform);
    SourceSteeringPivot = Pivot;
    CockpitVisualDriver->SetSteeringVisualComponent(Pivot);
    return true;
}

bool APinkCabChaosTatraPawn::ApplyVehicleVisualProfile(const FPinkCabVehicleVisualProfile& Profile)
{
    if (!VehicleVisualShell || !CockpitAssembly || !CockpitVisualDriver || !Profile.IsValid()) return false;
    const FPinkCabVehicleVisualProfile Previous = VehicleVisualShell->GetProfile();

    CockpitVisualDriver->SetSteeringVisualComponent(nullptr);
    if (SourceSteeringPivot)
    {
        SourceSteeringPivot->DestroyComponent();
        SourceSteeringPivot = nullptr;
    }

    if (!VehicleVisualShell->ApplyProfile(Profile)) return false;
    if (!CockpitAssembly->ApplyVisualBindings(Profile.CockpitBindings))
    {
        VehicleVisualShell->ApplyProfile(Previous);
        ConfigureSourceSteeringVisual(Previous);
        return false;
    }
    CockpitAssembly->SetGeneratedVisualMode(!Profile.HasVisualAsset(), Profile.CockpitBindings);

    if (!ConfigureSourceSteeringVisual(Profile))
    {
        VehicleVisualShell->ApplyProfile(Previous);
        CockpitAssembly->ApplyVisualBindings(Previous.CockpitBindings);
        CockpitAssembly->SetGeneratedVisualMode(!Previous.HasVisualAsset(), Previous.CockpitBindings);
        ConfigureSourceSteeringVisual(Previous);
        return false;
    }

    CockpitAssembly->SetRelativeTransform(Profile.CockpitRootTransform);
    DriverHeadRoot->SetRelativeTransform(Profile.DriverHeadTransform);
    CockpitVisualDriver->InvalidateBaseTransforms();
    if (USkeletalMeshComponent* VehicleMesh = GetMesh())
    {
        VehicleMesh->SetVisibility(!(Profile.HasVisualAsset() && Profile.bHidePhysicsChassisWhenExteriorPresent), false);
    }
    return true;
}

bool APinkCabChaosTatraPawn::SyncLoadToChaos()
{
    UChaosWheeledVehicleMovementComponent* Movement = GetChaosMovement();
    return Movement && FPinkCabChaosLoadBridge::Apply(VehicleLoadState, TatraProfile, *Movement);
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
    if (!VehicleHealthService.ApplyHit(GetMutableVehicleHealthState(), Event)) return false;
    SyncCockpitToChaos();
    if (!VehicleHealthService.HasCapability(GetVehicleHealthState(), EPinkCabVehicleCapability::RunEngine))
    {
        if (UChaosWheeledVehicleMovementComponent* Movement = GetChaosMovement())
        {
            Movement->EnableMechanicalSim(false);
        }
    }
    return true;
}

bool APinkCabChaosTatraPawn::CaptureVehicleSnapshot(FPinkCabVehicleStateSnapshot& OutSnapshot) const
{
    return FPinkCabVehicleStateSnapshotCodec::Capture(GetVehicleHealthState(), VehicleLoadState, OutSnapshot);
}

bool APinkCabChaosTatraPawn::RestoreVehicleSnapshot(const FPinkCabVehicleStateSnapshot& Snapshot)
{
    FPinkCabVehicleHealthState RestoredHealth;
    FPinkCabVehicleLoadState RestoredLoad;
    if (!FPinkCabVehicleStateSnapshotCodec::Restore(Snapshot, RestoredHealth, RestoredLoad)) return false;

    FPinkCabVehicleHealthState& MutableHealth = GetMutableVehicleHealthState();
    const FPinkCabVehicleHealthState BeforeHealth = MutableHealth;
    const FPinkCabVehicleLoadState BeforeLoad = VehicleLoadState;
    MutableHealth = RestoredHealth;
    VehicleLoadState = RestoredLoad;
    if (!SyncLoadToChaos())
    {
        MutableHealth = BeforeHealth;
        VehicleLoadState = BeforeLoad;
        SyncLoadToChaos();
        return false;
    }
    SyncCockpitToChaos();
    return true;
}

void APinkCabChaosTatraPawn::ApplyMouseSteeringDelta(
    const float DeltaX,
    const bool bGazeHeld,
    const float DeltaSeconds)
{
    VehicleControlRuntime.ApplySteering(DeltaX, bGazeHeld, DeltaSeconds);
}

void APinkCabChaosTatraPawn::ApplyPhysicalControlMouseDelta(
    const FName TargetId,
    const bool bGripHeld,
    const float MouseDeltaX,
    const float MouseDeltaY,
    const float DeltaSeconds)
{
    VehicleControlRuntime.ApplyPhysicalControl(
        TargetId, bGripHeld, MouseDeltaX, MouseDeltaY, DeltaSeconds, CockpitState);
    GearLeverCursor = VehicleControlRuntime.GetGearLeverCursor();
    SyncCockpitToChaos();
}

void APinkCabChaosTatraPawn::ApplyVehicleInputFrame(
    const FPinkCabVehicleInputFrame& InputFrame,
    const float MouseDeltaX,
    const float DeltaSeconds)
{
    VehicleControlRuntime.ResolveControlFrame(
        InputFrame, MouseDeltaX, DeltaSeconds, CockpitState, GetMutableVehicleHealthState());
    if (CockpitInteraction)
    {
        CockpitInteraction->SetGazeHeld(InputFrame.bGazeHeld);
    }
    SyncCockpitToChaos();
}

bool APinkCabChaosTatraPawn::BeginDriverFrame(
    APlayerController& PC,
    FPinkCabPlayerInputSample& OutInput)
{
    if (DriverUi)
    {
        DriverUi->EnsurePlayableHudMounted();
    }
    OutInput = FPinkCabPlayerInputAdapter().Capture(PC, InputRouter);
    if (OutInput.bSystemMenuToggleRequested)
    {
        SetSystemMenuOpen(!IsSystemMenuOpen());
    }
    if (!UGameplayStatics::IsGamePaused(this))
    {
        return true;
    }
    ResetTransientCockpitInput();
    return false;
}

FPinkCabVehicleInputFrame APinkCabChaosTatraPawn::PrepareVehicleFrame(
    const FPinkCabPlayerInputSample& PlayerInput,
    const float DeltaSeconds,
    EPinkCabPedalWheelRecipient& OutWheelRecipient)
{
    const FPinkCabVehicleInputFrame RawInputFrame = FPinkCabVehicleInputFrame::FromDigital(
        PlayerInput.bGazeHeld,
        PlayerInput.bClutchHeld,
        PlayerInput.bBrakeHeld,
        PlayerInput.bThrottleHeld);
    TOptional<FPinkCabVehicleTelemetry> MotionTelemetry;
    FPinkCabVehicleTelemetry TelemetrySample;
    if (DynamicsProvider.ReadTelemetry(TelemetrySample))
    {
        MotionTelemetry = TelemetrySample;
    }
    const FPinkCabPreparedVehicleControlFrame PreparedControl =
        VehicleControlRuntime.PrepareInputFrame(
            RawInputFrame,
            PlayerInput.WheelSteps,
            MotionTelemetry,
            CockpitState,
            DeltaSeconds);
    OutWheelRecipient = PreparedControl.WheelRecipient;
    return PreparedControl.Frame;
}

bool APinkCabChaosTatraPawn::ProcessCockpitFrame(
    APlayerController& PC,
    const FPinkCabPlayerInputSample& PlayerInput,
    const EPinkCabPedalWheelRecipient WheelRecipient,
    const float DeltaSeconds)
{
    FPinkCabCockpitInteractionFrame InteractionFrame;
    InteractionFrame.bGazeHeld = PlayerInput.bGazeHeld;
    InteractionFrame.bQuickRecall1Held = PlayerInput.bQuickRecall1Held;
    InteractionFrame.bQuickRecall2Held = PlayerInput.bQuickRecall2Held;
    InteractionFrame.bQuickRecall3Held = PlayerInput.bQuickRecall3Held;
    InteractionFrame.bQuickRecall4Held = PlayerInput.bQuickRecall4Held;
    InteractionFrame.bGripHeld = PlayerInput.bGripHeld;
    InteractionFrame.bMomentaryHeld = PlayerInput.bMomentaryHeld;
    InteractionFrame.WheelSteps =
        WheelRecipient == EPinkCabPedalWheelRecipient::None ? PlayerInput.WheelSteps : 0;
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

    const FName ActiveGripTarget = CockpitInteraction->GetActiveGripTargetId();
    const bool bHandbrakeGripActive =
        CockpitInteraction->IsGripActive()
        && ActiveGripTarget == FName(TEXT("Handbrake"));
    const bool bGearboxGripActive =
        CockpitInteraction->IsGripActive()
        && ActiveGripTarget == FName(TEXT("Gearbox"));
    const bool bPhysicalGripActive = bHandbrakeGripActive || bGearboxGripActive;
    if (DriverUi)
    {
        DriverUi->SetPointerCapture(PC, bPhysicalGripActive);
    }
    ApplyPhysicalControlMouseDelta(
        ActiveGripTarget,
        CockpitInteraction->IsGripActive(),
        PlayerInput.DeviceX,
        PlayerInput.DeviceY,
        DeltaSeconds);
    bGearLeverDragging = bGearboxGripActive;
    GearLeverCursor = bGearLeverDragging
        ? VehicleControlRuntime.GetGearLeverCursor()
        : UPinkCabCockpitVisualDriverComponent::GearCursorForGear(CockpitState.GetSelectedGear());
    return bPhysicalGripActive;
}

void APinkCabChaosTatraPawn::UpdateDriverLook(
    const FPinkCabPlayerInputSample& PlayerInput,
    const bool bPhysicalGripActive,
    const float DeltaSeconds)
{
    const bool bGazeHeld = CockpitInteraction->IsGazeHeld() && !bPhysicalGripActive;
    SmoothedLookMouseX = FMath::FInterpTo(
        SmoothedLookMouseX, bGazeHeld ? PlayerInput.LookMouseX : 0.0f, DeltaSeconds, 12.0f);
    SmoothedLookMouseY = FMath::FInterpTo(
        SmoothedLookMouseY, bGazeHeld ? PlayerInput.LookMouseY : 0.0f, DeltaSeconds, 12.0f);
    if (bGazeHeld)
    {
        LookYaw = FMath::Clamp(LookYaw + SmoothedLookMouseX * 0.58f, -130.0f, 130.0f);
        LookPitch = FMath::Clamp(LookPitch + SmoothedLookMouseY * 0.46f, -58.0f, 48.0f);
    }
    else
    {
        LookYaw = FMath::FInterpTo(LookYaw, 0.0f, DeltaSeconds, 3.2f);
        LookPitch = FMath::FInterpTo(LookPitch, 0.0f, DeltaSeconds, 3.2f);
    }
    DriverHeadRoot->SetRelativeRotation(FRotator(LookPitch, LookYaw, 0.0f));
}

FPinkCabCockpitPresentationState APinkCabChaosTatraPawn::BuildCockpitPresentation(
    const float DeltaSeconds)
{
    FPinkCabCockpitPresentationState Presentation;
    VisualSteering = VehicleControlRuntime.GetControlState().Steering;
    Presentation.Steering = VisualSteering;
    Presentation.Clutch = VehicleControlRuntime.GetDisplayedClutchPedal();
    Presentation.Brake = VehicleControlRuntime.GetDisplayedBrakePedal();
    Presentation.Throttle = VehicleControlRuntime.GetDisplayedThrottlePedal();
    Presentation.SelectedGear = CockpitState.GetSelectedGear();
    Presentation.bGearLeverDragging = bGearLeverDragging;
    Presentation.GearLeverCursor = GearLeverCursor;
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
    }
    Presentation.EngineRpm = VehicleControlRuntime.GetDisplayedEngineRpm();
    const float Rpm01 = FMath::Clamp(Presentation.EngineRpm / 7000.0f, 0.0f, 1.0f);
    const float TemperatureTarget = Presentation.bIgnitionRunning
        ? FMath::Lerp(0.62f, 0.90f, Rpm01)
        : 0.12f;
    EngineTemperature01 = FMath::FInterpTo(
        EngineTemperature01,
        TemperatureTarget,
        DeltaSeconds,
        Presentation.bIgnitionRunning ? 0.08f : 0.03f);
    Presentation.EngineTemperature01 = EngineTemperature01;
    Presentation.Fuel01 = TatraProfile.FullFuelMassKg > KINDA_SMALL_NUMBER
        ? FMath::Clamp(VehicleLoadState.GetFuelMassKg() / TatraProfile.FullFuelMassKg, 0.0f, 1.0f)
        : 0.0f;
    return Presentation;
}

void APinkCabChaosTatraPawn::UpdateDriverUiState(
    const FPinkCabCockpitPresentationState& Presentation)
{
    if (!DriverUi)
    {
        return;
    }
    FPinkCabDriverUiState UiState;
    UiState.CurrentTargetId = CockpitInteraction ? CockpitInteraction->GetCurrentTargetId() : NAME_None;
    UiState.GearLeverCursor = GearLeverCursor;
    UiState.SpeedKmh = Presentation.SpeedKmh;
    UiState.EngineRpm = Presentation.EngineRpm;
    UiState.Clutch = Presentation.Clutch;
    UiState.Brake = Presentation.Brake;
    UiState.Throttle = Presentation.Throttle;
    UiState.Handbrake = VehicleControlRuntime.GetHandbrakeCommand();
    UiState.Steering = Presentation.Steering;
    UiState.Fuel01 = Presentation.Fuel01;
    UiState.EngineTemperature01 = Presentation.EngineTemperature01;
    UiState.BrakeTemperature01 = GetVehicleHealthState().GetBrakeTemperature01();
    UiState.ClutchTemperature01 = GetVehicleHealthState().GetClutchTemperature01();
    UiState.RequestedGear = VehicleControlRuntime.GetRequestedGear();
    UiState.EngagedGear = VehicleControlRuntime.GetEngagedGear();
    UiState.bGearLeverDragging = bGearLeverDragging;
    UiState.bEngineRunning = CockpitState.GetIgnitionState() == EPinkCabIgnitionState::Running;
    UiState.bEngineStalled = CockpitState.GetIgnitionState() == EPinkCabIgnitionState::Stalled;
    UiState.bMoving = GetMotionMode() == EPinkCabVehicleMotionMode::Moving;
    UiState.bRequiresThrottleDose = VehicleControlRuntime.RequiresThrottleDose();
    UiState.bParkingHandbrakeLatched = VehicleControlRuntime.IsParkingHandbrakeLatched();
    UiState.GearResult = VehicleControlRuntime.GetLastGearResult();
    DriverUi->UpdateState(UiState);
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

    FPinkCabPlayerInputSample PlayerInput;
    if (!BeginDriverFrame(*PC, PlayerInput))
    {
        return;
    }
    EPinkCabPedalWheelRecipient WheelRecipient = EPinkCabPedalWheelRecipient::None;
    const FPinkCabVehicleInputFrame InputFrame =
        PrepareVehicleFrame(PlayerInput, DeltaSeconds, WheelRecipient);
    const bool bPhysicalGripActive =
        ProcessCockpitFrame(*PC, PlayerInput, WheelRecipient, DeltaSeconds);
    const float SteeringMouseX = FPinkCabPhysicalInputConvention::SteeringRight(PlayerInput.DeviceX);
    ApplyVehicleInputFrame(
        InputFrame,
        bPhysicalGripActive ? 0.0f : SteeringMouseX,
        DeltaSeconds);
    UpdateDriverLook(PlayerInput, bPhysicalGripActive, DeltaSeconds);
    const FPinkCabCockpitPresentationState Presentation = BuildCockpitPresentation(DeltaSeconds);
    CockpitVisualDriver->Apply(*CockpitAssembly, Presentation);
    UpdateDriverUiState(Presentation);
}

UChaosWheeledVehicleMovementComponent* APinkCabChaosTatraPawn::GetChaosMovement() const
{
    return Cast<UChaosWheeledVehicleMovementComponent>(GetVehicleMovementComponent());
}
