#include "Vehicle/PinkCabChaosLoadBridge.h"

#include "ChaosWheeledVehicleMovementComponent.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/Actor.h"
#include "PhysicsEngine/BodyInstance.h"
#include "Vehicle/PinkCabTatraProfile.h"
#include "Vehicle/PinkCabVehicleLoadState.h"
#include "Vehicle/PinkCabVehicleMassProperties.h"
#include "Vehicle/PinkCabChaosVehicleMovementComponent.h"

bool FPinkCabChaosLoadBridge::Apply(
    const FPinkCabVehicleLoadState& Load,
    const FPinkCabTatraProfile& Profile,
    UChaosWheeledVehicleMovementComponent& Movement)
{
    FPinkCabVehicleMassProperties Properties;
    if (!Load.TryGetMassProperties(Profile, Properties)) return false;
    if (UPinkCabChaosVehicleMovementComponent* PinkCabMovement =
            Cast<UPinkCabChaosVehicleMovementComponent>(&Movement))
    {
        return PinkCabMovement->ConfigurePinkCabMass(Properties);
    }
    Movement.Mass = Properties.MassKg;
    Movement.bEnableCenterOfMassOverride = true;
    Movement.CenterOfMassOverride = Properties.CenterCm;
    AActor* Owner = Movement.GetOwner();
    if (UPrimitiveComponent* Chassis = Owner ? Cast<UPrimitiveComponent>(Owner->GetRootComponent()) : nullptr)
    {
        if (FBodyInstance* BodyInstance = Chassis->GetBodyInstance())
        {
            BodyInstance->UpdateMassProperties();
        }
    }
    return true;
}
