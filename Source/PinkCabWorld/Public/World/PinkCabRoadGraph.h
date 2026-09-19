#pragma once

#include "CoreMinimal.h"
#include "World/PinkCabChunkId.h"

struct PINKCABWORLD_API FPinkCabRoadNodeId
{
    FPinkCabRoadNodeId();
    explicit FPinkCabRoadNodeId(const FString& InValue);
    bool IsValid() const;
    const FString& Serialize() const;
    bool operator==(const FPinkCabRoadNodeId& Other) const;

private:
    FString Value;
};

struct PINKCABWORLD_API FPinkCabLaneId
{
    FPinkCabLaneId();
    explicit FPinkCabLaneId(const FString& InValue);
    bool IsValid() const;
    const FString& Serialize() const;
    bool operator==(const FPinkCabLaneId& Other) const;

private:
    FString Value;
};

struct FPinkCabLogicalLane
{
    FPinkCabLaneId LaneId;
    FPinkCabRoadNodeId FromNode;
    FPinkCabRoadNodeId ToNode;
    double LengthCm = 1.0;
    int32 Layer = 0;
};

class PINKCABWORLD_API FPinkCabRoadGraph
{
public:
    static FPinkCabRoadNodeId MakeNodeId(
        const FPinkCabCityIdentity& City,
        const FPinkCabChunkId& Chunk,
        int32 LocalNodeIndex);
    static FPinkCabLaneId MakeLaneId(
        const FPinkCabCityIdentity& City,
        const FPinkCabChunkId& Chunk,
        int32 LocalRoadIndex,
        int32 LocalLaneIndex);

    bool AddLane(const FPinkCabLogicalLane& Lane);
    const FPinkCabLogicalLane* FindLane(const FPinkCabLaneId& LaneId) const;
    TArray<FPinkCabLaneId> GetNextLanes(const FPinkCabLaneId& CurrentLane) const;
    TArray<FPinkCabLaneId> GetLaneIds() const;
    FString GetTopologySignature() const;
    int32 NumLanes() const;

private:
    TMap<FString, FPinkCabLogicalLane> Lanes;
    TMap<FString, TArray<FString>> OutgoingByNode;
};
