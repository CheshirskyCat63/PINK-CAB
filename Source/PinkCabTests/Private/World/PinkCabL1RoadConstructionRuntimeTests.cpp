#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "FileHelpers.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "World/PinkCabL1RoadChunkActor.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabL1RoadR2RuntimeChunkConstructionTest,
    "PinkCab.World.L1Road.R2.RuntimeChunkConstruction",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabL1RoadR2RuntimeChunkConstructionTest::RunTest(
    const FString& Parameters)
{
    UWorld* World = UEditorLoadingAndSavingUtils::NewBlankMap(false);
    TestNotNull(TEXT("R2 runtime test world created"), World);
    if (!World)
    {
        return false;
    }

    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride =
        ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    Params.OverrideLevel = World->PersistentLevel;
    APinkCabL1RoadChunkActor* Chunk =
        World->SpawnActor<APinkCabL1RoadChunkActor>(
            APinkCabL1RoadChunkActor::StaticClass(),
            FTransform::Identity,
            Params);
    TestNotNull(TEXT("R2 road chunk spawned"), Chunk);
    if (!Chunk)
    {
        return false;
    }

    UStaticMesh* Construction = Chunk->GetConstructionMesh();
    TestNotNull(TEXT("R2 construction mesh is loaded by chunk actor"),
        Construction);
    TestNotNull(TEXT("R2 construction component exists"),
        Chunk->GetConstructionMeshComponent());
    TestTrue(TEXT("road chunk visual readiness includes R2 construction"),
        Chunk->IsVisualReady());

    if (Chunk->GetConstructionMeshComponent())
    {
        TestEqual(TEXT("unbound R2 construction collision disabled"),
            Chunk->GetConstructionMeshComponent()->GetCollisionEnabled(),
            ECollisionEnabled::NoCollision);
    }

    const FPinkCabCityIdentity City = FPinkCabCityIdentity::Create(
        TEXT("CD869-R2-CONSTRUCTION"),
        TEXT("l1-endless-v1"),
        TEXT("road-v2-r2"));
    const FPinkCabChunkId Id =
        FPinkCabChunkId::From(City, {0, 0, 0});

    TestTrue(TEXT("R2 chunk binds with construction present"),
        Chunk->BindChunk(City, 0, Id));
    if (Chunk->GetConstructionMeshComponent())
    {
        TestEqual(TEXT("bound R2 construction collision enabled"),
            Chunk->GetConstructionMeshComponent()->GetCollisionEnabled(),
            ECollisionEnabled::QueryAndPhysics);
    }

    Chunk->ClearBinding();
    if (Chunk->GetConstructionMeshComponent())
    {
        TestEqual(TEXT("clear disables R2 construction collision"),
            Chunk->GetConstructionMeshComponent()->GetCollisionEnabled(),
            ECollisionEnabled::NoCollision);
    }

    Chunk->SetConstructionMesh(nullptr);
    TestFalse(TEXT("missing R2 construction fails visual readiness"),
        Chunk->IsVisualReady());
    TestFalse(TEXT("missing R2 construction refuses bind"),
        Chunk->BindChunk(City, 0, Id));

    Chunk->SetConstructionMesh(Construction);
    TestTrue(TEXT("restored R2 construction restores visual readiness"),
        Chunk->IsVisualReady());
    TestTrue(TEXT("restored R2 construction binds"),
        Chunk->BindChunk(City, 0, Id));

    return true;
}

#endif
