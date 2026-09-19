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

struct PINKCABWORLD_API FPinkCabCityLocationAnchor
{
    FString AnchorId;
    EPinkCabCityLocationKind Kind = EPinkCabCityLocationKind::Pickup;
    FPinkCabLaneId LaneId;
    double LongitudinalCm = 0.0;
    int32 Layer = 0;

    bool IsValid() const;
};

class PINKCABWORLD_API FPinkCabCityLocationRegistry
{
public:
    explicit FPinkCabCityLocationRegistry(int32 InMaxAnchors = 256);

    static FString MakeAnchorId(
        const FPinkCabCityIdentity& City,
        const FPinkCabLaneId& LaneId,
        EPinkCabCityLocationKind Kind,
        const FString& SemanticKey);
    bool TryRegister(const FPinkCabCityLocationAnchor& Anchor);
    bool TryGet(const FString& AnchorId, FPinkCabCityLocationAnchor& OutAnchor) const;
    int32 Num() const;
    TArray<FPinkCabCityLocationAnchor> GetAnchorsSorted() const;
    FString GetReconstructionSignature() const;

private:
    static FString KindToken(EPinkCabCityLocationKind Kind);

    int32 MaxAnchors = 256;
    TMap<FString, FPinkCabCityLocationAnchor> Anchors;
};
