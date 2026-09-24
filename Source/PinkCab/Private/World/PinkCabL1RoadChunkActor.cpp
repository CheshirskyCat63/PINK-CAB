#include "World/PinkCabL1RoadChunkActor.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"
#include "World/PinkCabL1EndlessRoadModel.h"

namespace
{
constexpr int32 NativeMetaRoadCurbMeshCount = 32;

// Captured from the MetaRoad 3.2.0 generated actor after splitting the
// kilometre at the two R1 access windows. MetaRoad emits one curb mesh per
// continuous curb span. Service-separator curb spans are deliberately absent
// inside Access A/B; all remaining spans retain the human-authored DefaultCurb.
const FVector NativeMetaRoadCurbRelativeLocations[NativeMetaRoadCurbMeshCount] =
{
    FVector(92500.0, -3240.0, 4.25),
    FVector(70000.0, -3240.0, 4.25),
    FVector(50000.0, -3240.0, 4.25),
    FVector(30000.0, -3240.0, 4.25),
    FVector(7500.0,  -3240.0, 4.25),
    FVector(7500.0,  -2200.0, 4.25),
    FVector(7500.0,  -2600.0, 4.25),
    FVector(7500.0,   -400.0, 4.25),
    FVector(30000.0,  -400.0, 4.25),
    FVector(50000.0,  -400.0, 4.25),
    FVector(70000.0,  -400.0, 4.25),
    FVector(92500.0,  -400.0, 4.25),
    FVector(92500.0,   400.0, 4.25),
    FVector(70000.0,   400.0, 4.25),
    FVector(50000.0,   400.0, 4.25),
    FVector(30000.0,   400.0, 4.25),
    FVector(7500.0,    400.0, 4.25),
    FVector(7500.0,   2200.0, 4.25),
    FVector(7500.0,   2600.0, 4.25),
    FVector(7500.0,   3240.0, 4.25),
    FVector(30000.0,  3240.0, 4.25),
    FVector(50000.0,  3240.0, 4.25),
    FVector(70000.0,  3240.0, 4.25),
    FVector(92500.0,  3240.0, 4.25),
    FVector(50000.0, -2200.0, 4.25),
    FVector(50000.0, -2600.0, 4.25),
    FVector(50000.0,  2200.0, 4.25),
    FVector(50000.0,  2600.0, 4.25),
    FVector(92500.0, -2200.0, 4.25),
    FVector(92500.0, -2600.0, 4.25),
    FVector(92500.0,  2200.0, 4.25),
    FVector(92500.0,  2600.0, 4.25)
};

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

    RoadSidewalksComponent =
        CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RoadSidewalks"));
    RoadSidewalksComponent->SetupAttachment(SceneRoot);
    RoadSidewalksComponent->SetMobility(EComponentMobility::Movable);
    RoadSidewalksComponent->SetGenerateOverlapEvents(false);
    RoadSidewalksComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    static ConstructorHelpers::FObjectFinder<UStaticMesh> SidewalksFinder(
        TEXT("/Game/World/L1/Road/RoadSidewalks.RoadSidewalks"));
    if (SidewalksFinder.Succeeded())
    {
        RoadSidewalksComponent->SetStaticMesh(SidewalksFinder.Object);
    }

    RoadCurbComponents.Reserve(NativeMetaRoadCurbMeshCount);
    for (int32 Index = 0; Index < NativeMetaRoadCurbMeshCount; ++Index)
    {
        const FString AssetName =
            Index == 0
                ? TEXT("RoadCurbs")
                : FString::Printf(TEXT("RoadCurbs%d"), Index);
        const FName ComponentName(
            *FString::Printf(TEXT("Native%s"), *AssetName));

        UStaticMeshComponent* CurbComponent =
            CreateDefaultSubobject<UStaticMeshComponent>(ComponentName);
        CurbComponent->SetupAttachment(SceneRoot);
        CurbComponent->SetMobility(EComponentMobility::Movable);
        CurbComponent->SetGenerateOverlapEvents(false);
        CurbComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        CurbComponent->SetRelativeLocation(
            NativeMetaRoadCurbRelativeLocations[Index]);

        const FString ObjectPath = FString::Printf(
            TEXT("/Game/World/L1/Road/%s.%s"),
            *AssetName,
            *AssetName);
        ConstructorHelpers::FObjectFinder<UStaticMesh> CurbFinder(*ObjectPath);
        if (CurbFinder.Succeeded())
        {
            CurbComponent->SetStaticMesh(CurbFinder.Object);
        }

        RoadCurbComponents.Add(CurbComponent);
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
    RoadMeshComponent->SetCollisionEnabled(
        ECollisionEnabled::QueryAndPhysics);
    SetNativeMetaRoadConstructionCollision(
        ECollisionEnabled::QueryAndPhysics);
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
        RoadMeshComponent->SetCollisionEnabled(
            ECollisionEnabled::NoCollision);
    }
    SetNativeMetaRoadConstructionCollision(
        ECollisionEnabled::NoCollision);
    SetActorHiddenInGame(true);
}

bool APinkCabL1RoadChunkActor::IsVisualReady() const
{
    return RoadMeshComponent != nullptr &&
        RoadMeshComponent->GetStaticMesh() != nullptr &&
        AreNativeMetaRoadConstructionMeshesReady();
}

bool APinkCabL1RoadChunkActor::AreNativeMetaRoadConstructionMeshesReady() const
{
    if (!RoadSidewalksComponent ||
        !RoadSidewalksComponent->GetStaticMesh() ||
        RoadCurbComponents.Num() != NativeMetaRoadCurbMeshCount)
    {
        return false;
    }

    for (const UStaticMeshComponent* Component : RoadCurbComponents)
    {
        if (!Component || !Component->GetStaticMesh())
        {
            return false;
        }
    }
    return true;
}

void APinkCabL1RoadChunkActor::SetNativeMetaRoadConstructionCollision(
    const ECollisionEnabled::Type Mode)
{
    if (RoadSidewalksComponent)
    {
        RoadSidewalksComponent->SetCollisionEnabled(Mode);
    }

    for (UStaticMeshComponent* Component : RoadCurbComponents)
    {
        if (Component)
        {
            Component->SetCollisionEnabled(Mode);
        }
    }
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
