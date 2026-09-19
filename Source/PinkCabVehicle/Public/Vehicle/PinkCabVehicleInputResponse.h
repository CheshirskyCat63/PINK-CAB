#pragma once

#include "CoreMinimal.h"

struct PINKCABVEHICLE_API FPinkCabVehicleInputResponse
{
    static float StepAxis(
        float Current,
        float Target,
        float DeltaSeconds,
        float PressSeconds,
        float ReleaseSeconds);
};
