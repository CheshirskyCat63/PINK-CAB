#pragma once

#include "CoreMinimal.h"

class FPinkCabTaximeter;
struct FPinkCabCockpitPresentationState;
struct FPinkCabCockpitState;

struct FPinkCabCockpitServiceSources
{
    const FPinkCabTaximeter* Taximeter = nullptr;
    const FPinkCabCockpitState* CockpitState = nullptr;
    TOptional<float> RouteProgress01;
    bool bRadioAvailable = false;
    bool bMirrorsAvailable = false;
};

struct FPinkCabCockpitServicePresentation
{
    bool bMeterAvailable = false;
    bool bMeterRunning = false;
    int64 FareMinor = 0;
    double DistanceKm = 0.0;
    double FareSeconds = 0.0;
    bool bDoorAvailable = false;
    bool bPassengerDoorOpen = false;
    bool bRouteAvailable = false;
    float RouteProgress01 = 0.0f;
    bool bRadioAvailable = false;
    bool bMirrorsAvailable = false;
};

class PINKCAB_API FPinkCabCockpitServiceBridge
{
public:
    static FPinkCabCockpitServicePresentation Read(
        const FPinkCabCockpitServiceSources& Sources);

    static void ApplyToPresentation(
        const FPinkCabCockpitServicePresentation& Services,
        FPinkCabCockpitPresentationState& InOutPresentation);
};
