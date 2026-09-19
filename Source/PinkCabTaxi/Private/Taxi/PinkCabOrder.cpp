#include "Taxi/PinkCabOrder.h"

bool FPinkCabOrder::IsValid() const
{
    return OrderId.IsValid()
        && PickupId.IsValid()
        && DestinationId.IsValid()
        && PassengerIdentityIds.Num() >= 1
        && PassengerIdentityIds.Num() <= 5;
}
