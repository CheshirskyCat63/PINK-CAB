#include "World/PinkCabWorldMaterializationPolicy.h"

bool FPinkCabWorldChunkCandidate::IsValid() const
{
    return ChunkId.IsValid();
}

bool FPinkCabWorldMaterializationSettings::IsValid() const
{
    return MaxMaterializedChunks > 0
        && MaxDistanceSquared >= 0
        && AllowedLayers.Num() > 0
        && MaxAnchorRequests >= 0;
}

void FPinkCabWorldMaterializationResult::Reset()
{
    MaterializeChunkIds.Reset();
    DematerializeChunkIds.Reset();
    RequestSignature.Reset();
}

bool FPinkCabWorldMaterializationPolicy::BuildWindow(
    const FPinkCabChunkCoord& PlayerCoord,
    const TArray<FPinkCabWorldChunkCandidate>& Candidates,
    const FPinkCabWorldMaterializationSettings& Settings,
    const TArray<FPinkCabChunkId>& PreviousMaterialized,
    FPinkCabWorldMaterializationResult& OutResult)
{
    OutResult.Reset();
    if (!Settings.IsValid()) return false;

    TArray<FScoredCandidate> Eligible;
    Eligible.Reserve(FMath::Min(Candidates.Num(), Settings.MaxMaterializedChunks * 2));
    TSet<FString> SeenIds;
    for (const FPinkCabWorldChunkCandidate& Candidate : Candidates)
    {
        if (!Candidate.IsValid()
            || !Settings.AllowedLayers.Contains(Candidate.Coord.Layer))
        {
            continue;
        }

        const int64 DX =
            static_cast<int64>(Candidate.Coord.Longitudinal) - PlayerCoord.Longitudinal;
        const int64 DY =
            static_cast<int64>(Candidate.Coord.Lateral) - PlayerCoord.Lateral;
        const int64 DistanceSquared = DX * DX + DY * DY;
        if (DistanceSquared > Settings.MaxDistanceSquared) continue;

        const FString Key = Candidate.ChunkId.Serialize();
        if (SeenIds.Contains(Key)) continue;

        SeenIds.Add(Key);
        Eligible.Add({Candidate.ChunkId, DistanceSquared});
    }

    Eligible.Sort([](const FScoredCandidate& A, const FScoredCandidate& B)
    {
        if (A.DistanceSquared != B.DistanceSquared)
        {
            return A.DistanceSquared < B.DistanceSquared;
        }
        return A.ChunkId.Serialize() < B.ChunkId.Serialize();
    });

    const int32 Count = FMath::Min(Settings.MaxMaterializedChunks, Eligible.Num());
    OutResult.MaterializeChunkIds.Reserve(Count);
    TSet<FString> SelectedIds;
    for (int32 Index = 0; Index < Count; ++Index)
    {
        OutResult.MaterializeChunkIds.Add(Eligible[Index].ChunkId);
        SelectedIds.Add(Eligible[Index].ChunkId.Serialize());
    }

    TArray<FString> DematerializeKeys;
    for (const FPinkCabChunkId& PreviousId : PreviousMaterialized)
    {
        if (PreviousId.IsValid() && !SelectedIds.Contains(PreviousId.Serialize()))
        {
            DematerializeKeys.AddUnique(PreviousId.Serialize());
        }
    }
    DematerializeKeys.Sort();
    for (const FString& Key : DematerializeKeys)
    {
        OutResult.DematerializeChunkIds.Add(FPinkCabChunkId(Key));
    }

    FString SignaturePayload;
    for (const FPinkCabChunkId& Id : OutResult.MaterializeChunkIds)
    {
        SignaturePayload += Id.Serialize();
        SignaturePayload += TEXT(";");
    }
    OutResult.RequestSignature =
        PinkCabWorldId::StableToken(TEXT("materialize:"), SignaturePayload);
    return true;
}

bool FPinkCabWorldMaterializationPolicy::BuildAnchorRequests(
    const FPinkCabCityLocationRegistry& Registry,
    const FPinkCabWorldMaterializationSettings& Settings,
    TArray<FString>& OutAnchorIds)
{
    OutAnchorIds.Reset();
    if (!Settings.IsValid()) return false;

    const TArray<FPinkCabCityLocationAnchor> Anchors = Registry.GetAnchorsSorted();
    OutAnchorIds.Reserve(FMath::Min(Settings.MaxAnchorRequests, Anchors.Num()));
    for (const FPinkCabCityLocationAnchor& Anchor : Anchors)
    {
        if (!Anchor.IsValid() || !Settings.AllowedLayers.Contains(Anchor.Layer))
        {
            continue;
        }
        OutAnchorIds.Add(Anchor.AnchorId);
        if (OutAnchorIds.Num() >= Settings.MaxAnchorRequests)
        {
            break;
        }
    }
    return true;
}
