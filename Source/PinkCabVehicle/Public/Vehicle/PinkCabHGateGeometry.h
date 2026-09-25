#pragma once

#include "CoreMinimal.h"

struct FPinkCabHGateState
{
    int32 RequestedGear = 0;
    int32 LastGateRow = 0;
    int32 LastGateColumn = 1;
    float LeverX = 1.0f;
    float LeverY = 0.0f;
};

class PINKCABVEHICLE_API FPinkCabHGateGeometry
{
public:
    static bool MoveGate(FPinkCabHGateState& State, float X, float Y);
    static bool ApplyDriverDelta(
        FPinkCabHGateState& State,
        float DriverRightCounts,
        float DriverForwardCounts);
    static void ResetToGear(FPinkCabHGateState& State, int32 Gear);

private:
    static int32 GearColumn(int32 Gear);
};
