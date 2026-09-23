#pragma once

#include "CoreMinimal.h"
#include "World/PinkCabCityIdentity.h"
#include "World/PinkCabRoadGraph.h"

enum class EPinkCabVerticalContactKind : uint8
{
    WallLeft,
    WallRight,
    FreightCeiling,
    ReceivingStrip,
    PoplarGapHook
};

struct PINKCABWORLD_API FPinkCabVerticalContactRecord
{
    FString ContactId;
    FPinkCabLaneId LaneId;
    EPinkCabVerticalContactKind Kind = EPinkCabVerticalContactKind::WallLeft;
    FString SemanticKey;
    int32 Layer = 0;

    bool IsValid() const;
};

class PINKCABWORLD_API FPinkCabVerticalContactRegistry
{
public:
    explicit FPinkCabVerticalContactRegistry(int32 InMaxContacts = 256);

    static FString MakeContactId(
        const FPinkCabCityIdentity& City,
        const FPinkCabLaneId& LaneId,
        EPinkCabVerticalContactKind Kind,
        const FString& SemanticKey);
    bool TryAdd(
        const FPinkCabCityIdentity& City,
        const FPinkCabLaneId& LaneId,
        EPinkCabVerticalContactKind Kind,
        const FString& SemanticKey,
        int32 Layer,
        FString& OutContactId);
    bool TryGet(const FString& ContactId, FPinkCabVerticalContactRecord& OutRecord) const;
    int32 Num() const;
    FString GetReconstructionSignature() const;

private:
    int32 MaxContacts = 256;
    TMap<FString, FPinkCabVerticalContactRecord> Contacts;
};
