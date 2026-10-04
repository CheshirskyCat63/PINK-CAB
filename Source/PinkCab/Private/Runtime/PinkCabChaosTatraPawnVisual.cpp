#include "Runtime/PinkCabChaosTatraPawn.h"
#include "Runtime/PinkCabVehicleVisualProfile.h"
#include "Runtime/PinkCabVehicleVisualShellComponent.h"
#include "ChaosVehicleWheel.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "Cockpit/PinkCabCockpitAssemblyComponent.h"
#include "Cockpit/PinkCabCockpitVisualDriverComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/PoseableMeshComponent.h"
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

namespace
{
void ResetPoseBones(UPoseableMeshComponent& Mesh, const TArray<FName>& Bones)
{
    for (const FName Bone : Bones)
    {
        if (Mesh.GetBoneIndex(Bone) != INDEX_NONE)
        {
            Mesh.ResetBoneTransformByName(Bone);
        }
    }
    Mesh.RefreshBoneTransforms();
}

void RotatePoseBone(
    UPoseableMeshComponent& Mesh,
    const FName Bone,
    const FQuat& Delta)
{
    if (Mesh.GetBoneIndex(Bone) == INDEX_NONE)
    {
        return;
    }
    FTransform Transform = Mesh.GetBoneTransformByName(
        Bone,
        EBoneSpaces::ComponentSpace);
    Transform.SetRotation((Delta * Transform.GetRotation()).GetNormalized());
    Mesh.SetBoneTransformByName(
        Bone,
        Transform,
        EBoneSpaces::ComponentSpace);
}
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


void APinkCabChaosTatraPawn::ApplyPrototypeDriverPose(const float Steering)
{
    if (!PrototypeDriverVisual || !PrototypeDriverVisual->GetSkinnedAsset())
    {
        return;
    }

    const float ClampedSteering = FMath::Clamp(Steering, -1.0f, 1.0f);
    if (bPrototypeDriverPoseInitialized
        && FMath::Abs(ClampedSteering - LastPrototypeDriverSteering) < 0.0125f)
    {
        return;
    }

    static const TArray<FName> PoseBones = {
        TEXT("pelvis"), TEXT("spine_01"), TEXT("spine_02"), TEXT("spine_03"),
        TEXT("thigh_l"), TEXT("thigh_r"), TEXT("calf_l"), TEXT("calf_r"),
        TEXT("foot_l"), TEXT("foot_r"),
        TEXT("clavicle_l"), TEXT("clavicle_r"),
        TEXT("upperarm_l"), TEXT("upperarm_r"),
        TEXT("lowerarm_l"), TEXT("lowerarm_r"),
        TEXT("hand_l"), TEXT("hand_r")
    };
    ResetPoseBones(*PrototypeDriverVisual, PoseBones);

    const auto AxisRotation = [](const FVector& Axis, const float Degrees)
    {
        return FQuat(Axis, FMath::DegreesToRadians(Degrees));
    };

    // Cheap placeholder driver: pelvis/back lean and a stable seated leg chain.
    // This stays presentation-only; the final heroine animation can replace it
    // without touching the vehicle/control contracts.
    RotatePoseBone(*PrototypeDriverVisual, TEXT("pelvis"),
        AxisRotation(FVector::RightVector, 4.0f));
    RotatePoseBone(*PrototypeDriverVisual, TEXT("spine_01"),
        AxisRotation(FVector::RightVector, -6.0f));
    RotatePoseBone(*PrototypeDriverVisual, TEXT("spine_02"),
        AxisRotation(FVector::RightVector, -4.0f));
    PrototypeDriverVisual->RefreshBoneTransforms();

    RotatePoseBone(*PrototypeDriverVisual, TEXT("thigh_l"),
        AxisRotation(FVector::RightVector, -73.0f));
    RotatePoseBone(*PrototypeDriverVisual, TEXT("thigh_r"),
        AxisRotation(FVector::RightVector, -73.0f));
    PrototypeDriverVisual->RefreshBoneTransforms();

    RotatePoseBone(*PrototypeDriverVisual, TEXT("calf_l"),
        AxisRotation(FVector::RightVector, 74.0f));
    RotatePoseBone(*PrototypeDriverVisual, TEXT("calf_r"),
        AxisRotation(FVector::RightVector, 74.0f));
    RotatePoseBone(*PrototypeDriverVisual, TEXT("foot_l"),
        AxisRotation(FVector::RightVector, 8.0f));
    RotatePoseBone(*PrototypeDriverVisual, TEXT("foot_r"),
        AxisRotation(FVector::RightVector, 8.0f));
    PrototypeDriverVisual->RefreshBoneTransforms();

    // Bring both arms from the mannequin A-pose toward the wheel. Steering adds
    // a small opposite hand-height/forearm bias so the dummy visibly follows
    // the actual cockpit steering state instead of remaining a statue.
    const float WheelBias = 13.0f * ClampedSteering;
    const FQuat LeftShoulder =
        AxisRotation(FVector::UpVector, 67.0f + WheelBias * 0.25f)
        * AxisRotation(FVector::RightVector, 18.0f);
    const FQuat RightShoulder =
        AxisRotation(FVector::UpVector, -67.0f + WheelBias * 0.25f)
        * AxisRotation(FVector::RightVector, 18.0f);
    RotatePoseBone(*PrototypeDriverVisual, TEXT("upperarm_l"), LeftShoulder);
    RotatePoseBone(*PrototypeDriverVisual, TEXT("upperarm_r"), RightShoulder);
    PrototypeDriverVisual->RefreshBoneTransforms();

    RotatePoseBone(*PrototypeDriverVisual, TEXT("lowerarm_l"),
        AxisRotation(FVector::UpVector, -42.0f + WheelBias));
    RotatePoseBone(*PrototypeDriverVisual, TEXT("lowerarm_r"),
        AxisRotation(FVector::UpVector, 42.0f + WheelBias));
    RotatePoseBone(*PrototypeDriverVisual, TEXT("hand_l"),
        AxisRotation(FVector::ForwardVector, -12.0f - WheelBias * 0.6f));
    RotatePoseBone(*PrototypeDriverVisual, TEXT("hand_r"),
        AxisRotation(FVector::ForwardVector, 12.0f - WheelBias * 0.6f));
    PrototypeDriverVisual->RefreshBoneTransforms();

    LastPrototypeDriverSteering = ClampedSteering;
    bPrototypeDriverPoseInitialized = true;
}

bool APinkCabChaosTatraPawn::SyncWheelPresentationFromChaos()
{
    if (!VehicleVisualShell)
    {
        return false;
    }

    UChaosWheeledVehicleMovementComponent* Movement = GetChaosMovement();
    if (!Movement
        || Movement->Wheels.Num() != 4
        || ActiveWheelPresentationPartIds.Num() != 4)
    {
        return false;
    }

    const FPinkCabVehicleVisualProfile& Profile = VehicleVisualShell->GetProfile();

    for (int32 Index = 0; Index < 4; ++Index)
    {
        const FName PartId = ActiveWheelPresentationPartIds[Index];
        UChaosVehicleWheel* ChaosWheel = Movement->Wheels[Index];
        UStaticMeshComponent* VisualWheel =
            VehicleVisualShell->GetPresentationPartComponent(PartId);
        const FPinkCabVehiclePresentationPart* Part =
            Profile.PresentationParts.FindByPredicate(
                [PartId](const FPinkCabVehiclePresentationPart& Candidate)
                {
                    return Candidate.PartId == PartId;
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
    if (!SourceSteering)
    {
        return false;
    }

    const FVector PivotLocal = Profile.SteeringPresentationPivot;
    const FVector AxisLocal = Profile.SteeringPresentationAxis.GetSafeNormal();
    if (PivotLocal.ContainsNaN()
        || Profile.SteeringPresentationAxis.ContainsNaN()
        || AxisLocal.IsNearlyZero())
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
