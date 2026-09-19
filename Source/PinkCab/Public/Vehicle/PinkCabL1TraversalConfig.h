#pragma once

struct FPinkCabL1TraversalConfig
{
    bool bAllowRoadToWall = true;
    bool bAllowRoadToFreight = true;
    bool bAllowRoadToGapTransfer = true;
    bool bAllowWallToFreight = true;
    bool bAllowWallToGapTransfer = true;
    bool bAllowWallToReceiving = true;
    bool bAllowFreightToReceiving = true;
    bool bAllowGapToReceiving = true;
};
