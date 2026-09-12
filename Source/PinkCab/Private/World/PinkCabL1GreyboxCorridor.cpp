#include "World/PinkCabL1GreyboxCorridor.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "World/PinkCabGreyboxAuthorityTag.h"

namespace PinkCabL1GreyboxPrivate
{
    constexpr int32 ContractLaneCount = 5;
    constexpr float LaneWidthCm = 360.0f;
    constexpr float CorridorLengthCm = 100000.0f;
    constexpr float LaneThicknessCm = 20.0f;
    constexpr float ContactWallThicknessCm = 30.0f;
    constexpr float ContactWallHeightCm = 500.0f;
}

APinkCabL1GreyboxCorridor::APinkCabL1GreyboxCorridor()
{
    PrimaryActorTick.bCanEverTick = false;
    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    SetRootComponent(SceneRoot);
    Tags.Add(FName(TEXT("NON_AUTHORITATIVE_GEOMETRY")));

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(TEXT("/Engine/BasicShapes/Cube.Cube"));
    UStaticMesh* Cube = CubeFinder.Succeeded() ? CubeFinder.Object : nullptr;
    for (int32 LaneIndex = 0; LaneIndex < PinkCabL1GreyboxPrivate::ContractLaneCount; ++LaneIndex)
    {
        const FName Name(*FString::Printf(TEXT("LaneSurface_%d"), LaneIndex));
        UStaticMeshComponent* Lane = CreateDefaultSubobject<UStaticMeshComponent>(Name);
        Lane->SetupAttachment(SceneRoot);
        Lane->SetStaticMesh(Cube);
        Lane->SetMobility(EComponentMobility::Static);
        const float CenterOffset = (LaneIndex - 2) * PinkCabL1GreyboxPrivate::LaneWidthCm;
        Lane->SetRelativeLocation(FVector(0.0f, CenterOffset, 0.0f));
        Lane->SetRelativeScale3D(FVector(
            PinkCabL1GreyboxPrivate::CorridorLengthCm / 100.0f,
            PinkCabL1GreyboxPrivate::LaneWidthCm / 100.0f,
            PinkCabL1GreyboxPrivate::LaneThicknessCm / 100.0f));
        Lane->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        Lane->SetCastShadow(false);
        LaneSurfaces.Add(Lane);
    }

    const float HalfRoadWidth = 0.5f * PinkCabL1GreyboxPrivate::ContractLaneCount * PinkCabL1GreyboxPrivate::LaneWidthCm;
    const FVector WallScale(
        PinkCabL1GreyboxPrivate::CorridorLengthCm / 100.0f,
        PinkCabL1GreyboxPrivate::ContactWallThicknessCm / 100.0f,
        PinkCabL1GreyboxPrivate::ContactWallHeightCm / 100.0f);
    LeftContactSurface = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LeftContactSurface"));
    LeftContactSurface->SetupAttachment(SceneRoot);
    LeftContactSurface->SetStaticMesh(Cube);
    LeftContactSurface->SetRelativeLocation(FVector(0.0f, -HalfRoadWidth, PinkCabL1GreyboxPrivate::ContactWallHeightCm * 0.5f));
    LeftContactSurface->SetRelativeScale3D(WallScale);
    LeftContactSurface->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    LeftContactSurface->SetCastShadow(false);

    RightContactSurface = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RightContactSurface"));
    RightContactSurface->SetupAttachment(SceneRoot);
    RightContactSurface->SetStaticMesh(Cube);
    RightContactSurface->SetRelativeLocation(FVector(0.0f, HalfRoadWidth, PinkCabL1GreyboxPrivate::ContactWallHeightCm * 0.5f));
    RightContactSurface->SetRelativeScale3D(WallScale);
    RightContactSurface->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    RightContactSurface->SetCastShadow(false);
}

FPinkCabLaneId APinkCabL1GreyboxCorridor::GetContractLaneId(const int32 LaneIndex) const
{
    if (LaneIndex < 0 || LaneIndex >= PinkCabL1GreyboxPrivate::ContractLaneCount)
    {
        return FPinkCabLaneId();
    }
    const FPinkCabCityIdentity City = FPinkCabCityIdentity::Create(
        TEXT("L1-GREYBOX-ZERO"), TEXT("dev-greybox-gen-v1"), TEXT("builtin-primitives-v1"));
    const FPinkCabChunkId Chunk = FPinkCabChunkId::From(City, {0, 0, 1});
    return FPinkCabRoadGraph::MakeLaneId(City, Chunk, 0, LaneIndex);
}

bool APinkCabL1GreyboxCorridor::HasSurfaceForLane(const FPinkCabLaneId& LaneId) const
{
    for (int32 LaneIndex = 0; LaneIndex < LaneSurfaces.Num(); ++LaneIndex)
    {
        if (GetContractLaneId(LaneIndex) == LaneId && LaneSurfaces[LaneIndex] != nullptr)
        {
            return true;
        }
    }
    return false;
}

FString APinkCabL1GreyboxCorridor::GetAuthorityMarker() const
{
    return FPinkCabGreyboxAuthorityTag::Marker();
}
