#include "Cockpit/PinkCabCockpitServiceBridge.h"

#include "Cockpit/PinkCabCockpitPresentationState.h"
#include "Taxi/PinkCabTaximeter.h"
#include "Vehicle/PinkCabCockpitState.h"

FPinkCabCockpitServicePresentation FPinkCabCockpitServiceBridge::Read(
    const FPinkCabCockpitServiceSources& Sources)
{
    FPinkCabCockpitServicePresentation Result;
    if (Sources.Taximeter)
    {
        Result.bMeterAvailable = true;
        Result.bMeterRunning = Sources.Taximeter->IsRunning();
        Result.FareMinor = Sources.Taximeter->GetFareMinor();
        Result.DistanceKm = Sources.Taximeter->GetDistanceKm();
        Result.FareSeconds = Sources.Taximeter->GetFareSeconds();
    }
    if (Sources.CockpitState)
    {
        Result.bDoorAvailable = true;
        Result.bPassengerDoorOpen = Sources.CockpitState->IsPassengerDoorOpen();
    }
    if (Sources.RouteProgress01.IsSet() && FMath::IsFinite(Sources.RouteProgress01.GetValue()))
    {
        Result.bRouteAvailable = true;
        Result.RouteProgress01 = FMath::Clamp(Sources.RouteProgress01.GetValue(), 0.0f, 1.0f);
    }
    Result.bRadioAvailable = Sources.bRadioAvailable;
    Result.bMirrorsAvailable = Sources.bMirrorsAvailable;
    return Result;
}

void FPinkCabCockpitServiceBridge::ApplyToPresentation(
    const FPinkCabCockpitServicePresentation& Services,
    FPinkCabCockpitPresentationState& InOutPresentation)
{
    InOutPresentation.bMeterAvailable = Services.bMeterAvailable;
    InOutPresentation.bMeterRunning = Services.bMeterRunning;
    InOutPresentation.FareMinor = Services.FareMinor;
    InOutPresentation.MeterDistanceKm = Services.DistanceKm;
    InOutPresentation.MeterFareSeconds = Services.FareSeconds;
    InOutPresentation.bPassengerDoorAvailable = Services.bDoorAvailable;
    InOutPresentation.bPassengerDoorOpen = Services.bPassengerDoorOpen;
    InOutPresentation.bRouteAvailable = Services.bRouteAvailable;
    InOutPresentation.RouteProgress01 = Services.RouteProgress01;
    InOutPresentation.bRadioAvailable = Services.bRadioAvailable;
    InOutPresentation.bMirrorsAvailable = Services.bMirrorsAvailable;
}
