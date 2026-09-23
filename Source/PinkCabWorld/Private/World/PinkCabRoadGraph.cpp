#include "World/PinkCabRoadGraph.h"

FPinkCabRoadNodeId::FPinkCabRoadNodeId() = default;
FPinkCabRoadNodeId::FPinkCabRoadNodeId(const FString& InValue) : Value(InValue) {}
bool FPinkCabRoadNodeId::IsValid() const { return !Value.IsEmpty(); }
const FString& FPinkCabRoadNodeId::Serialize() const { return Value; }
bool FPinkCabRoadNodeId::operator==(const FPinkCabRoadNodeId& Other) const { return Value == Other.Value; }

FPinkCabLaneId::FPinkCabLaneId() = default;
FPinkCabLaneId::FPinkCabLaneId(const FString& InValue) : Value(InValue) {}
bool FPinkCabLaneId::IsValid() const { return !Value.IsEmpty(); }
const FString& FPinkCabLaneId::Serialize() const { return Value; }
bool FPinkCabLaneId::operator==(const FPinkCabLaneId& Other) const { return Value == Other.Value; }

FPinkCabRoadNodeId FPinkCabRoadGraph::MakeNodeId(
    const FPinkCabCityIdentity& City,
    const FPinkCabChunkId& Chunk,
    int32 LocalNodeIndex)
{
    const FString Payload = FString::Printf(
        TEXT("%s|%s|node|%d"),
        *City.GetStableKey(),
        *Chunk.Serialize(),
        LocalNodeIndex);
    return FPinkCabRoadNodeId(PinkCabWorldId::StableToken(TEXT("node:"), Payload));
}

FPinkCabLaneId FPinkCabRoadGraph::MakeLaneId(
    const FPinkCabCityIdentity& City,
    const FPinkCabChunkId& Chunk,
    int32 LocalRoadIndex,
    int32 LocalLaneIndex)
{
    const FString Payload = FString::Printf(
        TEXT("%s|%s|road|%d|lane|%d"),
        *City.GetStableKey(),
        *Chunk.Serialize(),
        LocalRoadIndex,
        LocalLaneIndex);
    return FPinkCabLaneId(PinkCabWorldId::StableToken(TEXT("lane:"), Payload));
}

bool FPinkCabRoadGraph::AddLane(const FPinkCabLogicalLane& Lane)
{
    if (!Lane.LaneId.IsValid()
        || !Lane.FromNode.IsValid()
        || !Lane.ToNode.IsValid()
        || !FMath::IsFinite(Lane.LengthCm)
        || Lane.LengthCm <= 0.0
        || Lane.Layer < 0)
    {
        return false;
    }

    const FString Key = Lane.LaneId.Serialize();
    if (Lanes.Contains(Key)) return false;

    Lanes.Add(Key, Lane);
    OutgoingByNode.FindOrAdd(Lane.FromNode.Serialize()).Add(Key);
    return true;
}

const FPinkCabLogicalLane* FPinkCabRoadGraph::FindLane(const FPinkCabLaneId& LaneId) const
{
    return Lanes.Find(LaneId.Serialize());
}

TArray<FPinkCabLaneId> FPinkCabRoadGraph::GetNextLanes(const FPinkCabLaneId& CurrentLane) const
{
    TArray<FPinkCabLaneId> Result;
    const FPinkCabLogicalLane* Current = Lanes.Find(CurrentLane.Serialize());
    if (!Current) return Result;

    const TArray<FString>* NextKeys = OutgoingByNode.Find(Current->ToNode.Serialize());
    if (!NextKeys) return Result;

    TArray<FString> SortedKeys = *NextKeys;
    SortedKeys.Sort();
    Result.Reserve(SortedKeys.Num());
    for (const FString& Key : SortedKeys)
    {
        Result.Add(FPinkCabLaneId(Key));
    }
    return Result;
}

TArray<FPinkCabLaneId> FPinkCabRoadGraph::GetLaneIds() const
{
    TArray<FString> Keys;
    Lanes.GetKeys(Keys);
    Keys.Sort();

    TArray<FPinkCabLaneId> Result;
    Result.Reserve(Keys.Num());
    for (const FString& Key : Keys)
    {
        Result.Add(FPinkCabLaneId(Key));
    }
    return Result;
}

FString FPinkCabRoadGraph::GetTopologySignature() const
{
    TArray<FString> Keys;
    Lanes.GetKeys(Keys);
    Keys.Sort();
    FString Payload;
    for (const FString& Key : Keys)
    {
        const FPinkCabLogicalLane& Lane = Lanes[Key];
        Payload += Key;
        Payload += TEXT("|");
        Payload += Lane.FromNode.Serialize();
        Payload += TEXT("|");
        Payload += Lane.ToNode.Serialize();
        Payload += TEXT("|");
        Payload += FString::Printf(TEXT("%.3f|%d;"), Lane.LengthCm, Lane.Layer);
    }
    return PinkCabWorldId::StableToken(TEXT("topology:"), Payload);
}

int32 FPinkCabRoadGraph::NumLanes() const { return Lanes.Num(); }
