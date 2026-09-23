#pragma once

#include "Vehicle/PinkCabVehicleHealthState.h"

class PINKCABVEHICLE_API FPinkCabVehicleHealthBinding
{
public:
    const FPinkCabVehicleHealthState& Get() const;
    FPinkCabVehicleHealthState& GetMutable();
    void Bind(FPinkCabVehicleHealthState& InExternal);
    void UnbindPreservingState();
    bool IsBound() const;

private:
    FPinkCabVehicleHealthState Fallback;
    FPinkCabVehicleHealthState* External = nullptr;
};
