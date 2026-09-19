#pragma once

#include "CoreMinimal.h"
#include "Core/PinkCabStableId.h"
#include "World/PinkCabCityIdentity.h"

struct PINKCAB_API FPinkCabServiceOwnerIdentity
{
    FPinkCabStableId VehicleId;
    FPinkCabStableId BuildOwnerId;
    FPinkCabStableId HealthOwnerId;
    FPinkCabStableId InventoryOwnerId;
    FPinkCabStableId EconomyOwnerId;

    bool IsValid() const;
    bool IsSameAs(
        const FPinkCabServiceOwnerIdentity& Other) const;
};

struct PINKCAB_API FPinkCabServiceContext
{
    FPinkCabCityIdentity City;
    FString SemanticKey;
    FString ServiceNodeId;
    FPinkCabServiceOwnerIdentity Owners;

    bool IsValid() const;
    static FString MakeNodeId(
        const FPinkCabCityIdentity& City,
        uint8 KindValue,
        const FString& SemanticKey);
};
