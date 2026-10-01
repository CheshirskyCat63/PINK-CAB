#pragma once

#include "CoreMinimal.h"
#include "WheeledVehiclePawn.h"
#include "PinkCabPhysicsFixturePawn.generated.h"

class UChaosWheeledVehicleMovementComponent;
class APinkCabPhysicsFixturePawn;

struct FPinkCabPhysicsFixtureRestObservation
{
    int64 MechanicalStep = 0;
    int64 ElapsedMechanicalSteps = 0;
    float EngineRpm = 0.0f;
    float TargetIdleRpm = 0.0f;
    float MaxDrivenWheelRpm = 0.0f;
    float BodyLinearSpeedCmPerSec = 0.0f;
    float BodyAngularSpeedDegPerSec = 0.0f;
    int32 NativeCurrentGear = 0;
    int32 NativeTargetGear = 0;
};

class FPinkCabPhysicsFixtureRestGate
{
public:
    void Reset();
    bool Update(APinkCabPhysicsFixturePawn& Pawn);

    const FPinkCabPhysicsFixtureRestObservation& GetObservation() const
    {
        return Observation;
    }

    int32 GetStableMechanicalSteps() const
    {
        return StableMechanicalSteps;
    }

private:
    FPinkCabPhysicsFixtureRestObservation Observation;
    int64 StartMechanicalStep = -1;
    int64 LastMechanicalStep = -1;
    int32 StableMechanicalSteps = 0;
};

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
    APinkCabPhysicsFixturePawn* SpawnFreshPawn(
        UWorld& World,
        const FVector& Location,
        const FRotator& Rotation);
    AActor* FindOrSpawnFlatFloor(UWorld& World);
    void KeepAwake(APinkCabPhysicsFixturePawn& Pawn);
    void DestroyPawns(UWorld& World);
}
