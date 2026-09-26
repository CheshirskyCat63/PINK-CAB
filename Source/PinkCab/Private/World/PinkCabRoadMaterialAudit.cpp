#include "World/PinkCabRoadMaterialAudit.h"

#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"

namespace
{
bool HasExactMaterial(
    const UStaticMeshComponent* Component,
    const TCHAR* ExpectedPath)
{
    if (!Component || Component->GetNumMaterials() <= 0)
    {
        return false;
    }

    for (int32 Index = 0; Index < Component->GetNumMaterials(); ++Index)
    {
        const UMaterialInterface* Material = Component->GetMaterial(Index);
        if (!Material || Material->GetPathName() != ExpectedPath)
        {
            return false;
        }
    }
    return true;
}

bool HasExactMaterialOnAll(
    const TArray<TObjectPtr<UStaticMeshComponent>>& Components,
    const TCHAR* ExpectedPath)
{
    for (const UStaticMeshComponent* Component : Components)
    {
        if (!HasExactMaterial(Component, ExpectedPath))
        {
            return false;
        }
    }
    return true;
}

bool CanLoadNativeAsphaltTextures()
{
    static const TCHAR* RequiredTexturePaths[] =
    {
        TEXT("/MetaRoad/MetaRoad/Textures/Asphalt/tiggcjdo_8K_Albedo.tiggcjdo_8K_Albedo"),
        TEXT("/MetaRoad/MetaRoad/Textures/Asphalt/tiggcjdo_8K_Normal.tiggcjdo_8K_Normal"),
        TEXT("/MetaRoad/MetaRoad/Textures/Asphalt/tiggcjdo_8K_Roughness.tiggcjdo_8K_Roughness")
    };

    for (const TCHAR* TexturePath : RequiredTexturePaths)
    {
        if (!LoadObject<UObject>(nullptr, TexturePath))
        {
            return false;
        }
    }
    return true;
}
}

bool PinkCabRoadMaterialAudit::ValidateNativeMetaRoadStack(
    const UStaticMeshComponent* RoadSurface,
    const UStaticMeshComponent* Sidewalks,
    const TArray<TObjectPtr<UStaticMeshComponent>>& Marks,
    const TArray<TObjectPtr<UStaticMeshComponent>>& Curbs)
{
    const bool bMaterialsValid =
        HasExactMaterial(
            RoadSurface,
            TEXT("/MetaRoad/MetaRoad/Materials/MI_DriveSurface.MI_DriveSurface"))
        && HasExactMaterial(
            Sidewalks,
            TEXT("/MetaRoad/MetaRoad/Materials/M_Sidewolk.M_Sidewolk"))
        && HasExactMaterialOnAll(
            Marks,
            TEXT("/MetaRoad/MetaRoad/Materials/M_Mark.M_Mark"))
        && HasExactMaterialOnAll(
            Curbs,
            TEXT("/MetaRoad/MetaRoad/Materials/M_Curb.M_Curb"));

    const bool bTexturesValid = CanLoadNativeAsphaltTextures();

    UE_LOG(
        LogTemp,
        Display,
        TEXT("PINKCAB_ROAD_MATERIAL_AUDIT=%s materials=%s textures8k=%s"),
        bMaterialsValid && bTexturesValid ? TEXT("PASS") : TEXT("FAIL"),
        bMaterialsValid ? TEXT("PASS") : TEXT("FAIL"),
        bTexturesValid ? TEXT("PASS") : TEXT("FAIL"));

    return bMaterialsValid && bTexturesValid;
}
