#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Misc/PackageName.h"
#include "FileHelpers.h"
#include "Editor.h"
#include "EngineUtils.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
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
        TestTrue(
            *FString::Printf(TEXT("road material slot %d is project-owned"), Index),
            Path.StartsWith(TEXT("/Game/World/L1/Road/Materials/")));
        TestFalse(
            *FString::Printf(TEXT("road material slot %d has no runtime MetaRoad dependency"), Index),
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
