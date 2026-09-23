#include "Vehicle/PinkCabL1TraversalTransitionPolicy.h"

bool FPinkCabL1TraversalTransitionPolicy::IsWallKind(
    const EPinkCabVerticalContactKind Kind)
{
    return Kind == EPinkCabVerticalContactKind::WallLeft
        || Kind == EPinkCabVerticalContactKind::WallRight;
}

bool FPinkCabL1TraversalTransitionPolicy::CanEnterFromRoad(
    const FPinkCabL1TraversalConfig& Config,
    const EPinkCabVerticalContactKind Kind)
{
    if (IsWallKind(Kind)) return Config.bAllowRoadToWall;
    if (Kind == EPinkCabVerticalContactKind::FreightCeiling) return Config.bAllowRoadToFreight;
    if (Kind == EPinkCabVerticalContactKind::PoplarGapHook) return Config.bAllowRoadToGapTransfer;
    return false;
}

bool FPinkCabL1TraversalTransitionPolicy::CanEnterFromWall(
    const FPinkCabL1TraversalConfig& Config,
    const EPinkCabVerticalContactKind Kind)
{
    if (IsWallKind(Kind)) return true;
    if (Kind == EPinkCabVerticalContactKind::FreightCeiling) return Config.bAllowWallToFreight;
    if (Kind == EPinkCabVerticalContactKind::PoplarGapHook) return Config.bAllowWallToGapTransfer;
    if (Kind == EPinkCabVerticalContactKind::ReceivingStrip) return Config.bAllowWallToReceiving;
    return false;
}

bool FPinkCabL1TraversalTransitionPolicy::CanEnterFromFreight(
    const FPinkCabL1TraversalConfig& Config,
    const EPinkCabVerticalContactKind Kind)
{
    if (Kind == EPinkCabVerticalContactKind::FreightCeiling) return true;
    return Kind == EPinkCabVerticalContactKind::ReceivingStrip
        && Config.bAllowFreightToReceiving;
}

bool FPinkCabL1TraversalTransitionPolicy::CanEnterFromGap(
    const FPinkCabL1TraversalConfig& Config,
    const EPinkCabVerticalContactKind Kind)
{
    return Kind == EPinkCabVerticalContactKind::ReceivingStrip
        && Config.bAllowGapToReceiving;
}
