#include "World/PinkCabL1RoadConstructionModel.h"

#include "World/PinkCabL1EndlessRoadModel.h"

FPinkCabL1RoadConstructionBand
FPinkCabL1RoadConstructionModel::ResolveServiceSeparator(
    const double LocalLongitudinalCm)
{
    FPinkCabL1RoadConstructionBand Result;

    const double WidthCm =
        FPinkCabL1EndlessRoadModel::ResolveAccessSeparatorWidthCm(
            LocalLongitudinalCm);
    Result.WidthCm = FMath::Clamp(
        WidthCm,
        0.0,
        ServiceOuterEdgeAbsYCm - ExpressOuterEdgeAbsYCm);
    Result.bPresent = Result.WidthCm > KINDA_SMALL_NUMBER;

    if (Result.bPresent)
    {
        // The outer separator edge remains pinned to the local carriageway.
        // R1 connector pavement grows from the express side into this band.
        Result.CenterAbsYCm =
            ServiceOuterEdgeAbsYCm - Result.WidthCm * 0.5;
    }
    else
    {
        // Keep a deterministic anchor even when the band is fully open.
        Result.CenterAbsYCm = ServiceOuterEdgeAbsYCm;
    }

    return Result;
}

double FPinkCabL1RoadConstructionModel::ResolveOuterCurbCenterAbsYCm()
{
    return RoadHalfWidthCm - CurbWidthCm * 0.5;
}
