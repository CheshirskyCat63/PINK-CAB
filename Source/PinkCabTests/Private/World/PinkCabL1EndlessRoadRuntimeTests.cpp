#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "FileHelpers.h"
#include "World/PinkCabL1RoadChunkActor.h"
#include "World/PinkCabL1EndlessRoadModel.h"
#include "World/PinkCabL1EndlessRoadStreamer.h"
#include "World/PinkCabWorldMaterializationPolicy.h"
#include "Engine/World.h"
#include "EngineUtils.h"

namespace PinkCabL1EndlessRoadRuntimeTests
{
UWorld* NewTestWorld(FAutomationTestBase& Test)
{
    UWorld* World = UEditorLoadingAndSavingUtils::NewBlankMap(false);
    Test.TestNotNull(TEXT("blank editor world created"), World);
    return World;
}

APinkCabL1RoadChunkActor* SpawnChunk(UWorld& World, FAutomationTestBase& Test)
{
    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    Params.OverrideLevel = World.PersistentLevel;
    APinkCabL1RoadChunkActor* Chunk =
        World.SpawnActor<APinkCabL1RoadChunkActor>(
            APinkCabL1RoadChunkActor::StaticClass(),
            FTransform::Identity,
            Params);
    Test.TestNotNull(TEXT("road chunk actor spawned"), Chunk);
    return Chunk;
}

FPinkCabCityIdentity TestCity()
{
    return FPinkCabCityIdentity::Create(
        TEXT("CD869-ENDLESS-ROAD"),
        TEXT("l1-endless-v1"),
        TEXT("metaroad-3.2.0"));
}

FPinkCabChunkId ChunkId(const FPinkCabCityIdentity& City, const int32 Index)
{
    return FPinkCabChunkId::From(City, {Index, 0, 0});
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabL1RoadChunkBindingTest,
    "PinkCab.World.L1EndlessRoad.Runtime.ChunkBinding",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabL1RoadChunkBindingTest::RunTest(const FString& Parameters)
{
    UWorld* World = PinkCabL1EndlessRoadRuntimeTests::NewTestWorld(*this);
    if (!World) return false;

    APinkCabL1RoadChunkActor* Chunk =
        PinkCabL1EndlessRoadRuntimeTests::SpawnChunk(*World, *this);
    if (!Chunk) return false;

    TestFalse(TEXT("new chunk starts unbound"), Chunk->IsBound());
    TestTrue(TEXT("canonical baked MetaRoad visual is ready"), Chunk->IsVisualReady());
    TestEqual(TEXT("unbound index uses INDEX_NONE"), Chunk->GetBoundChunkIndex(), INDEX_NONE);

    const FPinkCabCityIdentity City = PinkCabL1EndlessRoadRuntimeTests::TestCity();
    const FPinkCabChunkId Id0 = PinkCabL1EndlessRoadRuntimeTests::ChunkId(City, 0);
    TestTrue(TEXT("chunk zero binds"), Chunk->BindChunk(City, 0, Id0));
    TestTrue(TEXT("chunk reports bound"), Chunk->IsBound());
    TestTrue(TEXT("bound id is exact"), Chunk->GetBoundChunkId() == Id0);
    TestEqual(TEXT("chunk zero world X"), Chunk->GetActorLocation().X, 0.0);

    const FPinkCabChunkId Id7 = PinkCabL1EndlessRoadRuntimeTests::ChunkId(City, 7);
    TestTrue(TEXT("same actor rebinds to positive chunk"), Chunk->BindChunk(City, 7, Id7));
    TestEqual(TEXT("positive chunk index stored"), Chunk->GetBoundChunkIndex(), 7);
    TestTrue(TEXT("positive chunk id replaces old id"), Chunk->GetBoundChunkId() == Id7);
    TestEqual(TEXT("chunk seven world X"),
        Chunk->GetActorLocation().X,
        7.0 * FPinkCabL1EndlessRoadModel::ChunkLengthCm);

    const FPinkCabChunkId IdNeg3 = PinkCabL1EndlessRoadRuntimeTests::ChunkId(City, -3);
    TestTrue(TEXT("same actor rebinds to negative chunk"), Chunk->BindChunk(City, -3, IdNeg3));
    TestEqual(TEXT("negative chunk index stored"), Chunk->GetBoundChunkIndex(), -3);
    TestTrue(TEXT("negative chunk id replaces previous id"), Chunk->GetBoundChunkId() == IdNeg3);
    TestEqual(TEXT("chunk minus three world X"),
        Chunk->GetActorLocation().X,
        -3.0 * FPinkCabL1EndlessRoadModel::ChunkLengthCm);

    Chunk->ClearBinding();
    TestFalse(TEXT("clear returns actor to unbound state"), Chunk->IsBound());
    TestEqual(TEXT("clear resets bound index"), Chunk->GetBoundChunkIndex(), INDEX_NONE);
    TestFalse(TEXT("clear resets logical id"), Chunk->GetBoundChunkId().IsValid());
    TestTrue(TEXT("cleared representation is hidden"), Chunk->IsHidden());

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabL1RoadChunkValidationTest,
    "PinkCab.World.L1EndlessRoad.Runtime.ChunkValidation",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabL1RoadChunkValidationTest::RunTest(const FString& Parameters)
{
    UWorld* World = PinkCabL1EndlessRoadRuntimeTests::NewTestWorld(*this);
    if (!World) return false;

    APinkCabL1RoadChunkActor* Chunk =
        PinkCabL1EndlessRoadRuntimeTests::SpawnChunk(*World, *this);
    if (!Chunk) return false;

    const FPinkCabCityIdentity City = PinkCabL1EndlessRoadRuntimeTests::TestCity();
    const FPinkCabChunkId Correct = PinkCabL1EndlessRoadRuntimeTests::ChunkId(City, 3);
    const FPinkCabChunkId Wrong = PinkCabL1EndlessRoadRuntimeTests::ChunkId(City, 4);

    TestFalse(TEXT("binding rejects mismatched logical id"),
        Chunk->BindChunk(City, 3, Wrong));
    TestFalse(TEXT("failed logical bind leaves actor unbound"), Chunk->IsBound());

    UStaticMesh* CanonicalMesh = Chunk->GetRoadMesh();
    TestNotNull(TEXT("canonical road mesh exists before fail-closed test"), CanonicalMesh);
    Chunk->SetRoadMesh(nullptr);
    TestFalse(TEXT("missing road visual reports not ready"), Chunk->IsVisualReady());
    TestFalse(TEXT("missing road visual refuses bind"),
        Chunk->BindChunk(City, 3, Correct));
    TestFalse(TEXT("missing visual does not create a bound fallback"), Chunk->IsBound());

    Chunk->SetRoadMesh(CanonicalMesh);
    TestTrue(TEXT("restored canonical visual becomes ready"), Chunk->IsVisualReady());
    TestTrue(TEXT("restored canonical visual can bind"), Chunk->BindChunk(City, 3, Correct));

    return true;
}


namespace PinkCabL1EndlessRoadStreamerTests
{
APinkCabL1EndlessRoadStreamer* SpawnStreamer(UWorld& World, FAutomationTestBase& Test)
{
    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    Params.OverrideLevel = World.PersistentLevel;
    APinkCabL1EndlessRoadStreamer* Streamer =
        World.SpawnActor<APinkCabL1EndlessRoadStreamer>(
            APinkCabL1EndlessRoadStreamer::StaticClass(),
            FTransform::Identity,
            Params);
    Test.TestNotNull(TEXT("endless road streamer spawned"), Streamer);
    return Streamer;
}

void TestIndices(
    FAutomationTestBase& Test,
    const TCHAR* Label,
    const TArray<int32>& Actual,
    std::initializer_list<int32> Expected)
{
    TArray<int32> SortedActual = Actual;
    SortedActual.Sort();
    TArray<int32> SortedExpected;
    for (const int32 Value : Expected)
    {
        SortedExpected.Add(Value);
    }
    SortedExpected.Sort();

    Test.TestEqual(
        *FString::Printf(TEXT("%s count"), Label),
        SortedActual.Num(),
        SortedExpected.Num());
    const int32 Count = FMath::Min(SortedActual.Num(), SortedExpected.Num());
    for (int32 Index = 0; Index < Count; ++Index)
    {
        Test.TestEqual(
            *FString::Printf(TEXT("%s[%d]"), Label, Index),
            SortedActual[Index],
            SortedExpected[Index]);
    }
}

int32 CountChunkActors(UWorld& World)
{
    int32 Count = 0;
    for (TActorIterator<APinkCabL1RoadChunkActor> It(&World); It; ++It)
    {
        ++Count;
    }
    return Count;
}

FString FindActiveIdForIndex(
    const APinkCabL1EndlessRoadStreamer& Streamer,
    const int32 ChunkIndex)
{
    if (const APinkCabL1RoadChunkActor* Chunk =
        Streamer.FindActiveChunkActor(ChunkIndex))
    {
        return Chunk->GetBoundChunkId().Serialize();
    }
    return FString();
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabL1EndlessRoadStreamerWindowTest,
    "PinkCab.World.L1EndlessRoad.Streamer.DirectionWindow",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabL1EndlessRoadStreamerWindowTest::RunTest(const FString& Parameters)
{
    UWorld* World = PinkCabL1EndlessRoadRuntimeTests::NewTestWorld(*this);
    if (!World) return false;

    APinkCabL1EndlessRoadStreamer* Streamer =
        PinkCabL1EndlessRoadStreamerTests::SpawnStreamer(*World, *this);
    if (!Streamer) return false;

    TestTrue(TEXT("initial positive window refreshes"),
        Streamer->RefreshForState(FVector(50000.0, 0.0, 0.0), FVector(100.0, 0.0, 0.0)));
    TestEqual(TEXT("pool is fixed at seven"), Streamer->GetPoolSize(), 7);
    TestEqual(TEXT("seven chunks are active"), Streamer->GetActiveChunkCount(), 7);
    TestEqual(TEXT("current chunk is zero"), Streamer->GetCurrentChunkIndex(), 0);
    TestEqual(TEXT("stable direction is positive"),
        Streamer->GetStableTravelDirection(),
        EPinkCabLongitudinalTravelDirection::Positive);
    PinkCabL1EndlessRoadStreamerTests::TestIndices(
        *this, TEXT("positive window"),
        Streamer->GetActiveChunkIndices(),
        {-2, -1, 0, 1, 2, 3, 4});
    TestEqual(TEXT("exactly seven physical road actors exist"),
        PinkCabL1EndlessRoadStreamerTests::CountChunkActors(*World), 7);

    TestTrue(TEXT("next positive chunk refreshes by reuse"),
        Streamer->RefreshForState(FVector(150000.0, 0.0, 0.0), FVector(100.0, 0.0, 0.0)));
    PinkCabL1EndlessRoadStreamerTests::TestIndices(
        *this, TEXT("shifted positive window"),
        Streamer->GetActiveChunkIndices(),
        {-1, 0, 1, 2, 3, 4, 5});
    TestEqual(TEXT("shift does not grow physical pool"),
        PinkCabL1EndlessRoadStreamerTests::CountChunkActors(*World), 7);

    const double Chunk10X =
        10.0 * FPinkCabL1EndlessRoadModel::ChunkLengthCm + 10.0;
    TestTrue(TEXT("negative travel flips materialization window"),
        Streamer->RefreshForState(FVector(Chunk10X, 0.0, 0.0), FVector(-100.0, 0.0, 0.0)));
    TestEqual(TEXT("current chunk is ten"), Streamer->GetCurrentChunkIndex(), 10);
    TestEqual(TEXT("stable direction is negative"),
        Streamer->GetStableTravelDirection(),
        EPinkCabLongitudinalTravelDirection::Negative);
    PinkCabL1EndlessRoadStreamerTests::TestIndices(
        *this, TEXT("negative window"),
        Streamer->GetActiveChunkIndices(),
        {6, 7, 8, 9, 10, 11, 12});

    TestTrue(TEXT("near-standstill refresh remains valid"),
        Streamer->RefreshForState(FVector(Chunk10X, 0.0, 0.0), FVector(10.0, 0.0, 0.0)));
    TestEqual(TEXT("near standstill retains negative direction"),
        Streamer->GetStableTravelDirection(),
        EPinkCabLongitudinalTravelDirection::Negative);
    PinkCabL1EndlessRoadStreamerTests::TestIndices(
        *this, TEXT("standstill-hysteresis window"),
        Streamer->GetActiveChunkIndices(),
        {6, 7, 8, 9, 10, 11, 12});
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabL1EndlessRoadStreamerLongRunTest,
    "PinkCab.World.L1EndlessRoad.Streamer.BoundedLongRun",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabL1EndlessRoadStreamerLongRunTest::RunTest(const FString& Parameters)
{
    UWorld* World = PinkCabL1EndlessRoadRuntimeTests::NewTestWorld(*this);
    if (!World) return false;

    APinkCabL1EndlessRoadStreamer* Streamer =
        PinkCabL1EndlessRoadStreamerTests::SpawnStreamer(*World, *this);
    if (!Streamer) return false;

    TestTrue(TEXT("origin window materializes"),
        Streamer->RefreshForState(FVector(10.0, 0.0, 0.0), FVector(100.0, 0.0, 0.0)));
    const FString OriginIdA =
        PinkCabL1EndlessRoadStreamerTests::FindActiveIdForIndex(*Streamer, 0);
    TestFalse(TEXT("origin logical id is present"), OriginIdA.IsEmpty());

    bool bAllRefreshes = true;
    int32 MaxPool = Streamer->GetPoolSize();
    int32 MaxActive = Streamer->GetActiveChunkCount();
    for (int32 ChunkIndex = 1; ChunkIndex <= 100; ++ChunkIndex)
    {
        const double X =
            static_cast<double>(ChunkIndex) *
            FPinkCabL1EndlessRoadModel::ChunkLengthCm + 10.0;
        bAllRefreshes &= Streamer->RefreshForState(
            FVector(X, 0.0, 0.0),
            FVector(100.0, 0.0, 0.0));
        MaxPool = FMath::Max(MaxPool, Streamer->GetPoolSize());
        MaxActive = FMath::Max(MaxActive, Streamer->GetActiveChunkCount());
    }

    TestTrue(TEXT("all 100 sequential kilometre refreshes succeed"), bAllRefreshes);
    TestEqual(TEXT("pool never grows beyond seven"), MaxPool, 7);
    TestEqual(TEXT("active set never grows beyond seven"), MaxActive, 7);
    TestEqual(TEXT("world still contains seven road chunk actors"),
        PinkCabL1EndlessRoadStreamerTests::CountChunkActors(*World), 7);

    const double TeleportX =
        1000.0 * FPinkCabL1EndlessRoadModel::ChunkLengthCm + 10.0;
    TestTrue(TEXT("large teleport reconstructs one bounded window"),
        Streamer->RefreshForState(
            FVector(TeleportX, 0.0, 0.0),
            FVector(100.0, 0.0, 0.0)));
    TestEqual(TEXT("teleport does not grow pool"), Streamer->GetPoolSize(), 7);
    TestEqual(TEXT("teleport active set remains seven"), Streamer->GetActiveChunkCount(), 7);
    PinkCabL1EndlessRoadStreamerTests::TestIndices(
        *this, TEXT("teleport window"),
        Streamer->GetActiveChunkIndices(),
        {998, 999, 1000, 1001, 1002, 1003, 1004});

    TestTrue(TEXT("return to origin materializes"),
        Streamer->RefreshForState(FVector(10.0, 0.0, 0.0), FVector(100.0, 0.0, 0.0)));
    const FString OriginIdB =
        PinkCabL1EndlessRoadStreamerTests::FindActiveIdForIndex(*Streamer, 0);
    TestEqual(TEXT("origin logical identity reconstructs exactly"), OriginIdB, OriginIdA);
    TestEqual(TEXT("return still has seven physical actors"),
        PinkCabL1EndlessRoadStreamerTests::CountChunkActors(*World), 7);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabL1EndlessRoadStreamerPolicyTest,
    "PinkCab.World.L1EndlessRoad.Streamer.MaterializationPolicy",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabL1EndlessRoadStreamerPolicyTest::RunTest(const FString& Parameters)
{
    UWorld* World = PinkCabL1EndlessRoadRuntimeTests::NewTestWorld(*this);
    if (!World) return false;

    APinkCabL1EndlessRoadStreamer* Streamer =
        PinkCabL1EndlessRoadStreamerTests::SpawnStreamer(*World, *this);
    if (!Streamer) return false;

    constexpr int32 Current = 5;
    const FPinkCabL1EndlessRoadWindow Window =
        FPinkCabL1EndlessRoadModel::BuildWindow(
            Current, EPinkCabLongitudinalTravelDirection::Positive);

    TArray<FPinkCabWorldChunkCandidate> Candidates;
    for (const FPinkCabChunkCoord& Coord : Window.DesiredCoords)
    {
        FPinkCabWorldChunkCandidate Candidate;
        Candidate.Coord = Coord;
        Candidate.ChunkId = FPinkCabChunkId::From(Streamer->GetCityIdentity(), Coord);
        Candidates.Add(Candidate);
    }

    FPinkCabWorldMaterializationSettings Settings;
    Settings.MaxMaterializedChunks = FPinkCabL1EndlessRoadModel::PoolSize;
    Settings.MaxDistanceSquared =
        FPinkCabL1EndlessRoadModel::AheadCount *
        FPinkCabL1EndlessRoadModel::AheadCount;
    Settings.AllowedLayers = {0};
    Settings.MaxAnchorRequests = 0;

    FPinkCabWorldMaterializationResult Expected;
    TestTrue(TEXT("reference materialization policy builds"),
        FPinkCabWorldMaterializationPolicy::BuildWindow(
            {Current, 0, 0},
            Candidates,
            Settings,
            {},
            Expected));

    const double X =
        static_cast<double>(Current) *
        FPinkCabL1EndlessRoadModel::ChunkLengthCm + 10.0;
    TestTrue(TEXT("streamer materializes same state"),
        Streamer->RefreshForState(FVector(X, 0.0, 0.0), FVector(100.0, 0.0, 0.0)));
    TestEqual(TEXT("streamer exposes exact policy request signature"),
        Streamer->GetLastMaterializationSignature(),
        Expected.RequestSignature);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabL1EndlessRoadStreamerMissingVisualTest,
    "PinkCab.World.L1EndlessRoad.Streamer.MissingVisualFailsBounded",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabL1EndlessRoadStreamerMissingVisualTest::RunTest(const FString& Parameters)
{
    UWorld* World = PinkCabL1EndlessRoadRuntimeTests::NewTestWorld(*this);
    if (!World) return false;

    APinkCabL1EndlessRoadStreamer* Streamer =
        PinkCabL1EndlessRoadStreamerTests::SpawnStreamer(*World, *this);
    if (!Streamer) return false;

    TestTrue(TEXT("initial window materializes"),
        Streamer->RefreshForState(FVector(10.0, 0.0, 0.0), FVector(100.0, 0.0, 0.0)));

    APinkCabL1RoadChunkActor* LeavingSlot = Streamer->FindActiveChunkActor(-2);
    TestNotNull(TEXT("leaving pool slot found"), LeavingSlot);
    if (!LeavingSlot) return false;

    LeavingSlot->SetRoadMesh(nullptr);
    TestFalse(TEXT("removing visual clears its binding"), LeavingSlot->IsBound());

    TestFalse(TEXT("next window fails closed when only free slot lacks road visual"),
        Streamer->RefreshForState(
            FVector(150000.0, 0.0, 0.0),
            FVector(100.0, 0.0, 0.0)));
    TestEqual(TEXT("failed refresh never expands pool"), Streamer->GetPoolSize(), 7);
    TestTrue(TEXT("failed refresh remains bounded"),
        Streamer->GetActiveChunkCount() <= 7);
    TestEqual(TEXT("world actor count remains fixed"),
        PinkCabL1EndlessRoadStreamerTests::CountChunkActors(*World), 7);
    return true;
}

#endif
