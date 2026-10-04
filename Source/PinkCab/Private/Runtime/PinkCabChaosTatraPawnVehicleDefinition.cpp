#include "Runtime/PinkCabChaosTatraPawn.h"

#include "ChaosWheeledVehicleMovementComponent.h"
#include "Components/SkinnedMeshComponent.h"
#include "Components/PoseableMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "Vehicle/PinkCabVehicleDefinition.h"

namespace
{
const FPinkCabVehicleWheelBinding* FindWheelBinding(
    const UPinkCabVehicleDefinition& Definition,
    const FName WheelId)
{
    return Definition.Wheels.FindByPredicate(
        [WheelId](const FPinkCabVehicleWheelBinding& Wheel)
        {
            return Wheel.WheelId == WheelId;
        });
}

bool ResolveRuntimeWheelBindings(
    const UPinkCabVehicleDefinition& Definition,
    const FPinkCabVehicleWheelBinding* (&OutBindings)[4])
{
    static const FName WheelIds[4] = {
        TEXT("WheelFL"), TEXT("WheelFR"), TEXT("WheelRL"), TEXT("WheelRR")};
    for (int32 Index = 0; Index < 4; ++Index)
    {
        OutBindings[Index] = FindWheelBinding(Definition, WheelIds[Index]);
        if (!OutBindings[Index])
        {
            return false;
        }
    }
    return true;
}

void ApplyCarrierAssets(
    USkeletalMeshComponent& VehicleMesh,
    USkeletalMesh& CarrierMesh,
    UPhysicsAsset& CarrierPhysics)
{
    if (VehicleMesh.GetSkeletalMeshAsset() != &CarrierMesh)
    {
        VehicleMesh.SetSkeletalMesh(&CarrierMesh);
    }
    if (VehicleMesh.GetPhysicsAsset() != &CarrierPhysics)
    {
        VehicleMesh.SetPhysicsAsset(&CarrierPhysics, false);
    }
    VehicleMesh.SetCollisionProfileName(TEXT("Vehicle"));
    VehicleMesh.SetSimulatePhysics(true);
    VehicleMesh.SetOwnerNoSee(true);
}

void ApplyWheelBindings(
    UChaosWheeledVehicleMovementComponent& Movement,
    const FPinkCabVehicleWheelBinding* const (&Bindings)[4])
{
    for (int32 Index = 0; Index < 4; ++Index)
    {
        Movement.WheelSetups[Index].BoneName = Bindings[Index]->BoneName;
        Movement.WheelSetups[Index].AdditionalOffset = FVector::ZeroVector;
    }
}
}

bool APinkCabChaosTatraPawn::ApplyVehicleDefinition(
    const UPinkCabVehicleDefinition& Definition)
{
    FString FailureReason;
    if (!Definition.IsValid(&FailureReason))
    {
        UE_LOG(LogTemp, Warning,
            TEXT("PINKCAB_VEHICLE_DEFINITION_REJECTED id=%s reason=%s"),
            *Definition.VehicleId.ToString(),
            *FailureReason);
        return false;
    }

    USkeletalMeshComponent* VehicleMesh = GetMesh();
    UChaosWheeledVehicleMovementComponent* Movement = GetChaosMovement();
    if (!VehicleMesh || !Movement || Movement->WheelSetups.Num() != 4)
    {
        return false;
    }

    USkeletalMesh* CarrierMesh = Definition.PhysicsCarrierMesh.LoadSynchronous();
    UPhysicsAsset* CarrierPhysics = Definition.PhysicsAsset.LoadSynchronous();
    const FPinkCabVehicleWheelBinding* WheelBindings[4] = {};
    if (!CarrierMesh
        || !CarrierPhysics
        || !ResolveRuntimeWheelBindings(Definition, WheelBindings))
    {
        return false;
    }

    if (!ApplyVehicleVisualProfile(Definition.BuildVisualProfile()))
    {
        return false;
    }

    ApplyCarrierAssets(*VehicleMesh, *CarrierMesh, *CarrierPhysics);
    ApplyWheelBindings(*Movement, WheelBindings);
    ActiveWheelPresentationPartIds.Reset(4);
    for (int32 Index = 0; Index < 4; ++Index)
    {
        ActiveWheelPresentationPartIds.Add(WheelBindings[Index]->PresentationPartId);
    }

    if (USkinnedMeshComponent* DriverVisual = PrototypeDriverVisual)
    {
        if (!Definition.DriverMesh.IsNull())
        {
            if (USkeletalMesh* DriverMesh = Definition.DriverMesh.LoadSynchronous())
            {
                DriverVisual->SetSkinnedAssetAndUpdate(DriverMesh, true);
            }
        }
        DriverVisual->SetRelativeTransform(Definition.DriverTransform);
    }

    ActiveVehicleDefinitionId = Definition.VehicleId;
    return true;
}
