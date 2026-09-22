#include "Runtime/PinkCabChaosTatraPawn.h"
#include "Runtime/PinkCabVehicleVisualProfile.h"
#include "Runtime/PinkCabVehicleVisualShellComponent.h"
#include "ChaosVehicleWheel.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "Cockpit/PinkCabCockpitAssemblyComponent.h"
#include "Cockpit/PinkCabCockpitVisualDriverComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "Math/RotationMatrix.h"

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


bool APinkCabChaosTatraPawn::AlignInitialPresentationToGround()
{
    UWorld* World = GetWorld();
    if (!World || !VehicleVisualShell)
    {
        return false;
    }

    static const FName WheelIds[] = {
        TEXT("WheelFL"), TEXT("WheelFR"), TEXT("WheelRL"), TEXT("WheelRR")};
    float LowestTyreZ = TNumericLimits<float>::Max();
    for (const FName WheelId : WheelIds)
    {
        UStaticMeshComponent* Wheel = VehicleVisualShell->GetPresentationPartComponent(WheelId);
        if (!Wheel || !Wheel->IsRegistered() || !Wheel->GetStaticMesh())
        {
            return false;
        }
        Wheel->SetVisibility(true, false);
        Wheel->SetHiddenInGame(false, false);
        LowestTyreZ = FMath::Min(LowestTyreZ, Wheel->Bounds.GetBox().Min.Z);
    }
    if (!FMath::IsFinite(LowestTyreZ))
    {
        return false;
    }

    const FVector ActorLocation = GetActorLocation();
    FHitResult GroundHit;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);
    const FVector TraceStart(ActorLocation.X, ActorLocation.Y, ActorLocation.Z + 500.0f);
    const FVector TraceEnd(ActorLocation.X, ActorLocation.Y, ActorLocation.Z - 1500.0f);
    if (!World->LineTraceSingleByChannel(
            GroundHit, TraceStart, TraceEnd, ECC_Visibility, QueryParams))
    {
        return false;
    }

    constexpr float TyreClearanceCm = 1.0f;
    const float OffsetZ = GroundHit.ImpactPoint.Z + TyreClearanceCm - LowestTyreZ;
    AddActorWorldOffset(
        FVector(0.0f, 0.0f, OffsetZ), false, nullptr, ETeleportType::TeleportPhysics);

    if (USkeletalMeshComponent* PhysicsMesh = GetMesh())
    {
        PhysicsMesh->SetPhysicsLinearVelocity(FVector::ZeroVector);
        PhysicsMesh->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
    }
    return true;
}

bool APinkCabChaosTatraPawn::SyncWheelPresentationFromChaos()
{
    if (!VehicleVisualShell)
    {
        return false;
    }

    UChaosWheeledVehicleMovementComponent* Movement = GetChaosMovement();
    if (!Movement || Movement->Wheels.Num() != 4)
    {
        return false;
    }

    static const FName WheelIds[4] = {
        TEXT("WheelFL"), TEXT("WheelFR"), TEXT("WheelRL"), TEXT("WheelRR")};
    const FPinkCabVehicleVisualProfile& Profile = VehicleVisualShell->GetProfile();

    for (int32 Index = 0; Index < 4; ++Index)
    {
        UChaosVehicleWheel* ChaosWheel = Movement->Wheels[Index];
        UStaticMeshComponent* VisualWheel =
            VehicleVisualShell->GetPresentationPartComponent(WheelIds[Index]);
        const FPinkCabVehiclePresentationPart* Part =
            Profile.PresentationParts.FindByPredicate(
                [Index](const FPinkCabVehiclePresentationPart& Candidate)
                {
                    return Candidate.PartId == WheelIds[Index];
                });

        if (!ChaosWheel || !VisualWheel || !Part || ChaosWheel->Location.ContainsNaN())
        {
            return false;
        }

        const FQuat SteeringRotation(
            FVector::UpVector,
            FMath::DegreesToRadians(ChaosWheel->GetSteerAngle()));
        const FQuat SpinRotation(
            FVector::ForwardVector,
            FMath::DegreesToRadians(ChaosWheel->GetRotationAngle()));
        const FQuat DynamicRotation =
            SteeringRotation * Part->LocalTransform.GetRotation() * SpinRotation;

        // Keep the donor tyre strictly presentation-only. Chaos owns the
        // physical wheel center; the visible static mesh stays in the authored
        // vehicle-local frame and only receives the simulated suspension/steer/
        // spin pose. Never teleport a child component through world space from
        // the vehicle tick.
        const FVector DynamicLocation =
            Part->LocalTransform.GetLocation()
            + ChaosWheel->GetSuspensionAxis() * ChaosWheel->GetSuspensionOffset();
        VisualWheel->SetRelativeLocation(
            DynamicLocation,
            false,
            nullptr,
            ETeleportType::None);
        VisualWheel->SetRelativeRotation(
            DynamicRotation,
            false,
            nullptr,
            ETeleportType::None);
        VisualWheel->SetRelativeScale3D(Part->LocalTransform.GetScale3D());
        VisualWheel->SetVisibility(true, false);
        VisualWheel->SetHiddenInGame(false, false);
    }
    return true;
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
