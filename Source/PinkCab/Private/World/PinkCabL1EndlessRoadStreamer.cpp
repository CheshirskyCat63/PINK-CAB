#include "World/PinkCabL1EndlessRoadStreamer.h"

#include "Kismet/GameplayStatics.h"
#include "World/PinkCabL1RoadChunkActor.h"
#include "World/PinkCabWorldMaterializationPolicy.h"

APinkCabL1EndlessRoadStreamer::APinkCabL1EndlessRoadStreamer()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = true;

    RoadChunkClass = APinkCabL1RoadChunkActor::StaticClass();
    CityIdentity = FPinkCabCityIdentity::Create(
        TEXT("CD869-L1-ENDLESS"),
        TEXT("l1-endless-v1"),
        TEXT("metaroad-3.2.0"));
}

void APinkCabL1EndlessRoadStreamer::BeginPlay()
{
    Super::BeginPlay();

    if (!TrackedActor)
    {
        TrackedActor = UGameplayStatics::GetPlayerPawn(this, 0);
    }

    if (TrackedActor)
    {
        RefreshForState(
            TrackedActor->GetActorLocation(),
            TrackedActor->GetVelocity());
    }
}

void APinkCabL1EndlessRoadStreamer::Tick(const float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (!TrackedActor)
    {
        return;
    }

    RefreshForState(
        TrackedActor->GetActorLocation(),
        TrackedActor->GetVelocity());
}

bool APinkCabL1EndlessRoadStreamer::EnsurePool()
{
    if (!GetWorld() || !RoadChunkClass)
    {
        return false;
    }

    for (int32 Index = ChunkPool.Num();
         Index < FPinkCabL1EndlessRoadModel::PoolSize;
         ++Index)
    {
        FActorSpawnParameters Params;
        Params.Owner = this;
        Params.SpawnCollisionHandlingOverride =
            ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

        APinkCabL1RoadChunkActor* Chunk =
            GetWorld()->SpawnActor<APinkCabL1RoadChunkActor>(
                RoadChunkClass,
                FTransform::Identity,
                Params);
        if (!Chunk)
        {
            return false;
        }

        Chunk->ClearBinding();
        ChunkPool.Add(Chunk);
    }

    return ChunkPool.Num() == FPinkCabL1EndlessRoadModel::PoolSize;
}

bool APinkCabL1EndlessRoadStreamer::RefreshForState(
    const FVector& WorldLocation,
    const FVector& WorldVelocity)
{
    if (!CityIdentity.IsValid() || !EnsurePool())
    {
        return false;
    }

    const int32 RequestedChunkIndex =
        FPinkCabL1EndlessRoadModel::ResolveChunkIndex(WorldLocation.X);
    const EPinkCabLongitudinalTravelDirection RequestedDirection =
        FPinkCabL1EndlessRoadModel::ResolveTravelDirection(
            WorldVelocity.X,
            StableTravelDirection,
            DirectionThresholdCmPerSec);

    if (bHasMaterializedState &&
        RequestedChunkIndex == CurrentChunkIndex &&
        RequestedDirection == StableTravelDirection &&
        GetActiveChunkCount() == FPinkCabL1EndlessRoadModel::PoolSize)
    {
        return true;
    }

    const FPinkCabL1EndlessRoadWindow DesiredWindow =
        FPinkCabL1EndlessRoadModel::BuildWindow(
            RequestedChunkIndex,
            RequestedDirection);

    TArray<FPinkCabWorldChunkCandidate> Candidates;
    Candidates.Reserve(DesiredWindow.DesiredCoords.Num());

    TMap<FString, int32> ChunkIndexById;
    for (const FPinkCabChunkCoord& Coord : DesiredWindow.DesiredCoords)
    {
        FPinkCabWorldChunkCandidate Candidate;
        Candidate.Coord = Coord;
        Candidate.ChunkId = FPinkCabChunkId::From(CityIdentity, Coord);
        ChunkIndexById.Add(Candidate.ChunkId.Serialize(), Coord.Longitudinal);
        Candidates.Add(MoveTemp(Candidate));
    }

    FPinkCabWorldMaterializationSettings Settings;
    Settings.MaxMaterializedChunks = FPinkCabL1EndlessRoadModel::PoolSize;
    Settings.MaxDistanceSquared =
        static_cast<int64>(FPinkCabL1EndlessRoadModel::AheadCount) *
        FPinkCabL1EndlessRoadModel::AheadCount;
    Settings.AllowedLayers = {0};
    Settings.MaxAnchorRequests = 0;

    const TArray<FPinkCabChunkId> PreviousMaterialized = GetActiveChunkIds();

    FPinkCabWorldMaterializationResult Result;
    if (!FPinkCabWorldMaterializationPolicy::BuildWindow(
            {RequestedChunkIndex, 0, 0},
            Candidates,
            Settings,
            PreviousMaterialized,
            Result))
    {
        return false;
    }

    if (Result.MaterializeChunkIds.Num() !=
        FPinkCabL1EndlessRoadModel::PoolSize)
    {
        return false;
    }

    TSet<FString> DematerializeKeys;
    for (const FPinkCabChunkId& Id : Result.DematerializeChunkIds)
    {
        DematerializeKeys.Add(Id.Serialize());
    }

    for (APinkCabL1RoadChunkActor* Chunk : ChunkPool)
    {
        if (Chunk &&
            Chunk->IsBound() &&
            DematerializeKeys.Contains(Chunk->GetBoundChunkId().Serialize()))
        {
            Chunk->ClearBinding();
        }
    }

    for (const FPinkCabChunkId& Id : Result.MaterializeChunkIds)
    {
        bool bAlreadyBound = false;
        for (APinkCabL1RoadChunkActor* Chunk : ChunkPool)
        {
            if (Chunk &&
                Chunk->IsBound() &&
                Chunk->GetBoundChunkId() == Id)
            {
                bAlreadyBound = true;
                break;
            }
        }
        if (bAlreadyBound)
        {
            continue;
        }

        const int32* ChunkIndex = ChunkIndexById.Find(Id.Serialize());
        if (!ChunkIndex)
        {
            return false;
        }

        APinkCabL1RoadChunkActor* FreeChunk = nullptr;
        for (APinkCabL1RoadChunkActor* Chunk : ChunkPool)
        {
            if (Chunk && !Chunk->IsBound())
            {
                FreeChunk = Chunk;
                break;
            }
        }

        if (!FreeChunk ||
            !FreeChunk->BindChunk(CityIdentity, *ChunkIndex, Id))
        {
            return false;
        }
    }

    CurrentChunkIndex = RequestedChunkIndex;
    StableTravelDirection = RequestedDirection;
    LastMaterializationSignature = Result.RequestSignature;
    bHasMaterializedState = true;
    return GetActiveChunkCount() == FPinkCabL1EndlessRoadModel::PoolSize;
}

int32 APinkCabL1EndlessRoadStreamer::GetActiveChunkCount() const
{
    int32 Count = 0;
    for (const APinkCabL1RoadChunkActor* Chunk : ChunkPool)
    {
        if (Chunk && Chunk->IsBound())
        {
            ++Count;
        }
    }
    return Count;
}

TArray<FPinkCabChunkId> APinkCabL1EndlessRoadStreamer::GetActiveChunkIds() const
{
    TArray<FPinkCabChunkId> Result;
    Result.Reserve(ChunkPool.Num());
    for (const APinkCabL1RoadChunkActor* Chunk : ChunkPool)
    {
        if (Chunk && Chunk->IsBound())
        {
            Result.Add(Chunk->GetBoundChunkId());
        }
    }
    return Result;
}

TArray<int32> APinkCabL1EndlessRoadStreamer::GetActiveChunkIndices() const
{
    TArray<int32> Result;
    Result.Reserve(ChunkPool.Num());
    for (const APinkCabL1RoadChunkActor* Chunk : ChunkPool)
    {
        if (Chunk && Chunk->IsBound())
        {
            Result.Add(Chunk->GetBoundChunkIndex());
        }
    }
    return Result;
}

APinkCabL1RoadChunkActor*
APinkCabL1EndlessRoadStreamer::FindActiveChunkActor(const int32 ChunkIndex) const
{
    for (APinkCabL1RoadChunkActor* Chunk : ChunkPool)
    {
        if (Chunk &&
            Chunk->IsBound() &&
            Chunk->GetBoundChunkIndex() == ChunkIndex)
        {
            return Chunk;
        }
    }
    return nullptr;
}
