#include "World/PinkCabCityLocationRegistry.h"

bool FPinkCabCityLocationAnchor::IsValid() const
{
    return !AnchorId.IsEmpty()
        && LaneId.IsValid()
        && FMath::IsFinite(LongitudinalCm)
        && LongitudinalCm >= 0.0;
}

FPinkCabCityLocationRegistry::FPinkCabCityLocationRegistry(int32 InMaxAnchors)
    : MaxAnchors(FMath::Max(1, InMaxAnchors))
{
}

FString FPinkCabCityLocationRegistry::MakeAnchorId(
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
        *City.GetStableKey(),
        *LaneId.Serialize(),
        *KindToken(Kind),
        *CleanKey);
    return PinkCabWorldId::StableToken(TEXT("anchor:"), Payload);
}

bool FPinkCabCityLocationRegistry::TryRegister(const FPinkCabCityLocationAnchor& Anchor)
{
    if (!Anchor.IsValid() || Anchors.Contains(Anchor.AnchorId) || Anchors.Num() >= MaxAnchors)
    {
        return false;
    }
    Anchors.Add(Anchor.AnchorId, Anchor);
    return true;
}

bool FPinkCabCityLocationRegistry::TryGet(
    const FString& AnchorId,
    FPinkCabCityLocationAnchor& OutAnchor) const
{
    const FPinkCabCityLocationAnchor* Found = Anchors.Find(AnchorId);
    if (!Found) return false;

    OutAnchor = *Found;
    return true;
}

int32 FPinkCabCityLocationRegistry::Num() const { return Anchors.Num(); }

TArray<FPinkCabCityLocationAnchor> FPinkCabCityLocationRegistry::GetAnchorsSorted() const
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

FString FPinkCabCityLocationRegistry::GetReconstructionSignature() const
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

FString FPinkCabCityLocationRegistry::KindToken(EPinkCabCityLocationKind Kind)
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
