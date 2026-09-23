#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Misc/PackageName.h"
#include "FileHelpers.h"
#include "Editor.h"
#include "EngineUtils.h"
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
    TestTrue(TEXT("endless Level 1 candidate map package exists"),
        FPackageName::DoesPackageExist(MapPackage));

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

#endif
