#include "Runtime/PinkCabChaosTatraPawn.h"
#include "Runtime/PinkCabVehicleVisualProfile.h"
#include "Runtime/PinkCabVehicleVisualShellComponent.h"
#include "ChaosVehicleWheel.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "Cockpit/PinkCabCockpitPresentationState.h"
#include "Cockpit/PinkCabCockpitVisualDriverComponent.h"
#include "Components/PoseableMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Vehicle/PinkCabChaosVehicleMovementComponent.h"

namespace
{
const TMap<FName, float>& Rig06BoneOpenAngles()
{
    static const TMap<FName, float> Angles = {
        { TEXT("Door_FL"), -72.0f },
        { TEXT("Door_FR"), 72.0f },
        { TEXT("Door_RL"), -68.0f },
        { TEXT("Door_RR"), 68.0f },
        { TEXT("Trunk_Front"), -58.0f },
        { TEXT("Hood_Rear"), 58.0f },
    };
    return Angles;
}

bool IsRig06BoneProfile(const FPinkCabVehicleVisualProfile& Profile)
{
    return Profile.bUsePoseableSkeletalPresentation
        && Profile.ProfileId == FName(TEXT("PinkCab.Visual.Tatra613.Rig06.TexturedOpenables"));
}
}


bool APinkCabChaosTatraPawn::CaptureRig06RestPose()
{
    Rig06RestBoneTransforms.Reset();
    Rig06OpenableTargets.Reset();
    Rig06OpenableCurrent.Reset();
    if (!VehicleVisualShell || !IsRig06BoneProfile(VehicleVisualShell->GetProfile()))
    {
        return false;
    }

    static const FName RequiredBones[] = {
        TEXT("Phys_Wheel_FL"), TEXT("Phys_Wheel_FR"),
        TEXT("Phys_Wheel_BL"), TEXT("Phys_Wheel_BR"),
        TEXT("Steering_Wheel"), TEXT("Cabin_GearLever"),
        TEXT("Cabin_ClutchPedal"), TEXT("Cabin_BrakePedal"),
        TEXT("Cabin_ThrottlePedal"),
        TEXT("Cabin_Handbrake"), TEXT("Door_FL"), TEXT("Door_FR"),
        TEXT("Door_RL"), TEXT("Door_RR"), TEXT("Trunk_Front"),
        TEXT("Hood_Rear")
    };

    for (const FName BoneName : RequiredBones)
    {
        if (!VehicleVisualShell->ResetPoseableBoneTransform(BoneName))
        {
            Rig06RestBoneTransforms.Reset();
            return false;
        }
    }
    VehicleVisualShell->RefreshPoseableBoneTransforms();

    for (const FName BoneName : RequiredBones)
    {
        FTransform Rest;
        if (!VehicleVisualShell->GetPoseableBoneTransform(BoneName, Rest)
            || Rest.ContainsNaN())
        {
            Rig06RestBoneTransforms.Reset();
            return false;
        }
        Rig06RestBoneTransforms.Add(BoneName, Rest);
    }

    for (const TPair<FName, float>& Entry : Rig06BoneOpenAngles())
    {
        Rig06OpenableTargets.Add(Entry.Key, 0.0f);
        Rig06OpenableCurrent.Add(Entry.Key, 0.0f);
    }
    return true;
}

bool APinkCabChaosTatraPawn::ApplyRig06BoneRotation(
    const FName BoneName,
    const float AngleDegrees)
{
    if (!VehicleVisualShell)
    {
        return false;
    }
    const FTransform* Rest = Rig06RestBoneTransforms.Find(BoneName);
    if (!Rest)
    {
        return false;
    }

    FTransform Dynamic = *Rest;
    const FQuat LocalHingeRotation(
        FVector::YAxisVector,
        FMath::DegreesToRadians(AngleDegrees));
    Dynamic.SetRotation(
        (Rest->GetRotation() * LocalHingeRotation).GetNormalized());
    return VehicleVisualShell->SetPoseableBoneTransform(BoneName, Dynamic);
}

bool APinkCabChaosTatraPawn::SetTatraOpenable(
    const FName BoneName,
    const float Open01)
{
    if (!Rig06BoneOpenAngles().Contains(BoneName)
        || !Rig06RestBoneTransforms.Contains(BoneName))
    {
        return false;
    }
    Rig06OpenableTargets.FindOrAdd(BoneName) =
        FMath::Clamp(Open01, 0.0f, 1.0f);
    return true;
}

bool APinkCabChaosTatraPawn::ToggleTatraOpenable(const FName BoneName)
{
    if (!Rig06BoneOpenAngles().Contains(BoneName)
        || !Rig06RestBoneTransforms.Contains(BoneName))
    {
        return false;
    }
    const float CurrentTarget = Rig06OpenableTargets.FindRef(BoneName);
    return SetTatraOpenable(BoneName, CurrentTarget >= 0.5f ? 0.0f : 1.0f);
}

float APinkCabChaosTatraPawn::GetTatraOpenable(const FName BoneName) const
{
    return Rig06OpenableCurrent.FindRef(BoneName);
}

void APinkCabChaosTatraPawn::UpdateRig06Openables(const float DeltaSeconds)
{
    if (!VehicleVisualShell
        || !IsRig06BoneProfile(VehicleVisualShell->GetProfile())
        || Rig06RestBoneTransforms.IsEmpty())
    {
        return;
    }

    for (const TPair<FName, float>& Entry : Rig06BoneOpenAngles())
    {
        const float Target = Rig06OpenableTargets.FindRef(Entry.Key);
        float& Current = Rig06OpenableCurrent.FindOrAdd(Entry.Key);
        Current = FMath::FInterpConstantTo(
            Current,
            Target,
            FMath::Max(DeltaSeconds, 0.0f),
            1.6f);
        ApplyRig06BoneRotation(Entry.Key, Entry.Value * Current);
    }
}

bool APinkCabChaosTatraPawn::SyncRig06WheelBonesFromChaos()
{
    if (!VehicleVisualShell
        || !IsRig06BoneProfile(VehicleVisualShell->GetProfile()))
    {
        return false;
    }
    UPinkCabChaosVehicleMovementComponent* Movement = Cast<UPinkCabChaosVehicleMovementComponent>(GetChaosMovement());
    const UPoseableMeshComponent* Exterior = VehicleVisualShell->GetExteriorPoseablePresentation();
    if (!Movement || Movement->Wheels.Num() != 4 || !Exterior || !GetMesh())
    {
        return false;
    }

    static const FName WheelBones[4] = {
        TEXT("Phys_Wheel_FL"), TEXT("Phys_Wheel_FR"),
        TEXT("Phys_Wheel_BL"), TEXT("Phys_Wheel_BR")
    };

    for (int32 Index = 0; Index < 4; ++Index)
    {
        UChaosVehicleWheel* ChaosWheel = Movement->Wheels[Index];
        const FTransform* Rest = Rig06RestBoneTransforms.Find(WheelBones[Index]);
        if (!ChaosWheel || !Rest || ChaosWheel->Location.ContainsNaN())
        {
            return false;
        }

        FTransform Dynamic = *Rest;

        FVector PhysicalCenter;
        if (!Movement->GetWheelPresentationCenter(Index, PhysicalCenter)) return false;
        // Convert the one physical wheel center into authored component units.
        // This follows both suspension and chassis pose without moving physics.
        Dynamic.SetLocation(Exterior->GetComponentTransform().InverseTransformPosition(
            GetMesh()->GetComponentTransform().TransformPosition(PhysicalCenter)));

        const FQuat Steering(
            FVector::UpVector,
            FMath::DegreesToRadians(ChaosWheel->GetSteerAngle()));
        const FQuat SpinLocal(
            FVector::YAxisVector,
            FMath::DegreesToRadians(ChaosWheel->GetRotationAngle()));
        Dynamic.SetRotation(
            (Steering * Rest->GetRotation() * SpinLocal).GetNormalized());

        if (!VehicleVisualShell->SetPoseableBoneTransform(
            WheelBones[Index],
            Dynamic))
        {
            return false;
        }
    }
    return true;
}

void APinkCabChaosTatraPawn::SyncRig06CockpitBones(
    const FPinkCabCockpitPresentationState& Presentation)
{
    if (!VehicleVisualShell
        || !IsRig06BoneProfile(VehicleVisualShell->GetProfile()))
    {
        return;
    }

    // V22 authored pedal pivots share the vehicle's transverse axis. Preserve
    // their measured 24/20/28 degree downward strokes in component space;
    // the imported bone basis is not a second input or physics authority.
    const FName Pedals[] = { TEXT("Cabin_ClutchPedal"), TEXT("Cabin_BrakePedal"), TEXT("Cabin_ThrottlePedal") };
    const float Travel[] = { 24.0f, 20.0f, 28.0f };
    const float Inputs[] = { Presentation.Clutch, Presentation.Brake, Presentation.Throttle };
    for (int32 Index = 0; Index < UE_ARRAY_COUNT(Pedals); ++Index)
    {
        if (const FTransform* Rest = Rig06RestBoneTransforms.Find(Pedals[Index]))
        {
            FTransform Pedal = *Rest;
            const FQuat Press(FVector::YAxisVector,
                FMath::DegreesToRadians(-Travel[Index] * FMath::Clamp(Inputs[Index], 0.0f, 1.0f)));
            Pedal.SetRotation((Press * Rest->GetRotation()).GetNormalized());
            VehicleVisualShell->SetPoseableBoneTransform(Pedals[Index], Pedal);
        }
    }

    if (const FTransform* SteeringRest =
        Rig06RestBoneTransforms.Find(TEXT("Steering_Wheel")))
    {
        FTransform Steering = *SteeringRest;
        // RIG06 Steering_Wheel is authored around local +Y, opposite to
        // the legacy preserved-scene steering pivot. Keep semantic +right and
        // adapt only at this model boundary: +right must rotate the visible
        // wheel clockwise/right from the driver seat.
        const float AngleDegrees =
            FMath::Clamp(Presentation.Steering, -1.0f, 1.0f) * 450.0f;
        const FQuat LocalTurn(
            FVector::YAxisVector,
            FMath::DegreesToRadians(AngleDegrees));
        Steering.SetRotation(
            (SteeringRest->GetRotation() * LocalTurn).GetNormalized());
        VehicleVisualShell->SetPoseableBoneTransform(
            TEXT("Steering_Wheel"),
            Steering);
    }

    if (const FTransform* GearRest =
        Rig06RestBoneTransforms.Find(TEXT("Cabin_GearLever")))
    {
        FTransform Gear = *GearRest;
        const FVector GearOffsetCm = Presentation.bGearLeverDragging
            ? UPinkCabCockpitVisualDriverComponent::GearLeverOffsetFromCursor(
                Presentation.GearLeverCursor)
            : UPinkCabCockpitVisualDriverComponent::GearLeverOffset(
                Presentation.SelectedGear);
        // RIG06 asset-space is meters; presentation component scales it by 100.
        Gear.SetLocation(GearRest->GetLocation() + GearOffsetCm / 100.0f);
        VehicleVisualShell->SetPoseableBoneTransform(
            TEXT("Cabin_GearLever"),
            Gear);
    }

    if (const FTransform* HandbrakeRest =
        Rig06RestBoneTransforms.Find(TEXT("Cabin_Handbrake")))
    {
        FTransform Handbrake = *HandbrakeRest;
        const float AngleDegrees =
            UPinkCabCockpitVisualDriverComponent::HandbrakeAngleDegrees(
                Presentation.Handbrake);
        const FQuat LocalPull(
            FVector::YAxisVector,
            FMath::DegreesToRadians(AngleDegrees));
        Handbrake.SetRotation(
            (HandbrakeRest->GetRotation() * LocalPull).GetNormalized());
        VehicleVisualShell->SetPoseableBoneTransform(
            TEXT("Cabin_Handbrake"),
            Handbrake);
    }
}
