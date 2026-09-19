#pragma once

#include "Interaction/PinkCabInteractionModel.h"
#include "Vehicle/PinkCabCockpitState.h"

struct PINKCAB_API FPinkCabCockpitInteractionRouter
{
    static constexpr float HandbrakeWheelStep = 0.25f;

    static bool Apply(const FPinkCabInteractionEvent& Event, FPinkCabCockpitState& State);
};
