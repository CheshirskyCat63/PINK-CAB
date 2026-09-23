#include "Vehicle/PinkCabChaosLoadBridge.h"

#include "ChaosWheeledVehicleMovementComponent.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/Actor.h"
#include "PhysicsEngine/BodyInstance.h"
#include "Vehicle/PinkCabTatraProfile.h"
#include "Vehicle/PinkCabVehicleLoadState.h"

bool FPinkCabChaosLoadBridge::Apply(
    const FPinkCabVehicleLoadState& Load,
    const FPinkCabTatraProfile& Profile,
    UChaosWheeledVehicleMovementComponent& Movement)
{
    const float TotalMassKg = Load.GetTotalMassKg(Profile);
    const float LongitudinalCgCm = Load.GetLongitudinalCgInputCm(Profile);
    if (!FMath::IsFinite(TotalMassKg) || TotalMassKg <= KINDA_SMALL_NUMBER
        || !FMath::IsFinite(LongitudinalCgCm))
    {
        return false;
    }

    Movement.Mass = TotalMassKg;
    Movement.bEnableCenterOfMassOverride = true;
    Movement.CenterOfMassOverride.X = LongitudinalCgCm;
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
