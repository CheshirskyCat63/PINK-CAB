#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Taxi/PinkCabTaximeter.h"
#include "Taxi/PinkCabFarePassengerManifest.h"
#include "Vehicle/PinkCabVehicleLoadState.h"
#include "Vehicle/PinkCabTatraProfile.h"

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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabFarePassengerManifestSeatsTest,
    "PinkCab.Taxi.FareLoop.Passengers.SeatsAndMass",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabFarePassengerManifestSeatsTest::RunTest(const FString& Parameters)
{
    const TArray<FPinkCabFarePassengerInput> Inputs = {
        {FPinkCabStableId(TEXT("pax-a")), 61.0f},
        {FPinkCabStableId(TEXT("pax-b")), 72.5f},
        {FPinkCabStableId(TEXT("pax-c")), 84.0f}};
    FPinkCabFarePassengerManifest Manifest;
    TestTrue(TEXT("three-passenger manifest builds"),
        FPinkCabFarePassengerManifest::TryCreate(FPinkCabStableId(TEXT("fare-passengers")), Inputs, Manifest));
    TestEqual(TEXT("three passengers retained"), Manifest.GetRecords().Num(), 3);
    TestEqual(TEXT("first seat is rear1"), Manifest.GetRecords()[0].Seat, EPinkCabPassengerSeat::Rear1);
    TestEqual(TEXT("third seat is rear3"), Manifest.GetRecords()[2].Seat, EPinkCabPassengerSeat::Rear3);
    TestEqual(TEXT("physical mass is preserved exactly"), Manifest.GetTotalPassengerMassKg(), 217.5f);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabFarePassengerManifestBoardExitTest,
    "PinkCab.Taxi.FareLoop.Passengers.BoardExitExactlyOnce",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabFarePassengerManifestBoardExitTest::RunTest(const FString& Parameters)
{
    const TArray<FPinkCabFarePassengerInput> Inputs = {
        {FPinkCabStableId(TEXT("pax-1")), 60.0f},
        {FPinkCabStableId(TEXT("pax-2")), 70.0f}};
    FPinkCabFarePassengerManifest Manifest;
    TestTrue(TEXT("manifest builds"),
        FPinkCabFarePassengerManifest::TryCreate(FPinkCabStableId(TEXT("fare-load")), Inputs, Manifest));

    FPinkCabVehicleLoadState Load;
    const FPinkCabTatraProfile Profile = FPinkCabTatraProfile::Canonical();
    const float BeforeKg = Load.GetTotalMassKg(Profile);
    TestFalse(TEXT("moving vehicle cannot board"), Manifest.TryBoard(false, true, Load));
    TestFalse(TEXT("closed door cannot board"), Manifest.TryBoard(true, false, Load));
    TestTrue(TEXT("full stop plus open door boards once"), Manifest.TryBoard(true, true, Load));
    TestEqual(TEXT("board adds exact passenger mass"), Load.GetTotalMassKg(Profile), BeforeKg + 130.0f);
    TestFalse(TEXT("duplicate board rejected"), Manifest.TryBoard(true, true, Load));
    TestEqual(TEXT("duplicate board cannot double mass"), Load.GetTotalMassKg(Profile), BeforeKg + 130.0f);

    TestFalse(TEXT("moving vehicle cannot exit"), Manifest.TryExit(false, true, Load));
    TestFalse(TEXT("closed door cannot exit"), Manifest.TryExit(true, false, Load));
    TestTrue(TEXT("full stop plus open door exits once"), Manifest.TryExit(true, true, Load));
    TestEqual(TEXT("exit removes fare group mass"), Load.GetTotalMassKg(Profile), BeforeKg);
    TestFalse(TEXT("duplicate exit rejected"), Manifest.TryExit(true, true, Load));
    return true;
}

#endif
