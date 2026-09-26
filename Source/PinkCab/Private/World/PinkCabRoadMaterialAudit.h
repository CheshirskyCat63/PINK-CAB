#pragma once

#include "CoreMinimal.h"

class UStaticMeshComponent;

namespace PinkCabRoadMaterialAudit
{
bool ValidateNativeMetaRoadStack(
    const UStaticMeshComponent* RoadSurface,
    const UStaticMeshComponent* Sidewalks,
    const TArray<TObjectPtr<UStaticMeshComponent>>& Marks,
    const TArray<TObjectPtr<UStaticMeshComponent>>& Curbs);
}
