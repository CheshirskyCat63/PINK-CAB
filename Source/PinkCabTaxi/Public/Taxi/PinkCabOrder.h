#pragma once

#include "CoreMinimal.h"
#include "Core/PinkCabStableId.h"

enum class EPinkCabFareMode : uint8
{
    Metered,
    OffMeter
};

struct PINKCABTAXI_API FPinkCabOrder
{
    FPinkCabStableId OrderId;
    FPinkCabStableId PickupId;
    FPinkCabStableId DestinationId;
    TArray<FPinkCabStableId> PassengerIdentityIds;
    EPinkCabFareMode FareMode = EPinkCabFareMode::Metered;

    bool IsValid() const;
};
