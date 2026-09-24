#include "World/PinkCabL1EndlessRoadStreamer.h"

#include "Kismet/GameplayStatics.h"
#include "World/PinkCabL1RoadChunkActor.h"
#include "World/PinkCabWorldMaterializationPolicy.h"


namespace PinkCabL1EndlessRoadStreamer
{
bool BuildRoadGraph(
    const FPinkCabCityIdentity& City,
    const FPinkCabL1EndlessRoadWindow& Window,
    FPinkCabRoadGraph& OutGraph)
{
    for (const FPinkCabChunkCoord& Coord : Window.DesiredCoords)
    {
        if (!FPinkCabL1EndlessRoadModel::AppendStraightChunkLanes(
                City, Coord.Longitudinal, OutGraph))
        {
            return false;
        }
    }
    return OutGraph.NumLanes() ==
        FPinkCabL1EndlessRoadModel::PoolSize *
        FPinkCabL1EndlessRoadModel::GroundLaneCount;
}

void BuildCandidates(
    const FPinkCabCityIdentity& City,
    const FPinkCabL1EndlessRoadWindow& Window,
    TArray<FPinkCabWorldChunkCandidate>& OutCandidates,
    TMap<FString, int32>& OutChunkIndexById)
{
    OutCandidates.Reserve(Window.DesiredCoords.Num());
    for (const FPinkCabChunkCoord& Coord : Window.DesiredCoords)
    {
        FPinkCabWorldChunkCandidate Candidate;
        Candidate.Coord = Coord;
        Candidate.ChunkId = FPinkCabChunkId::From(City, Coord);
        OutChunkIndexById.Add(
            Candidate.ChunkId.Serialize(), Coord.Longitudinal);
        OutCandidates.Add(MoveTemp(Candidate));
    }
}

FPinkCabWorldMaterializationSettings BuildSettings()
{
    FPinkCabWorldMaterializationSettings Settings;
    Settings.MaxMaterializedChunks = FPinkCabL1EndlessRoadModel::PoolSize;
    Settings.MaxDistanceSquared =
        static_cast<int64>(FPinkCabL1EndlessRoadModel::AheadCount) *
        FPinkCabL1EndlessRoadModel::AheadCount;
    Settings.AllowedLayers = {0};
    Settings.MaxAnchorRequests = 0;
    return Settings;
}

void ClearDematerialized(
    const TArray<TObjectPtr<APinkCabL1RoadChunkActor>>& Pool,
    const TArray<FPinkCabChunkId>& Ids)
{
    TSet<FString> Keys;
    for (const FPinkCabChunkId& Id : Ids)
    {
        Keys.Add(Id.Serialize());
    }
    for (APinkCabL1RoadChunkActor* Chunk : Pool)
    {
        if (Chunk && Chunk->IsBound() &&
            Keys.Contains(Chunk->GetBoundChunkId().Serialize()))
        {
            Chunk->ClearBinding();
        }
    }
}

bool HasLogicalChunkBinding(
    const TArray<TObjectPtr<APinkCabL1RoadChunkActor>>& Pool,
    const FPinkCabChunkId& Id)
{
    for (const APinkCabL1RoadChunkActor* Chunk : Pool)
    {
        if (Chunk && Chunk->IsBound() && Chunk->GetBoundChunkId() == Id)
        {
            return true;
        }
    }
    return false;
}

bool CanApplyMaterialization(
    const FPinkCabWorldMaterializationResult& Result,
    const TMap<FString, int32>& ChunkIndexById,
    const TArray<TObjectPtr<APinkCabL1RoadChunkActor>>& Pool)
{
    TSet<FString> DematerializeKeys;
    for (const FPinkCabChunkId& Id : Result.DematerializeChunkIds)
    {
        DematerializeKeys.Add(Id.Serialize());
    }

    int32 NeededBindings = 0;
    for (const FPinkCabChunkId& Id : Result.MaterializeChunkIds)
    {
        if (HasLogicalChunkBinding(Pool, Id))
        {
            continue;
        }
        if (!ChunkIndexById.Contains(Id.Serialize()))
        {
            return false;
        }
        ++NeededBindings;
    }

    int32 ReadyReusableSlots = 0;
    for (const APinkCabL1RoadChunkActor* Chunk : Pool)
    {
        if (!Chunk || !Chunk->IsVisualReady())
        {
            continue;
        }

        if (!Chunk->IsBound() ||
            DematerializeKeys.Contains(
                Chunk->GetBoundChunkId().Serialize()))
        {
            ++ReadyReusableSlots;
        }
    }

    return ReadyReusableSlots >= NeededBindings;
}

APinkCabL1RoadChunkActor* FindFree(
    const TArray<TObjectPtr<APinkCabL1RoadChunkActor>>& Pool)
{
    for (APinkCabL1RoadChunkActor* Chunk : Pool)
    {
        if (Chunk && !Chunk->IsBound() && Chunk->IsVisualReady())
        {
            return Chunk;
        }
    }
    return nullptr;
}

bool BindMaterialized(
    const FPinkCabCityIdentity& City,
    const FPinkCabWorldMaterializationResult& Result,
    const TMap<FString, int32>& ChunkIndexById,
    const TArray<TObjectPtr<APinkCabL1RoadChunkActor>>& Pool)
{
    for (const FPinkCabChunkId& Id : Result.MaterializeChunkIds)
    {
        if (HasLogicalChunkBinding(Pool, Id))
        {
            continue;
        }

        const int32* ChunkIndex = ChunkIndexById.Find(Id.Serialize());
        APinkCabL1RoadChunkActor* FreeChunk = FindFree(Pool);
        if (!ChunkIndex || !FreeChunk ||
            !FreeChunk->BindChunk(City, *ChunkIndex, Id))
        {
            return false;
        }
    }
    return true;
}
}

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
            RequestedChunkIndex, RequestedDirection);

    FPinkCabRoadGraph DesiredRoadGraph;
    if (!PinkCabL1EndlessRoadStreamer::BuildRoadGraph(
            CityIdentity, DesiredWindow, DesiredRoadGraph))
    {
        return false;
    }

    TArray<FPinkCabWorldChunkCandidate> Candidates;
    TMap<FString, int32> ChunkIndexById;
    PinkCabL1EndlessRoadStreamer::BuildCandidates(
        CityIdentity, DesiredWindow, Candidates, ChunkIndexById);

    FPinkCabWorldMaterializationResult Result;
    if (!FPinkCabWorldMaterializationPolicy::BuildWindow(
            {RequestedChunkIndex, 0, 0},
            Candidates,
            PinkCabL1EndlessRoadStreamer::BuildSettings(),
            GetActiveChunkIds(),
            Result) ||
        Result.MaterializeChunkIds.Num() !=
            FPinkCabL1EndlessRoadModel::PoolSize)
    {
        return false;
    }

    if (!PinkCabL1EndlessRoadStreamer::CanApplyMaterialization(
            Result, ChunkIndexById, ChunkPool))
    {
        return false;
    }

    PinkCabL1EndlessRoadStreamer::ClearDematerialized(
        ChunkPool, Result.DematerializeChunkIds);
    if (!PinkCabL1EndlessRoadStreamer::BindMaterialized(
            CityIdentity, Result, ChunkIndexById, ChunkPool))
    {
        return false;
    }

    CurrentChunkIndex = RequestedChunkIndex;
    StableTravelDirection = RequestedDirection;
    LastMaterializationSignature = Result.RequestSignature;
    ActiveRoadGraph = MoveTemp(DesiredRoadGraph);
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
