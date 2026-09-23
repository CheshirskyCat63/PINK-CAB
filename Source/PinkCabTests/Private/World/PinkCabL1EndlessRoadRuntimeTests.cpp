#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "FileHelpers.h"
#include "World/PinkCabL1RoadChunkActor.h"
#include "World/PinkCabL1EndlessRoadModel.h"
#include "Engine/World.h"

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

#endif
