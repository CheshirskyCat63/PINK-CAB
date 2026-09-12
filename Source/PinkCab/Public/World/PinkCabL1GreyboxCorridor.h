#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "World/PinkCabRoadGraph.h"
#include "PinkCabL1GreyboxCorridor.generated.h"

class USceneComponent;
class UStaticMeshComponent;

UCLASS(NotBlueprintable)
class PINKCAB_API APinkCabL1GreyboxCorridor : public AActor
{
    GENERATED_BODY()

public:
    APinkCabL1GreyboxCorridor();

    int32 GetLaneSurfaceCount() const { return LaneSurfaces.Num(); }
    FPinkCabLaneId GetContractLaneId(int32 LaneIndex) const;
    bool HasSurfaceForLane(const FPinkCabLaneId& LaneId) const;
    bool HasLeftContactSurface() const { return LeftContactSurface != nullptr; }
    bool HasRightContactSurface() const { return RightContactSurface != nullptr; }
    FString GetAuthorityMarker() const;

private:
    UPROPERTY(VisibleAnywhere, Category="PINK CAB|Dev Greybox")
    TObjectPtr<USceneComponent> SceneRoot;
    UPROPERTY(VisibleAnywhere, Category="PINK CAB|Dev Greybox")
    TArray<TObjectPtr<UStaticMeshComponent>> LaneSurfaces;

    UPROPERTY(VisibleAnywhere, Category="PINK CAB|Dev Greybox")
    TObjectPtr<UStaticMeshComponent> LeftContactSurface;

    UPROPERTY(VisibleAnywhere, Category="PINK CAB|Dev Greybox")
    TObjectPtr<UStaticMeshComponent> RightContactSurface;
};
