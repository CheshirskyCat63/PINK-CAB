#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Taxi/PinkCabTaximeter.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabTaximeterMeteredTest,
    "PinkCab.Taxi.FareLoop.Taximeter.MeteredDistanceAndTime",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabTaximeterMeteredTest::RunTest(const FString& Parameters)
{
    FPinkCabTaximeter Meter(FPinkCabFarePricingTerms::Metered(100, 200, 60));
    TestTrue(TEXT("meter starts once"), Meter.Start());
    TestFalse(TEXT("running meter does not restart"), Meter.Start());

    Meter.Tick(1.5, 30.0, false);
    TestEqual(TEXT("fare is base plus distance plus time"), Meter.GetFareMinor(), int64(430));

    Meter.Tick(0.0, 60.0, true);
    TestEqual(TEXT("hard pause excludes fare time"), Meter.GetFareMinor(), int64(430));

    Meter.Tick(-5.0, -30.0, false);
    TestEqual(TEXT("negative deltas are ignored"), Meter.GetFareMinor(), int64(430));
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabTaximeterOffMeterAndStopTest,
    "PinkCab.Taxi.FareLoop.Taximeter.OffMeterAndStop",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabTaximeterOffMeterAndStopTest::RunTest(const FString& Parameters)
{
    FPinkCabTaximeter Meter(FPinkCabFarePricingTerms::OffMeter(775));
    TestTrue(TEXT("off-meter session starts"), Meter.Start());
    Meter.Tick(12.0, 900.0, false);
    TestEqual(TEXT("off-meter keeps explicit agreed amount"), Meter.GetFareMinor(), int64(775));

    TestTrue(TEXT("meter stops once"), Meter.Stop());
    TestFalse(TEXT("stopped meter does not stop twice"), Meter.Stop());
    Meter.Tick(4.0, 300.0, false);
    TestEqual(TEXT("stopped meter is frozen"), Meter.GetFareMinor(), int64(775));
    return true;
}

#endif
