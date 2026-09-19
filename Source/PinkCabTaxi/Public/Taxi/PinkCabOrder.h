#pragma once

#include "CoreMinimal.h"
#include "Core/PinkCabStableId.h"

enum class EPinkCabFareMode : uint8
{
    Metered,
    OffMeter
};

struct FPinkCabOrder
{
    FPinkCabStableId OrderId;
    FPinkCabStableId PickupId;
    FPinkCabStableId DestinationId;
    TArray<FPinkCabStableId> PassengerIdentityIds;
    EPinkCabFareMode FareMode = EPinkCabFareMode::Metered;

    bool IsValid() const
    {
        return OrderId.IsValid() && PickupId.IsValid() && DestinationId.IsValid()
            && PassengerIdentityIds.Num() >= 1 && PassengerIdentityIds.Num() <= 5;
    }
};
