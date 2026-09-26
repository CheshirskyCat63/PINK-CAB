#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Misc/PackageName.h"
#include "FileHelpers.h"
#include "Editor.h"
#include "EngineUtils.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "Modules/ModuleManager.h"
#include "Runtime/PinkCabChaosTatraPawn.h"
#include "World/PinkCabL1EndlessRoadStreamer.h"
#include "World/PinkCabL1RoadChunkActor.h"
#include "World/PinkCabL1GreyboxCorridor.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabL1EndlessRoadMapCompositionTest,
    "PinkCab.World.L1EndlessRoad.Map.Composition",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabL1EndlessRoadMapCompositionTest::RunTest(const FString& Parameters)
{
    const FString MapPackage = TEXT("/Game/Dev/Maps/L_PinkCab_L1_EndlessStraight");
    const bool bMapExists = FPackageName::DoesPackageExist(MapPackage);
    TestTrue(TEXT("endless Level 1 candidate map package exists"), bMapExists);
    if (!bMapExists)
    {
        return false;
    }

    const bool bLoaded = FEditorFileUtils::LoadMap(MapPackage, false, true);
    TestTrue(TEXT("endless Level 1 candidate map loads"), bLoaded);
    if (!bLoaded)
    {
        return false;
    }

    UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
    TestNotNull(TEXT("editor world exists"), World);
    if (!World)
    {
        return false;
    }

    APinkCabChaosTatraPawn* Pawn = nullptr;
    APinkCabL1EndlessRoadStreamer* Streamer = nullptr;
    int32 PawnCount = 0;
    int32 StreamerCount = 0;
    int32 LegacyGreyboxCount = 0;

    for (TActorIterator<APinkCabChaosTatraPawn> It(World); It; ++It)
    {
        Pawn = *It;
        ++PawnCount;
    }
    for (TActorIterator<APinkCabL1EndlessRoadStreamer> It(World); It; ++It)
    {
        Streamer = *It;
        ++StreamerCount;
    }
    for (TActorIterator<APinkCabL1GreyboxCorridor> It(World); It; ++It)
    {
        ++LegacyGreyboxCount;
    }

    TestEqual(TEXT("candidate map contains one current Tatra"), PawnCount, 1);
    TestEqual(TEXT("candidate map contains one endless-road streamer"), StreamerCount, 1);
    TestEqual(TEXT("finite L1 greybox is not a drive surface in candidate map"),
        LegacyGreyboxCount, 0);

    if (!Pawn || !Streamer)
    {
        return false;
    }

    TestTrue(TEXT("streamer tracks saved Tatra actor"),
        Streamer->GetTrackedActor() == Pawn);

    TestTrue(TEXT("candidate map can materialize initial seven-kilometre window"),
        Streamer->RefreshForState(
            Pawn->GetActorLocation(),
            FVector(100.0, 0.0, 0.0)));
    TestEqual(TEXT("initial map materialization has seven active chunks"),
        Streamer->GetActiveChunkCount(), 7);
    TestEqual(TEXT("initial map materialization uses seven physical chunks"),
        Streamer->GetPoolSize(), 7);

    for (const int32 ChunkIndex : Streamer->GetActiveChunkIndices())
    {
        const APinkCabL1RoadChunkActor* Chunk =
            Streamer->FindActiveChunkActor(ChunkIndex);
        TestNotNull(TEXT("every active logical chunk has physical representation"), Chunk);
        if (Chunk)
        {
            TestTrue(TEXT("every active physical chunk has baked MetaRoad visual"),
                Chunk->IsVisualReady());
        }
    }

    TestTrue(TEXT("candidate map passes MapCheck"),
        GEditor->Exec(World, TEXT("MAP CHECK"), *GLog));
    return true;
}


namespace PinkCabL1EndlessRoadAssetContract
{
TArray<FName> FindMetaRoadDependencies(const FName RootPackage)
{
    IAssetRegistry& Registry =
        FModuleManager::LoadModuleChecked<FAssetRegistryModule>(
            TEXT("AssetRegistry")).Get();

    TSet<FName> Visited;
    TArray<FName> Pending;
    TArray<FName> MetaRoadDependencies;
    Pending.Add(RootPackage);

    while (Pending.Num() > 0)
    {
        const FName Current = Pending.Pop(EAllowShrinking::No);
        if (Visited.Contains(Current))
        {
            continue;
        }
        Visited.Add(Current);

        TArray<FName> Dependencies;
        Registry.GetDependencies(
            Current,
            Dependencies,
            UE::AssetRegistry::EDependencyCategory::Package);

        for (const FName Dependency : Dependencies)
        {
            const FString Path = Dependency.ToString();
            if (Path.StartsWith(TEXT("/MetaRoad/")))
            {
                MetaRoadDependencies.AddUnique(Dependency);
            }
            if (!Visited.Contains(Dependency))
            {
                Pending.Add(Dependency);
            }
        }
    }

    MetaRoadDependencies.Sort(FNameLexicalLess());
    return MetaRoadDependencies;
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabL1EndlessRoadMaterialContractTest,
    "PinkCab.World.L1EndlessRoad.Asset.MaterialContract",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabL1EndlessRoadMaterialContractTest::RunTest(const FString& Parameters)
{
    UStaticMesh* Road = LoadObject<UStaticMesh>(
        nullptr,
        TEXT("/Game/World/L1/Road/RoadSurface.RoadSurface"));
    TestNotNull(TEXT("baked MetaRoad road surface loads"), Road);
    if (!Road)
    {
        return false;
    }

    // The baked road mesh owns the material dependency. The map owns the
    // streamer/chunk actors that load RoadSurface at runtime, so requiring the
    // .umap package itself to duplicate /MetaRoad/ package references is an
    // incorrect dependency-grain contract.
    const FName RoadSurfacePackage(TEXT("/Game/World/L1/Road/RoadSurface"));
    const TArray<FName> MetaRoadDependencies =
        PinkCabL1EndlessRoadAssetContract::FindMetaRoadDependencies(
            RoadSurfacePackage);
    bool bHasDriveSurface = false;
    bool bHasNativeAsphaltMaster = false;
    bool bHasNativeAlbedo = false;
    bool bHasNativeNormal = false;
    bool bHasNativeRoughness = false;
    for (const FName Dependency : MetaRoadDependencies)
    {
        const FString DependencyPath = Dependency.ToString();
        AddInfo(FString::Printf(
            TEXT("CD869_R4_NATIVE_METAROAD_DEP[%s]=%s"),
            *RoadSurfacePackage.ToString(),
            *DependencyPath));
        bHasDriveSurface |=
            DependencyPath.Contains(
                TEXT("/MetaRoad/MetaRoad/Materials/MI_DriveSurface"));
        bHasNativeAsphaltMaster |=
            DependencyPath.Contains(
                TEXT("/MetaRoad/MetaRoad/Materials/Master/M_Asphalt"));
        bHasNativeAlbedo |=
            DependencyPath.Contains(
                TEXT("/MetaRoad/MetaRoad/Textures/Asphalt/tiggcjdo_8K_Albedo"));
        bHasNativeNormal |=
            DependencyPath.Contains(
                TEXT("/MetaRoad/MetaRoad/Textures/Asphalt/tiggcjdo_8K_Normal"));
        bHasNativeRoughness |=
            DependencyPath.Contains(
                TEXT("/MetaRoad/MetaRoad/Textures/Asphalt/tiggcjdo_8K_Roughness"));
    }

    TestTrue(
        TEXT("RoadSurface retains native MetaRoad runtime dependencies"),
        MetaRoadDependencies.Num() > 0);
    TestTrue(
        TEXT("RoadSurface reaches MetaRoad MI_DriveSurface"),
        bHasDriveSurface);
    TestTrue(
        TEXT("RoadSurface reaches native MetaRoad asphalt master"),
        bHasNativeAsphaltMaster);
    TestTrue(
        TEXT("RoadSurface reaches native MetaRoad 8K asphalt albedo"),
        bHasNativeAlbedo);
    TestTrue(
        TEXT("RoadSurface reaches native MetaRoad 8K asphalt normal"),
        bHasNativeNormal);
    TestTrue(
        TEXT("RoadSurface reaches native MetaRoad 8K asphalt roughness"),
        bHasNativeRoughness);

    const TArray<FStaticMaterial>& Materials = Road->GetStaticMaterials();
    TestTrue(TEXT("baked road has at least one authored material slot"),
        Materials.Num() > 0);

    int32 ValidMaterials = 0;
    for (int32 Index = 0; Index < Materials.Num(); ++Index)
    {
        UMaterialInterface* Material = Materials[Index].MaterialInterface;
        TestNotNull(
            *FString::Printf(TEXT("road material slot %d is assigned"), Index),
            Material);
        if (!Material)
        {
            continue;
        }

        const FString Path = Material->GetPathName();
        AddInfo(FString::Printf(
            TEXT("CD869_ROAD_MATERIAL_SLOT[%d]=%s"),
            Index,
            *Path));
        TestEqual(
            *FString::Printf(TEXT("road material slot %d uses native MetaRoad drive surface"), Index),
            Path,
            FString(TEXT("/MetaRoad/MetaRoad/Materials/MI_DriveSurface.MI_DriveSurface")));
        TestTrue(
            *FString::Printf(TEXT("road material slot %d remains MetaRoad-owned"), Index),
            Path.StartsWith(TEXT("/MetaRoad/")));
        TestFalse(
            *FString::Printf(TEXT("road material slot %d is not Engine default"), Index),
            Path.Contains(TEXT("/Engine/EngineMaterials/DefaultMaterial")));
        ++ValidMaterials;
    }

    TestEqual(TEXT("every baked road material slot is assigned"),
        ValidMaterials, Materials.Num());
    return true;
}

#endif
