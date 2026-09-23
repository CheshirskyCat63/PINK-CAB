#pragma once

#include "CoreMinimal.h"
#include "World/PinkCabRoadGraph.h"

enum class EPinkCabCityDeltaKind : uint8
{
    LaneClosure,
    LaneSlowdown,
    ModuleOverride,
    Incident
};

struct FPinkCabCityDeltaRecord
{
    FString DeltaId;
    EPinkCabCityDeltaKind Kind = EPinkCabCityDeltaKind::LaneClosure;
    FString SubjectStableId;
    FString OperationKey;
};

class PINKCABWORLD_API FPinkCabCityDeltaState
{
public:
    explicit FPinkCabCityDeltaState(int32 InMaxDeltas = 256);

    static FString MakeDeltaId(
        const FPinkCabCityIdentity& City,
        EPinkCabCityDeltaKind Kind,
        const FString& SubjectStableId,
        const FString& OperationKey);
    bool TryAddLaneClosure(
        const FPinkCabCityIdentity& City,
        const FPinkCabLaneId& LaneId,
        const FString& OperationKey,
        FString& OutDeltaId);
    bool IsLaneClosed(const FPinkCabLaneId& LaneId) const;
    int32 Num() const;
    FString GetReconstructionSignature() const;

private:
    friend class FPinkCabWorldSessionSnapshotCodec;

    static FString KindToken(EPinkCabCityDeltaKind Kind);

    int32 MaxDeltas = 256;
    TMap<FString, FPinkCabCityDeltaRecord> Deltas;
    TSet<FString> ClosedLaneIds;
};
