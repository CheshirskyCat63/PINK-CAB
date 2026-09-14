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

class FPinkCabCityDeltaState
{
public:
    explicit FPinkCabCityDeltaState(int32 InMaxDeltas = 256)
        : MaxDeltas(FMath::Max(1, InMaxDeltas))
    {
    }

    static FString MakeDeltaId(
        const FPinkCabCityIdentity& City,
        EPinkCabCityDeltaKind Kind,
        const FString& SubjectStableId,
        const FString& OperationKey)
    {
        const FString CleanOperation = OperationKey.TrimStartAndEnd();
        if (!City.IsValid() || SubjectStableId.IsEmpty() || CleanOperation.IsEmpty())
        {
            return FString();
        }
        const FString Payload = FString::Printf(
            TEXT("%s|%s|%s|%s"),
            *City.GetStableKey(), *KindToken(Kind), *SubjectStableId, *CleanOperation);
        return PinkCabWorldId::StableToken(TEXT("delta:"), Payload);
    }

    bool TryAddLaneClosure(
        const FPinkCabCityIdentity& City,
        const FPinkCabLaneId& LaneId,
        const FString& OperationKey,
        FString& OutDeltaId)
    {
        const FString DeltaId = MakeDeltaId(
            City, EPinkCabCityDeltaKind::LaneClosure, LaneId.Serialize(), OperationKey);
        if (DeltaId.IsEmpty() || Deltas.Contains(DeltaId) || Deltas.Num() >= MaxDeltas)
        {
            return false;
        }
        FPinkCabCityDeltaRecord Record;
        Record.DeltaId = DeltaId;
        Record.Kind = EPinkCabCityDeltaKind::LaneClosure;
        Record.SubjectStableId = LaneId.Serialize();
        Record.OperationKey = OperationKey.TrimStartAndEnd();
        Deltas.Add(DeltaId, MoveTemp(Record));
        ClosedLaneIds.Add(LaneId.Serialize());
        OutDeltaId = DeltaId;
        return true;
    }

    bool IsLaneClosed(const FPinkCabLaneId& LaneId) const
    {
        return LaneId.IsValid() && ClosedLaneIds.Contains(LaneId.Serialize());
    }

    int32 Num() const
    {
        return Deltas.Num();
    }

    FString GetReconstructionSignature() const
    {
        TArray<FString> Keys;
        Deltas.GetKeys(Keys);
        Keys.Sort();
        FString Payload;
        for (const FString& Key : Keys)
        {
            const FPinkCabCityDeltaRecord& Record = Deltas[Key];
            Payload += Record.DeltaId;
            Payload += TEXT("|");
            Payload += KindToken(Record.Kind);
            Payload += TEXT("|");
            Payload += Record.SubjectStableId;
            Payload += TEXT("|");
            Payload += Record.OperationKey;
            Payload += TEXT(";");
        }
        return PinkCabWorldId::StableToken(TEXT("city-deltas:"), Payload);
    }

private:
    friend class FPinkCabWorldSessionSnapshotCodec;

    static FString KindToken(EPinkCabCityDeltaKind Kind)
    {
        switch (Kind)
        {
        case EPinkCabCityDeltaKind::LaneClosure: return TEXT("lane-closure");
        case EPinkCabCityDeltaKind::LaneSlowdown: return TEXT("lane-slowdown");
        case EPinkCabCityDeltaKind::ModuleOverride: return TEXT("module-override");
        case EPinkCabCityDeltaKind::Incident: return TEXT("incident");
        default: return TEXT("unknown");
        }
    }

    int32 MaxDeltas = 256;
    TMap<FString, FPinkCabCityDeltaRecord> Deltas;
    TSet<FString> ClosedLaneIds;
};
