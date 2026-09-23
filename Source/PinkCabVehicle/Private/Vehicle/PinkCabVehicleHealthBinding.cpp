#include "Vehicle/PinkCabVehicleHealthBinding.h"

const FPinkCabVehicleHealthState& FPinkCabVehicleHealthBinding::Get() const
{
    return External ? *External : Fallback;
}

FPinkCabVehicleHealthState& FPinkCabVehicleHealthBinding::GetMutable()
{
    return External ? *External : Fallback;
}

void FPinkCabVehicleHealthBinding::Bind(FPinkCabVehicleHealthState& InExternal)
{
    External = &InExternal;
}

void FPinkCabVehicleHealthBinding::UnbindPreservingState()
{
    if (External)
    {
        Fallback = *External;
        External = nullptr;
    }
}

bool FPinkCabVehicleHealthBinding::IsBound() const
{
    return External != nullptr;
}
