#pragma once

#include "CoreMinimal.h"

struct FPinkCabPlayerInputSample
{
    bool bSystemMenuToggleRequested = false;
    bool bGazeHeld = false;
    bool bQuickRecall1Held = false;
    bool bQuickRecall2Held = false;
    bool bQuickRecall3Held = false;
    bool bQuickRecall4Held = false;
    bool bGripHeld = false;
    bool bMomentaryHeld = false;
    bool bClutchHeld = false;
    bool bBrakeHeld = false;
    bool bThrottleHeld = false;

    float LookMouseX = 0.0f;
    float LookMouseY = 0.0f;
    float DeviceX = 0.0f;
    float DeviceY = 0.0f;
    int32 WheelSteps = 0;
};
