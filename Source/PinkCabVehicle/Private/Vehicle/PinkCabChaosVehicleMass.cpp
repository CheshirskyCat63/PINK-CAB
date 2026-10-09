#include "Vehicle/PinkCabChaosVehicleMovementComponent.h"

#include "Components/PrimitiveComponent.h"
#include "GameFramework/Actor.h"
#include "Physics/PhysicsInterfaceCore.h"
#include "PhysicsEngine/BodyInstance.h"

namespace
{
FBodyInstance* MassBody(UActorComponent& Component)
{
    const AActor* Owner = Component.GetOwner();
    UPrimitiveComponent* Root = Owner ? Cast<UPrimitiveComponent>(Owner->GetRootComponent()) : nullptr;
    return Root ? Root->GetBodyInstance() : nullptr;
}
}

bool UPinkCabChaosVehicleMovementComponent::ConfigurePinkCabMass(
    const FPinkCabVehicleMassProperties& InProperties)
{
    if (!InProperties.IsValid()) return false;
    MassProperties = InProperties;
    Mass = MassProperties.MassKg;
    bEnableCenterOfMassOverride = true;
    CenterOfMassOverride = MassProperties.CenterCm;
    if (FBodyInstance* Body = MassBody(*this))
    {
        Body->UpdateMassProperties();
        ApplyPinkCabMassProperties(Body);
    }
    return true;
}

void UPinkCabChaosVehicleMovementComponent::SetupVehicleMass()
{
    Super::SetupVehicleMass();
    if (FBodyInstance* Body = MassBody(*this))
    {
        // Install after the native mass callback; retain the full mass frame when
        // native physics recalculates/recreates this body's collision properties.
        Body->OnRecalculatedMassProperties().Remove(MassRecalculationHandle);
        MassRecalculationHandle = Body->OnRecalculatedMassProperties().AddUObject(
            this, &UPinkCabChaosVehicleMovementComponent::ApplyPinkCabMassProperties);
        ApplyPinkCabMassProperties(Body);
    }
}

void UPinkCabChaosVehicleMovementComponent::ApplyPinkCabMassProperties(FBodyInstance* Body)
{
    if (!Body || !Body->IsValidBodyInstance() || !MassProperties.IsValid()) return;
    const FPinkCabVehicleMassProperties Properties = MassProperties;
    FPhysicsCommand::ExecuteWrite(Body->ActorHandle, [&Properties](const FPhysicsActorHandle& Actor)
    {
        FPhysicsInterface::SetMass_AssumesLocked(Actor, Properties.MassKg);
        FPhysicsInterface::SetComLocalPose_AssumesLocked(Actor,
            FTransform(Properties.PrincipalRotation, Properties.CenterCm));
        FPhysicsInterface::SetMassSpaceInertiaTensor_AssumesLocked(Actor, Properties.PrincipalInertiaKgCm2);
    });
}
