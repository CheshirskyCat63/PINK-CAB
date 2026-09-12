#pragma once

#include "CoreMinimal.h"
#include "World/PinkCabChunkId.h"

struct FPinkCabRoadNodeId
{
    FPinkCabRoadNodeId() = default;
    explicit FPinkCabRoadNodeId(const FString& InValue) : Value(InValue) {}
    bool IsValid() const { return !Value.IsEmpty(); }
    const FString& Serialize() const { return Value; }
    bool operator==(const FPinkCabRoadNodeId& Other) const { return Value == Other.Value; }
private:
    FString Value;
};

struct FPinkCabLaneId
{
    FPinkCabLaneId() = default;
    explicit FPinkCabLaneId(const FString& InValue) : Value(InValue) {}
    bool IsValid() const { return !Value.IsEmpty(); }
    const FString& Serialize() const { return Value; }
    bool operator==(const FPinkCabLaneId& Other) const { return Value == Other.Value; }
private:
    FString Value;
};
struct FPinkCabLogicalLane
{
    FPinkCabLaneId LaneId;
    FPinkCabRoadNodeId FromNode;
    FPinkCabRoadNodeId ToNode;
};

class FPinkCabRoadGraph
{
public:
    static FPinkCabRoadNodeId MakeNodeId(
        const FPinkCabCityIdentity& City,
        const FPinkCabChunkId& Chunk,
        int32 LocalNodeIndex)
    {
        const FString Payload = FString::Printf(
            TEXT("%s|%s|node|%d"),
            *City.GetStableKey(), *Chunk.Serialize(), LocalNodeIndex);
        return FPinkCabRoadNodeId(PinkCabWorldId::StableToken(TEXT("node:"), Payload));
    }
    static FPinkCabLaneId MakeLaneId(
        const FPinkCabCityIdentity& City,
        const FPinkCabChunkId& Chunk,
        int32 LocalRoadIndex,
        int32 LocalLaneIndex)
    {
        const FString Payload = FString::Printf(
            TEXT("%s|%s|road|%d|lane|%d"),
            *City.GetStableKey(), *Chunk.Serialize(), LocalRoadIndex, LocalLaneIndex);
        return FPinkCabLaneId(PinkCabWorldId::StableToken(TEXT("lane:"), Payload));
    }

    bool AddLane(const FPinkCabLogicalLane& Lane)
    {
        if (!Lane.LaneId.IsValid() || !Lane.FromNode.IsValid() || !Lane.ToNode.IsValid())
        {
            return false;
        }
        const FString Key = Lane.LaneId.Serialize();
        if (Lanes.Contains(Key))
        {
            return false;
        }
        Lanes.Add(Key, Lane);
        OutgoingByNode.FindOrAdd(Lane.FromNode.Serialize()).Add(Key);
        return true;
    }
    TArray<FPinkCabLaneId> GetNextLanes(const FPinkCabLaneId& CurrentLane) const
    {
        TArray<FPinkCabLaneId> Result;
        const FPinkCabLogicalLane* Current = Lanes.Find(CurrentLane.Serialize());
        if (!Current)
        {
            return Result;
        }
        const TArray<FString>* NextKeys = OutgoingByNode.Find(Current->ToNode.Serialize());
        if (!NextKeys)
        {
            return Result;
        }
        Result.Reserve(NextKeys->Num());
        for (const FString& Key : *NextKeys)
        {
            Result.Add(FPinkCabLaneId(Key));
        }
        return Result;
    }

    FString GetTopologySignature() const
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
            Payload += TEXT(";");
        }
        return PinkCabWorldId::StableToken(TEXT("topology:"), Payload);
    }

    int32 NumLanes() const { return Lanes.Num(); }

private:
    TMap<FString, FPinkCabLogicalLane> Lanes;
    TMap<FString, TArray<FString>> OutgoingByNode;
};
