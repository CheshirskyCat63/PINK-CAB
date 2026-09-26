#pragma once

#include "CoreMinimal.h"

class UPhysicalMaterial;
class UStaticMeshComponent;

namespace PinkCabRoadSurfacePhysics
{
inline constexpr TCHAR DryAsphaltObjectPath[] =
    TEXT("/Game/World/L1/Road/Physics/PM_PC_DryAsphalt.PM_PC_DryAsphalt");
inline constexpr float DryAsphaltFriction = 1.0f;
inline constexpr float DryAsphaltRestitution = 0.0f;

PINKCAB_API UPhysicalMaterial* LoadDryAsphaltPhysicalMaterial();
PINKCAB_API void ApplyDryAsphaltPhysicalMaterial(
    UStaticMeshComponent* RoadSurface,
    UPhysicalMaterial* Material);
PINKCAB_API bool IsDryAsphaltPhysicalMaterial(
    const UPhysicalMaterial* Material);
PINKCAB_API void AuditIfRequested(
    UStaticMeshComponent* RoadSurface,
    UPhysicalMaterial* Material);
}
