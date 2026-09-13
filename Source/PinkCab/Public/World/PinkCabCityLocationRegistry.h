#pragma once

#include "CoreMinimal.h"
#include "World/PinkCabRoadGraph.h"

enum class EPinkCabCityLocationKind : uint8
{
    Pickup,
    Destination,
    Service,
    Rule
};

struct FPinkCabCityLocationAnchor
{
    FString AnchorId;
    EPinkCabCityLocationKind Kind = EPinkCabCityLocationKind::Pickup;
    FPinkCabLaneId LaneId;
    double LongitudinalCm = 0.0;
    int32 Layer = 0;

    bool IsValid() const
    {
        return !AnchorId.IsEmpty() && LaneId.IsValid()
            && FMath::IsFinite(LongitudinalCm) && LongitudinalCm >= 0.0;
    }
};

class FPinkCabCityLocationRegistry
{
public:
    explicit FPinkCabCityLocationRegistry(int32 InMaxAnchors = 256)
        : MaxAnchors(FMath::Max(1, InMaxAnchors))
    {
    }

    static FString MakeAnchorId(
        const FPinkCabCityIdentity& City,
        const FPinkCabLaneId& LaneId,
        EPinkCabCityLocationKind Kind,
        const FString& SemanticKey)
    {
        const FString CleanKey = SemanticKey.TrimStartAndEnd();
        if (!City.IsValid() || !LaneId.IsValid() || CleanKey.IsEmpty())
        {
            return FString();
        }
        const FString Payload = FString::Printf(
            TEXT("%s|%s|%s|%s"),
            *City.GetStableKey(), *LaneId.Serialize(), *KindToken(Kind), *CleanKey);
        return PinkCabWorldId::StableToken(TEXT("anchor:"), Payload);
    }

    bool TryRegister(const FPinkCabCityLocationAnchor& Anchor)
    {
        if (!Anchor.IsValid() || Anchors.Contains(Anchor.AnchorId) || Anchors.Num() >= MaxAnchors)
        {
            return false;
        }
        Anchors.Add(Anchor.AnchorId, Anchor);
        return true;
    }
    bool TryGet(const FString& AnchorId, FPinkCabCityLocationAnchor& OutAnchor) const
    {
        const FPinkCabCityLocationAnchor* Found = Anchors.Find(AnchorId);
        if (!Found)
        {
            return false;
        }
        OutAnchor = *Found;
        return true;
    }

    int32 Num() const
    {
        return Anchors.Num();
    }

    TArray<FPinkCabCityLocationAnchor> GetAnchorsSorted() const
    {
        TArray<FString> Keys;
        Anchors.GetKeys(Keys);
        Keys.Sort();
        TArray<FPinkCabCityLocationAnchor> Result;
        Result.Reserve(Keys.Num());
        for (const FString& Key : Keys)
        {
            Result.Add(Anchors[Key]);
        }
        return Result;
    }

    FString GetReconstructionSignature() const
    {
        TArray<FString> Keys;
        Anchors.GetKeys(Keys);
        Keys.Sort();
        FString Payload;
        for (const FString& Key : Keys)
        {
            const FPinkCabCityLocationAnchor& Anchor = Anchors[Key];
            Payload += Anchor.AnchorId;
            Payload += TEXT("|");
            Payload += KindToken(Anchor.Kind);
            Payload += TEXT("|");
            Payload += Anchor.LaneId.Serialize();
            Payload += TEXT("|");
            Payload += FString::Printf(TEXT("%.6f|%d;"), Anchor.LongitudinalCm, Anchor.Layer);
        }
        return PinkCabWorldId::StableToken(TEXT("city-anchors:"), Payload);
    }

private:
    static FString KindToken(EPinkCabCityLocationKind Kind)
    {
        switch (Kind)
        {
        case EPinkCabCityLocationKind::Pickup: return TEXT("pickup");
        case EPinkCabCityLocationKind::Destination: return TEXT("destination");
        case EPinkCabCityLocationKind::Service: return TEXT("service");
        case EPinkCabCityLocationKind::Rule: return TEXT("rule");
        default: return TEXT("unknown");
        }
    }

    int32 MaxAnchors = 256;
    TMap<FString, FPinkCabCityLocationAnchor> Anchors;
};
