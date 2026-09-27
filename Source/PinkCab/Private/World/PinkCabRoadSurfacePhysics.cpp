#include "World/PinkCabRoadSurfacePhysics.h"

#include "Components/StaticMeshComponent.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "PhysicsEngine/BodyInstance.h"
#include "UObject/UObjectGlobals.h"

UPhysicalMaterial* PinkCabRoadSurfacePhysics::LoadDryAsphaltPhysicalMaterial()
{
    return LoadObject<UPhysicalMaterial>(
        nullptr,
        DryAsphaltObjectPath);
}

void PinkCabRoadSurfacePhysics::ApplyDryAsphaltPhysicalMaterial(
    UStaticMeshComponent* RoadSurface,
    UPhysicalMaterial* Material)
{
    if (RoadSurface)
    {
        RoadSurface->SetPhysMaterialOverride(Material);
    }
}

bool PinkCabRoadSurfacePhysics::IsDryAsphaltPhysicalMaterial(
    const UPhysicalMaterial* Material)
{
    return Material != nullptr
        && Material->GetPathName() == DryAsphaltObjectPath
        && FMath::IsNearlyEqual(
            Material->Friction,
            DryAsphaltFriction,
            KINDA_SMALL_NUMBER)
        && FMath::IsNearlyEqual(
            Material->Restitution,
            DryAsphaltRestitution,
            KINDA_SMALL_NUMBER);
}

void PinkCabRoadSurfacePhysics::AuditIfRequested(
    UStaticMeshComponent* RoadSurface,
    UPhysicalMaterial* Material)
{
    if (!FParse::Param(
            FCommandLine::Get(),
            TEXT("PinkCabRoadSurfaceAudit")))
    {
        return;
    }

    static bool bAuditReported = false;
    if (bAuditReported)
    {
        return;
    }
    bAuditReported = true;

    UPhysicalMaterial* EffectiveMaterial = nullptr;
    if (RoadSurface)
    {
        if (FBodyInstance* BodyInstance = RoadSurface->GetBodyInstance())
        {
            EffectiveMaterial =
                BodyInstance->GetSimplePhysicalMaterial();
        }
    }

    const bool bValid =
        EffectiveMaterial == Material
        && IsDryAsphaltPhysicalMaterial(Material);

    UE_LOG(
        LogTemp,
        Display,
        TEXT("PINKCAB_ROAD_SURFACE_AUDIT=%s path=%s friction=%.3f restitution=%.3f"),
        bValid ? TEXT("PASS") : TEXT("FAIL"),
        Material ? *Material->GetPathName() : TEXT("NONE"),
        Material ? Material->Friction : -1.0f,
        Material ? Material->Restitution : -1.0f);
}
