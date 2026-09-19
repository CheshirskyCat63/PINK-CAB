#include "Service/PinkCabServiceContext.h"

bool FPinkCabServiceOwnerIdentity::IsValid() const
{
    return VehicleId.IsValid()
        && BuildOwnerId.IsValid()
        && HealthOwnerId.IsValid()
        && InventoryOwnerId.IsValid()
        && EconomyOwnerId.IsValid();
}

bool FPinkCabServiceOwnerIdentity::IsSameAs(
    const FPinkCabServiceOwnerIdentity& Other) const
{
    return VehicleId == Other.VehicleId
        && BuildOwnerId == Other.BuildOwnerId
        && HealthOwnerId == Other.HealthOwnerId
        && InventoryOwnerId == Other.InventoryOwnerId
        && EconomyOwnerId == Other.EconomyOwnerId;
}

bool FPinkCabServiceContext::IsValid() const
{
    return City.IsValid()
        && !SemanticKey.IsEmpty()
        && !ServiceNodeId.IsEmpty()
        && Owners.IsValid();
}

FString FPinkCabServiceContext::MakeNodeId(
    const FPinkCabCityIdentity& City,
    uint8 KindValue,
    const FString& SemanticKey)
{
    const FString CleanKey = SemanticKey.TrimStartAndEnd();
    if (!City.IsValid() || CleanKey.IsEmpty())
    {
        return FString();
    }

    const FString Payload = FString::Printf(
        TEXT("%s|%u|%s"),
        *City.GetStableKey(),
        KindValue,
        *CleanKey);
    return PinkCabWorldId::StableToken(
        TEXT("service-node:"),
        Payload);
}
