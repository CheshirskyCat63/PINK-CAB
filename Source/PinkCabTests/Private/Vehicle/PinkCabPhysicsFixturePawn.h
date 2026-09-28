#pragma once

#include "CoreMinimal.h"
#include "WheeledVehiclePawn.h"
#include "PinkCabPhysicsFixturePawn.generated.h"

class UChaosWheeledVehicleMovementComponent;

UCLASS()
class APinkCabPhysicsFixturePawn final : public AWheeledVehiclePawn
{
    GENERATED_BODY()

public:
    explicit APinkCabPhysicsFixturePawn(
        const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

    UChaosWheeledVehicleMovementComponent* GetChaosMovement() const;
};

namespace PinkCabPhysicsFixture
{
    inline const TCHAR* MapPath = TEXT("/Engine/Maps/Entry");

    APinkCabPhysicsFixturePawn* FindOrSpawnPawn(UWorld& World);
    AActor* FindOrSpawnFlatFloor(UWorld& World);
    void KeepAwake(APinkCabPhysicsFixturePawn& Pawn);
    void DestroyPawns(UWorld& World);
}
