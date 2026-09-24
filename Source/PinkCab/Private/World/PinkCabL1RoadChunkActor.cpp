#include "World/PinkCabL1RoadChunkActor.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"
#include "World/PinkCabL1EndlessRoadModel.h"

APinkCabL1RoadChunkActor::APinkCabL1RoadChunkActor()
{
    PrimaryActorTick.bCanEverTick = false;

    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    SetRootComponent(SceneRoot);

    RoadMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RoadSurface"));
    RoadMeshComponent->SetupAttachment(SceneRoot);
    RoadMeshComponent->SetMobility(EComponentMobility::Movable);
    RoadMeshComponent->SetGenerateOverlapEvents(false);
    RoadMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    static ConstructorHelpers::FObjectFinder<UStaticMesh> RoadMeshFinder(
        TEXT("/Game/World/L1/Road/RoadSurface.RoadSurface"));
    if (RoadMeshFinder.Succeeded())
    {
        RoadMeshComponent->SetStaticMesh(RoadMeshFinder.Object);
    }

    SetActorHiddenInGame(true);
    SetActorEnableCollision(false);
}

bool APinkCabL1RoadChunkActor::BindChunk(
    const FPinkCabCityIdentity& City,
    const int32 ChunkIndex,
    const FPinkCabChunkId& ChunkId)
{
    const FPinkCabChunkId ExpectedId =
        FPinkCabChunkId::From(City, {ChunkIndex, 0, 0});

    if (!City.IsValid() ||
        !ChunkId.IsValid() ||
        ChunkId != ExpectedId ||
        !IsVisualReady())
    {
        return false;
    }

    BoundChunkIndex = ChunkIndex;
    BoundChunkId = ChunkId;
    bBound = true;

    SetActorLocation(
        FVector(
            static_cast<double>(ChunkIndex) *
                FPinkCabL1EndlessRoadModel::ChunkLengthCm,
            0.0,
            0.0),
        false,
        nullptr,
        ETeleportType::TeleportPhysics);

    SetActorHiddenInGame(false);
    SetActorEnableCollision(true);
    RoadMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    return true;
}

void APinkCabL1RoadChunkActor::ClearBinding()
{
    bBound = false;
    BoundChunkIndex = INDEX_NONE;
    BoundChunkId = FPinkCabChunkId();

    SetActorEnableCollision(false);
    if (RoadMeshComponent)
    {
        RoadMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }
    SetActorHiddenInGame(true);
}

bool APinkCabL1RoadChunkActor::IsVisualReady() const
{
    return RoadMeshComponent != nullptr &&
        RoadMeshComponent->GetStaticMesh() != nullptr;
}

UStaticMesh* APinkCabL1RoadChunkActor::GetRoadMesh() const
{
    return RoadMeshComponent ? RoadMeshComponent->GetStaticMesh() : nullptr;
}

void APinkCabL1RoadChunkActor::SetRoadMesh(UStaticMesh* Mesh)
{
    if (!RoadMeshComponent)
    {
        return;
    }

    if (!Mesh && bBound)
    {
        ClearBinding();
    }

    RoadMeshComponent->SetStaticMesh(Mesh);
}
