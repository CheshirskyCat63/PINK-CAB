#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PinkCabChaosWeaveCourse.generated.h"

class USceneComponent;
class UStaticMeshComponent;
class UDirectionalLightComponent;

UCLASS()
class PINKCAB_API APinkCabChaosWeaveCourse : public AActor
{
    GENERATED_BODY()

public:
    APinkCabChaosWeaveCourse();

    static int32 GetObstacleCount() { return 12; }
    static float GetLongitudinalGapCm() { return 1800.0f; }
    static FVector GetObstacleLocation(int32 Index);
    static FTransform GetPawnSpawnTransform();
    static FVector GetCourseSizeCm() { return FVector(30000.0f, 1200.0f, 20.0f); }

    const TArray<TObjectPtr<UStaticMeshComponent>>& GetObstacleBlocks() const
    {
        return ObstacleBlocks;
    }

private:
    UPROPERTY(VisibleAnywhere)
    TObjectPtr<USceneComponent> SceneRoot;

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UStaticMeshComponent> RoadSurface;

    UPROPERTY(VisibleAnywhere)
    TArray<TObjectPtr<UStaticMeshComponent>> ObstacleBlocks;

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UDirectionalLightComponent> KeyLight;
};
