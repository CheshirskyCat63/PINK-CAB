#pragma once

#include "CoreMinimal.h"

struct PINKCABWORLD_API FPinkCabL1RoadConstructionBand
{
    bool bPresent = false;
    double WidthCm = 0.0;
    double CenterAbsYCm = 0.0;
};

class PINKCABWORLD_API FPinkCabL1RoadConstructionModel
{
public:
    static constexpr double ChunkLengthCm = 100000.0;

    // Frozen R1 cross-section, measured from road centerline.
    static constexpr double CentralMedianWidthCm = 800.0;
    static constexpr double RoadHalfWidthCm = 3340.0;
    static constexpr double ExpressOuterEdgeAbsYCm = 2200.0;
    static constexpr double ServiceOuterEdgeAbsYCm = 2600.0;
    static constexpr double LocalOuterEdgeAbsYCm = 3240.0;

    // R2 construction calibration. These are implementation values for the
    // first human-gated construction pass, not new world-canon dimensions.
    static constexpr double MedianDeckHeightCm = 6.0;
    static constexpr double ServiceDeckHeightCm = 6.0;
    static constexpr double CurbHeightCm = 12.0;
    static constexpr double CurbWidthCm = 18.0;
    static constexpr double ConstructionSliceLengthCm = 500.0;

    static FPinkCabL1RoadConstructionBand ResolveServiceSeparator(
        double LocalLongitudinalCm);

    static double ResolveOuterCurbCenterAbsYCm();
};
