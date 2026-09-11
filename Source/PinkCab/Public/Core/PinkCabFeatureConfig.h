#pragma once

#include "CoreMinimal.h"

enum class EPinkCabFeature : uint8
{
    L2 = 0,
    Damage,
    Neural,
    MovingFuel,
    ServiceNodes
};

struct FPinkCabFeatureConfig
{
    void SetEnabled(EPinkCabFeature Feature, bool bEnabled)
    {
        Flags[static_cast<uint8>(Feature)] = bEnabled;
    }

    bool IsEnabled(EPinkCabFeature Feature) const
    {
        return Flags[static_cast<uint8>(Feature)];
    }

private:
    bool Flags[5] = {false, false, false, false, false};
};
