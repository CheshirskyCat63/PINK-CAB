#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Taxi/PinkCabFareLoopCoordinator.h"
#include "Taxi/PinkCabFarePassengerManifest.h"
#include "Taxi/PinkCabOrder.h"
#include "Vehicle/PinkCabVehicleLoadState.h"
#include "Economy/PinkCabEconomyLedger.h"
#include "Economy/PinkCabFareSettlementService.h"

namespace PinkCabFareLoopTest
{
    static FPinkCabOrder MakeOrder(const FString& Suffix, EPinkCabFareMode Mode, int32 PassengerCount)
    {
        FPinkCabOrder Order;
        Order.OrderId = FPinkCabStableId(TEXT("order-") + Suffix);
        Order.PickupId = FPinkCabStableId(TEXT("pickup-") + Suffix);
        Order.DestinationId = FPinkCabStableId(TEXT("drop-") + Suffix);
        Order.FareMode = Mode;
        for (int32 Index = 0; Index < PassengerCount; ++Index)
        {
            Order.PassengerIdentityIds.Add(FPinkCabStableId(FString::Printf(TEXT("pax-%s-%d"), *Suffix, Index)));
        }
        return Order;
    }
    static bool MakeManifest(const FPinkCabStableId& FareId, const FPinkCabOrder& Order, FPinkCabFarePassengerManifest& OutManifest)
    {
        TArray<FPinkCabFarePassengerInput, TInlineAllocator<5>> Inputs;
        Inputs.Reserve(Order.PassengerIdentityIds.Num());
        for (int32 Index = 0; Index < Order.PassengerIdentityIds.Num(); ++Index)
        {
            Inputs.Add({Order.PassengerIdentityIds[Index], 60.0f + 5.0f * Index});
        }
        return FPinkCabFarePassengerManifest::TryCreate(FareId, Inputs, OutManifest);
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabFareLoopPaidMeteredTest,
    "PinkCab.Taxi.FareLoop.Coordinator.PaidMetered",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabFareLoopPaidMeteredTest::RunTest(const FString& Parameters)
{
    const FPinkCabStableId FareId(TEXT("fare-paid"));
    const FPinkCabOrder Order = PinkCabFareLoopTest::MakeOrder(TEXT("paid"), EPinkCabFareMode::Metered, 2);
    FPinkCabFarePassengerManifest Manifest;
    TestTrue(TEXT("manifest builds"), PinkCabFareLoopTest::MakeManifest(FareId, Order, Manifest));
    FPinkCabFareLoopCoordinator Loop;
    TestTrue(TEXT("coordinator builds"), FPinkCabFareLoopCoordinator::TryCreate(
        FareId, Order, FPinkCabFarePricingTerms::Metered(100, 200, 60), Manifest, Loop));
    FPinkCabVehicleLoadState Load;
    FPinkCabEconomyLedger Ledger(1000, 0);
    FPinkCabFareSettlementService Settlement;

    TestFalse(TEXT("pickup acceptance requires deliberate full stop"), Loop.TryAcceptPickup(true, false));
    TestTrue(TEXT("pickup accepted at full stop"), Loop.TryAcceptPickup(true, true));
    TestFalse(TEXT("closed door blocks boarding"), Loop.TryBoard(true, false, Load));
    TestTrue(TEXT("open door and full stop boards"), Loop.TryBoard(true, true, Load));
    TestTrue(TEXT("latched door starts ride"), Loop.TryStartRide(true));
    Loop.TickFare(1.5, 30.0, false);
    TestEqual(TEXT("metered fare accrues"), Loop.GetFareMinor(), int64(430));
    TestTrue(TEXT("eligible full-stop destination ends ride"), Loop.TryArrive(true, true));
    TestFalse(TEXT("passengers cannot exit unpaid paid-path"), Loop.TryExit(true, true, Load));
    TestTrue(TEXT("payment settles fare and tip"), Loop.TryPay(250, Ledger, Settlement));
    TestEqual(TEXT("fare plus tip credited exactly once"), Ledger.GetBalanceMinor(), int64(1680));
    TestTrue(TEXT("paid passengers exit at full stop with door open"), Loop.TryExit(true, true, Load));
    TestTrue(TEXT("completed fare releases slot for next fare"), Loop.CanAcceptNextFare());
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabFareLoopDeclineTest,
    "PinkCab.Taxi.FareLoop.Coordinator.Decline",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabFareLoopDeclineTest::RunTest(const FString& Parameters)
{
    const FPinkCabStableId FareId(TEXT("fare-decline"));
    const FPinkCabOrder Order = PinkCabFareLoopTest::MakeOrder(TEXT("decline"), EPinkCabFareMode::Metered, 1);
    FPinkCabFarePassengerManifest Manifest;
    TestTrue(TEXT("manifest builds"), PinkCabFareLoopTest::MakeManifest(FareId, Order, Manifest));
    FPinkCabFareLoopCoordinator Loop;
    TestTrue(TEXT("coordinator builds"), FPinkCabFareLoopCoordinator::TryCreate(
        FareId, Order, FPinkCabFarePricingTerms::Metered(0, 100, 0), Manifest, Loop));

    TestFalse(TEXT("moving decline rejected"), Loop.TryDecline(false));
    TestTrue(TEXT("full-stop decline accepted"), Loop.TryDecline(true));
    TestEqual(TEXT("declined state retained"), Loop.GetState(), EPinkCabFareLoopState::Declined);
    TestTrue(TEXT("decline immediately allows next fare"), Loop.CanAcceptNextFare());
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabFareLoopOffMeterEvasionTest,
    "PinkCab.Taxi.FareLoop.Coordinator.OffMeterEvasion",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabFareLoopOffMeterEvasionTest::RunTest(const FString& Parameters)
{
    const FPinkCabStableId FareId(TEXT("fare-evade-loop"));
    const FPinkCabOrder Order = PinkCabFareLoopTest::MakeOrder(TEXT("evade"), EPinkCabFareMode::OffMeter, 3);
    FPinkCabFarePassengerManifest Manifest;
    TestTrue(TEXT("manifest builds"), PinkCabFareLoopTest::MakeManifest(FareId, Order, Manifest));
    FPinkCabFareLoopCoordinator Loop;
    TestTrue(TEXT("coordinator builds"), FPinkCabFareLoopCoordinator::TryCreate(
        FareId, Order, FPinkCabFarePricingTerms::OffMeter(775), Manifest, Loop));
    FPinkCabVehicleLoadState Load;
    FPinkCabFareSettlementService Settlement;
    FPinkCabEconomyLedger Ledger(900, 0);

    TestTrue(TEXT("pickup accepted"), Loop.TryAcceptPickup(true, true));
    TestTrue(TEXT("group boards"), Loop.TryBoard(true, true, Load));
    TestTrue(TEXT("ride starts"), Loop.TryStartRide(true));
    Loop.TickFare(20.0, 1200.0, false);
    TestEqual(TEXT("off-meter amount stays agreed"), Loop.GetFareMinor(), int64(775));
    TestTrue(TEXT("destination reached"), Loop.TryArrive(true, true));
    TestFalse(TEXT("closed exit path cannot evade"), Loop.TryEvade(false, Settlement));
    TestTrue(TEXT("eligible open exit path records evasion"), Loop.TryEvade(true, Settlement));
    TestEqual(TEXT("evasion credits no money"), Ledger.GetBalanceMinor(), int64(900));
    TestTrue(TEXT("evaded passengers may exit"), Loop.TryExit(true, true, Load));
    TestTrue(TEXT("evaded completed fare releases next fare"), Loop.CanAcceptNextFare());
    return true;
}

#endif
