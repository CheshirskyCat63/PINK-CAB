#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "World/PinkCabCityIdentity.h"
#include "World/PinkCabL1EndlessRoadModel.h"
#include "PinkCabL1EndlessRoadStreamer.generated.h"

class APinkCabL1RoadChunkActor;

UCLASS()
class PINKCAB_API APinkCabL1EndlessRoadStreamer : public AActor
{
    GENERATED_BODY()

public:
    APinkCabL1EndlessRoadStreamer();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;

    bool RefreshForState(const FVector& WorldLocation, const FVector& WorldVelocity);
    void SetTrackedActor(AActor* Actor) { TrackedActor = Actor; }
    AActor* GetTrackedActor() const { return TrackedActor.Get(); }

    int32 GetCurrentChunkIndex() const { return CurrentChunkIndex; }
    int32 GetActiveChunkCount() const;
    int32 GetPoolSize() const { return ChunkPool.Num(); }
    TArray<FPinkCabChunkId> GetActiveChunkIds() const;
    TArray<int32> GetActiveChunkIndices() const;
    APinkCabL1RoadChunkActor* FindActiveChunkActor(int32 ChunkIndex) const;

    EPinkCabLongitudinalTravelDirection GetStableTravelDirection() const
    {
        return StableTravelDirection;
    }

    const FPinkCabCityIdentity& GetCityIdentity() const { return CityIdentity; }
    const FString& GetLastMaterializationSignature() const
    {
        return LastMaterializationSignature;
    }

private:
    bool EnsurePool();

    UPROPERTY(EditInstanceOnly, Category = "PinkCab|World|L1")
    TObjectPtr<AActor> TrackedActor;

    UPROPERTY(EditDefaultsOnly, Category = "PinkCab|World|L1")
    TSubclassOf<APinkCabL1RoadChunkActor> RoadChunkClass;

    UPROPERTY(Transient)
    TArray<TObjectPtr<APinkCabL1RoadChunkActor>> ChunkPool;

    UPROPERTY(EditAnywhere, Category = "PinkCab|World|L1", meta = (ClampMin = "0.0"))
    double DirectionThresholdCmPerSec = 25.0;

    FPinkCabCityIdentity CityIdentity;
    EPinkCabLongitudinalTravelDirection StableTravelDirection =
        EPinkCabLongitudinalTravelDirection::Positive;
    int32 CurrentChunkIndex = INDEX_NONE;
    FString LastMaterializationSignature;
    bool bHasMaterializedState = false;
};
