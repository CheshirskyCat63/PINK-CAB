#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "World/PinkCabChunkId.h"
#include "PinkCabL1RoadChunkActor.generated.h"

class USceneComponent;
class UStaticMesh;
class UStaticMeshComponent;

UCLASS()
class PINKCAB_API APinkCabL1RoadChunkActor : public AActor
{
    GENERATED_BODY()

public:
    APinkCabL1RoadChunkActor();

    bool BindChunk(
        const FPinkCabCityIdentity& City,
        int32 ChunkIndex,
        const FPinkCabChunkId& ChunkId);

    void ClearBinding();

    bool IsBound() const { return bBound; }
    bool IsVisualReady() const;
    int32 GetBoundChunkIndex() const { return BoundChunkIndex; }
    const FPinkCabChunkId& GetBoundChunkId() const { return BoundChunkId; }

    UStaticMesh* GetRoadMesh() const;
    void SetRoadMesh(UStaticMesh* Mesh);
    UStaticMeshComponent* GetRoadMeshComponent() const { return RoadMeshComponent; }

    UStaticMesh* GetConstructionMesh() const;
    void SetConstructionMesh(UStaticMesh* Mesh);
    UStaticMeshComponent* GetConstructionMeshComponent() const
    {
        return RoadConstructionComponent;
    }

private:
    UPROPERTY(VisibleAnywhere, Category = "PinkCab|World|L1")
    TObjectPtr<USceneComponent> SceneRoot;

    UPROPERTY(VisibleAnywhere, Category = "PinkCab|World|L1")
    TObjectPtr<UStaticMeshComponent> RoadMeshComponent;

    UPROPERTY(VisibleAnywhere, Category = "PinkCab|World|L1")
    TObjectPtr<UStaticMeshComponent> RoadConstructionComponent;

    bool bBound = false;
    int32 BoundChunkIndex = INDEX_NONE;
    FPinkCabChunkId BoundChunkId;
};
