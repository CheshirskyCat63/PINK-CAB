#pragma once

#include "Vehicle/PinkCabL1TraversalConfig.h"
#include "Vehicle/PinkCabL1TraversalTypes.h"

class PINKCAB_API FPinkCabL1TraversalTransitionPolicy
{
public:
    static bool IsWallKind(EPinkCabVerticalContactKind Kind);
    static bool CanEnterFromRoad(
        const FPinkCabL1TraversalConfig& Config,
        EPinkCabVerticalContactKind Kind);
    static bool CanEnterFromWall(
        const FPinkCabL1TraversalConfig& Config,
        EPinkCabVerticalContactKind Kind);
    static bool CanEnterFromFreight(
        const FPinkCabL1TraversalConfig& Config,
        EPinkCabVerticalContactKind Kind);
    static bool CanEnterFromGap(
        const FPinkCabL1TraversalConfig& Config,
        EPinkCabVerticalContactKind Kind);
};
