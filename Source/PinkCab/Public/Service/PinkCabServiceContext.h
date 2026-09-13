#pragma once

#include "CoreMinimal.h"
#include "Core/PinkCabStableId.h"
#include "World/PinkCabCityIdentity.h"

struct FPinkCabServiceOwnerIdentity
{
    FPinkCabStableId VehicleId;
    FPinkCabStableId BuildOwnerId;
    FPinkCabStableId HealthOwnerId;
    FPinkCabStableId InventoryOwnerId;
    FPinkCabStableId EconomyOwnerId;

    bool IsValid() const
    {
        return VehicleId.IsValid() && BuildOwnerId.IsValid()
            && HealthOwnerId.IsValid() && InventoryOwnerId.IsValid()
            && EconomyOwnerId.IsValid();
    }

    bool IsSameAs(const FPinkCabServiceOwnerIdentity& Other) const
    {
        return VehicleId == Other.VehicleId && BuildOwnerId == Other.BuildOwnerId
            && HealthOwnerId == Other.HealthOwnerId
            && InventoryOwnerId == Other.InventoryOwnerId
            && EconomyOwnerId == Other.EconomyOwnerId;
    }
};
struct FPinkCabServiceContext
{
    FPinkCabCityIdentity City;
    FString SemanticKey;
    FString ServiceNodeId;
    FPinkCabServiceOwnerIdentity Owners;

    bool IsValid() const
    {
        return City.IsValid() && !SemanticKey.IsEmpty()
            && !ServiceNodeId.IsEmpty() && Owners.IsValid();
    }

    static FString MakeNodeId(
        const FPinkCabCityIdentity& City,
        uint8 KindValue,
        const FString& SemanticKey)
    {
        const FString CleanKey = SemanticKey.TrimStartAndEnd();
        if (!City.IsValid() || CleanKey.IsEmpty()) return FString();
        const FString Payload = FString::Printf(
            TEXT("%s|%u|%s"), *City.GetStableKey(), KindValue, *CleanKey);
        return PinkCabWorldId::StableToken(TEXT("service-node:"), Payload);
    }
};
