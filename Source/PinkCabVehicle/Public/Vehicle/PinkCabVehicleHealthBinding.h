#pragma once

#include "Vehicle/PinkCabVehicleHealthState.h"

class FPinkCabVehicleHealthBinding
{
public:
    const FPinkCabVehicleHealthState& Get() const
    {
        return External ? *External : Fallback;
    }

    FPinkCabVehicleHealthState& GetMutable()
    {
        return External ? *External : Fallback;
    }

    void Bind(FPinkCabVehicleHealthState& InExternal)
    {
        External = &InExternal;
    }

    void UnbindPreservingState()
    {
        if (External)
        {
            Fallback = *External;
            External = nullptr;
        }
    }

    bool IsBound() const { return External != nullptr; }

private:
    FPinkCabVehicleHealthState Fallback;
    FPinkCabVehicleHealthState* External = nullptr;
};
