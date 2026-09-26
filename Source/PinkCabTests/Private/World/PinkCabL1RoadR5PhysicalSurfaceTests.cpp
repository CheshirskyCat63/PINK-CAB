#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "HAL/FileManager.h"
#include "Materials/MaterialInterface.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "UObject/Package.h"
#include "UObject/SavePackage.h"
#include "World/PinkCabL1RoadChunkActor.h"
#include "World/PinkCabRoadSurfacePhysics.h"

namespace PinkCabL1RoadR5PhysicalSurfaceTests
{
const TCHAR* DryAsphaltPackage =
    TEXT("/Game/World/L1/Road/Physics/PM_PC_DryAsphalt");
const TCHAR* DryAsphaltAssetName =
    TEXT("PM_PC_DryAsphalt");

bool SavePhysicalMaterial(
    UPackage* Package,
    UPhysicalMaterial* Material)
{
    if (!Package || !Material)
    {
        return false;
    }

    Material->PostEditChange();
    Package->MarkPackageDirty();

    const FString Filename =
        FPackageName::LongPackageNameToFilename(
            DryAsphaltPackage,
            FPackageName::GetAssetPackageExtension());

    IFileManager::Get().MakeDirectory(
        *FPaths::GetPath(Filename),
        true);

    FSavePackageArgs SaveArgs;
    SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
    SaveArgs.SaveFlags = SAVE_None;

    return UPackage::SavePackage(
        Package,
        Material,
        *Filename,
        SaveArgs);
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabGenerateL1RoadR5DryAsphaltPhysicalMaterial,
    "PinkCab.Editor.GenerateL1RoadR5DryAsphaltPhysicalMaterial",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabGenerateL1RoadR5DryAsphaltPhysicalMaterial::RunTest(
    const FString& Parameters)
{
    using namespace PinkCabL1RoadR5PhysicalSurfaceTests;

    UPhysicalMaterial* Material =
        LoadObject<UPhysicalMaterial>(
            nullptr,
            PinkCabRoadSurfacePhysics::DryAsphaltObjectPath);

    UPackage* Package = Material
        ? Material->GetOutermost()
        : CreatePackage(DryAsphaltPackage);

    if (!Material)
    {
        Material = NewObject<UPhysicalMaterial>(
            Package,
            DryAsphaltAssetName,
            RF_Public | RF_Standalone);
        FAssetRegistryModule::AssetCreated(Material);
    }

    TestNotNull(TEXT("R5 dry-asphalt package exists"), Package);
    TestNotNull(TEXT("R5 dry-asphalt physical material exists"), Material);
    if (!Package || !Material)
    {
        return false;
    }

    Material->Friction =
        PinkCabRoadSurfacePhysics::DryAsphaltFriction;
    Material->Restitution =
        PinkCabRoadSurfacePhysics::DryAsphaltRestitution;

    TestTrue(
        TEXT("R5 dry-asphalt physical material saves"),
        SavePhysicalMaterial(Package, Material));

    AddInfo(FString::Printf(
        TEXT("CD869_R5_DRY_ASPHALT_AUTHORED path=%s friction=%.3f restitution=%.3f"),
        *Material->GetPathName(),
        Material->Friction,
        Material->Restitution));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabL1RoadR5DryAsphaltPhysicalSurface,
    "PinkCab.World.L1Road.R5.DryAsphaltPhysicalSurface",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabL1RoadR5DryAsphaltPhysicalSurface::RunTest(
    const FString& Parameters)
{
    UPhysicalMaterial* Material =
        LoadObject<UPhysicalMaterial>(
            nullptr,
            PinkCabRoadSurfacePhysics::DryAsphaltObjectPath);
    TestNotNull(TEXT("R5 dry-asphalt asset loads"), Material);
    if (!Material)
    {
        return false;
    }

    TestTrue(
        TEXT("R5 dry-asphalt contract values are canonical"),
        PinkCabRoadSurfacePhysics::IsDryAsphaltPhysicalMaterial(Material));
    TestTrue(
        TEXT("R5 dry-asphalt friction is normalized"),
        FMath::IsNearlyEqual(
            Material->Friction,
            PinkCabRoadSurfacePhysics::DryAsphaltFriction,
            KINDA_SMALL_NUMBER));
    TestTrue(
        TEXT("R5 dry-asphalt has no bounce"),
        FMath::IsNearlyEqual(
            Material->Restitution,
            PinkCabRoadSurfacePhysics::DryAsphaltRestitution,
            KINDA_SMALL_NUMBER));

    const APinkCabL1RoadChunkActor* ChunkCDO =
        GetDefault<APinkCabL1RoadChunkActor>();
    TestNotNull(TEXT("L1 road chunk CDO exists"), ChunkCDO);
    if (!ChunkCDO)
    {
        return false;
    }

    TestEqual(
        TEXT("L1 road chunk owns exact R5 dry-asphalt material"),
        ChunkCDO->GetRoadPhysicalMaterial(),
        Material);

    UStaticMeshComponent* RoadComponent =
        ChunkCDO->GetRoadMeshComponent();
    TestNotNull(TEXT("L1 road chunk exposes road surface component"), RoadComponent);
    if (!RoadComponent)
    {
        return false;
    }

    // Never resolve BodyInstance physical materials on the native CDO:
    // UE requires GEngine to be initialized for that query. The runtime
    // BindChunk path applies the override, and the packaged R5 audit proves
    // the effective material after the actor is live with collision enabled.

    UStaticMesh* RoadMesh = ChunkCDO->GetRoadMesh();
    TestNotNull(TEXT("R4 RoadSurface mesh remains assigned"), RoadMesh);
    if (!RoadMesh)
    {
        return false;
    }

    for (int32 Index = 0;
         Index < RoadMesh->GetStaticMaterials().Num();
         ++Index)
    {
        UMaterialInterface* VisualMaterial =
            RoadMesh->GetStaticMaterials()[Index].MaterialInterface;
        TestNotNull(TEXT("R4 visual material remains assigned"), VisualMaterial);
        if (VisualMaterial)
        {
            TestEqual(
                *FString::Printf(
                    TEXT("R4 visual slot %d stays native MetaRoad"),
                    Index),
                VisualMaterial->GetPathName(),
                FString(
                    TEXT("/MetaRoad/MetaRoad/Materials/MI_DriveSurface.MI_DriveSurface")));
        }
    }

    AddInfo(FString::Printf(
        TEXT("CD869_R5_DRY_ASPHALT=PASS path=%s friction=%.3f restitution=%.3f"),
        *Material->GetPathName(),
        Material->Friction,
        Material->Restitution));
    return true;
}

#endif
