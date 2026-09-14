#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Cockpit/PinkCabCockpitServiceBridge.h"
#include "Cockpit/PinkCabCockpitPresentationState.h"
#include "Taxi/PinkCabTaximeter.h"
#include "Vehicle/PinkCabCockpitState.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabCockpitServiceBridgeMeterTest,
    "PinkCab.Cockpit.ServiceBridge.MeterDoorRoute",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabCockpitServiceBridgeMeterTest::RunTest(const FString& Parameters)
{
    FPinkCabTaximeter Meter(FPinkCabFarePricingTerms::Metered(100, 200, 60));
    TestTrue(TEXT("meter starts"), Meter.Start());
    Meter.Tick(1.0, 60.0, false);

    FPinkCabCockpitState Cockpit;
    Cockpit.SetPassengerDoorOpen(true);
    FPinkCabCockpitServiceSources Sources;
    Sources.Taximeter = &Meter;
    Sources.CockpitState = &Cockpit;
    Sources.RouteProgress01 = 0.25f;
    Sources.bRadioAvailable = true;
    Sources.bMirrorsAvailable = true;
    const FPinkCabCockpitServicePresentation View = FPinkCabCockpitServiceBridge::Read(Sources);
    TestTrue(TEXT("meter is available"), View.bMeterAvailable);
    TestTrue(TEXT("meter running state is authoritative"), View.bMeterRunning);
    TestEqual(TEXT("fare minor follows taximeter"), View.FareMinor, int64(360));
    TestEqual(TEXT("meter distance follows taximeter"), View.DistanceKm, 1.0);
    TestEqual(TEXT("meter time follows taximeter"), View.FareSeconds, 60.0);
    TestTrue(TEXT("door state is available"), View.bDoorAvailable);
    TestTrue(TEXT("door-open state follows cockpit"), View.bPassengerDoorOpen);
    TestTrue(TEXT("route surface is available"), View.bRouteAvailable);
    TestEqual(TEXT("route progress is preserved"), View.RouteProgress01, 0.25f);
    TestTrue(TEXT("radio capability is forwarded"), View.bRadioAvailable);
    TestTrue(TEXT("mirror capability is forwarded"), View.bMirrorsAvailable);

    FPinkCabCockpitPresentationState Presentation;
    FPinkCabCockpitServiceBridge::ApplyToPresentation(View, Presentation);
    TestTrue(TEXT("meter surface consumes bridge availability"), Presentation.bMeterAvailable);
    TestEqual(TEXT("meter fare is copied to presentation"), Presentation.FareMinor, int64(360));
    TestTrue(TEXT("door surface consumes bridge availability"), Presentation.bPassengerDoorAvailable);
    TestTrue(TEXT("route surface consumes bridge availability"), Presentation.bRouteAvailable);
    TestTrue(TEXT("radio surface consumes bridge availability"), Presentation.bRadioAvailable);
    TestTrue(TEXT("mirror surfaces consume bridge availability"), Presentation.bMirrorsAvailable);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabCockpitServiceBridgeFailClosedTest,
    "PinkCab.Cockpit.ServiceBridge.FailClosed",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPinkCabCockpitServiceBridgeFailClosedTest::RunTest(const FString& Parameters)
{
    const FPinkCabCockpitServicePresentation View =
        FPinkCabCockpitServiceBridge::Read(FPinkCabCockpitServiceSources{});
    TestFalse(TEXT("missing meter source fails closed"), View.bMeterAvailable);
    TestFalse(TEXT("missing door source fails closed"), View.bDoorAvailable);
    TestFalse(TEXT("missing route source fails closed"), View.bRouteAvailable);
    TestFalse(TEXT("missing radio source fails closed"), View.bRadioAvailable);
    TestFalse(TEXT("missing mirror source fails closed"), View.bMirrorsAvailable);
    TestEqual(TEXT("missing fare does not invent value"), View.FareMinor, int64(0));
    TestEqual(TEXT("missing route does not invent progress"), View.RouteProgress01, 0.0f);
    return true;
}

#endif
