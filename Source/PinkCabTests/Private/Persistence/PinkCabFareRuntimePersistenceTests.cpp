#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Persistence/PinkCabFareRuntimeSnapshot.h"
#include "Taxi/PinkCabFareLoopCoordinator.h"

namespace PinkCabFareRuntimePersistenceTests
{
FPinkCabOrder MakeOrder(EPinkCabFareMode Mode, TArray<FPinkCabStableId> PassengerIds)
{
    FPinkCabOrder Order;
    Order.OrderId = FPinkCabStableId(TEXT("order:persistence:fare"));
    Order.PickupId = FPinkCabStableId(TEXT("pickup:persistence:fare"));
    Order.DestinationId = FPinkCabStableId(TEXT("destination:persistence:fare"));
    Order.PassengerIdentityIds = MoveTemp(PassengerIds);
    Order.FareMode = Mode;
    return Order;
}

FPinkCabFarePassengerManifest MakeManifest(
    const FPinkCabStableId& FareId,
    TConstArrayView<FPinkCabFarePassengerInput> Inputs)
{
    FPinkCabFarePassengerManifest Manifest;
    FPinkCabPassengerLoadLayout Layout;
    Layout.RearLongitudinalCm = -35.0f;
    Layout.FrontLongitudinalCm = 45.0f;
    FPinkCabFarePassengerManifest::TryCreate(FareId, Inputs, Manifest, Layout);
    return Manifest;
}
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabFareRuntimeActiveMeteredTest,
    "PinkCab.Persistence.FareRuntimeSnapshot.ActiveMetered",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabFareRuntimeActiveMeteredTest::RunTest(const FString& Parameters)
{
    using namespace PinkCabFareRuntimePersistenceTests;
    const FPinkCabStableId FareId(TEXT("fare:persistence:active"));
    const TArray<FPinkCabStableId> PassengerIds = {
        FPinkCabStableId(TEXT("passenger:persistence:1")),
        FPinkCabStableId(TEXT("passenger:persistence:2"))};
    const FPinkCabOrder Order = MakeOrder(EPinkCabFareMode::Metered, PassengerIds);
    const TArray<FPinkCabFarePassengerInput> Inputs = {
        {PassengerIds[0], 65.0f}, {PassengerIds[1], 80.0f}};
    const FPinkCabFarePassengerManifest Manifest = MakeManifest(FareId, Inputs);

    FPinkCabFareLoopCoordinator Coordinator;
    TestTrue(TEXT("coordinator created"), FPinkCabFareLoopCoordinator::TryCreate(
        FareId, Order, FPinkCabFarePricingTerms::Metered(100, 50, 20), Manifest, Coordinator));
    FPinkCabVehicleLoadState Load;
    TestTrue(TEXT("pickup accepted"), Coordinator.TryAcceptPickup(true, true));
    TestTrue(TEXT("passengers board"), Coordinator.TryBoard(true, true, Load));
    TestTrue(TEXT("ride starts"), Coordinator.TryStartRide(true));
    Coordinator.TickFare(2.0, 60.0, false);
    TestEqual(TEXT("pre-snapshot fare"), Coordinator.GetFareMinor(), int64(220));

    FPinkCabFareRuntimeSnapshot Snapshot;
    TestTrue(TEXT("capture succeeds"),
        FPinkCabFareRuntimeSnapshotCodec::Capture(Coordinator, Snapshot));
    TestEqual(TEXT("manifest count captured"), Snapshot.Manifest.Records.Num(), 2);
    TestEqual(TEXT("first passenger id captured"),
        Snapshot.Manifest.Records[0].PassengerId, PassengerIds[0].Serialize());
    TestEqual(TEXT("second passenger mass captured"), Snapshot.Manifest.Records[1].MassKg, 80.0f);

    FPinkCabFareLoopCoordinator Restored;
    TestTrue(TEXT("restore succeeds"),
        FPinkCabFareRuntimeSnapshotCodec::Restore(Snapshot, Restored));
    TestEqual(TEXT("active state restored"), Restored.GetState(), EPinkCabFareLoopState::Active);
    TestEqual(TEXT("fare restored"), Restored.GetFareMinor(), int64(220));
    Restored.TickFare(1.0, 30.0, false);
    TestEqual(TEXT("restored meter continues"), Restored.GetFareMinor(), int64(280));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabFareRuntimeAwaitingPaymentTest,
    "PinkCab.Persistence.FareRuntimeSnapshot.AwaitingPaymentOffMeter",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPinkCabFareRuntimeAwaitingPaymentTest::RunTest(const FString& Parameters)
{
    using namespace PinkCabFareRuntimePersistenceTests;
    const FPinkCabStableId FareId(TEXT("fare:persistence:awaiting"));
    const FPinkCabStableId PassengerId(TEXT("passenger:persistence:awaiting"));
    const TArray<FPinkCabStableId> PassengerIds = {PassengerId};
    const FPinkCabOrder Order = MakeOrder(EPinkCabFareMode::OffMeter, PassengerIds);
    const TArray<FPinkCabFarePassengerInput> Inputs = {{PassengerId, 72.0f}};
    const FPinkCabFarePassengerManifest Manifest = MakeManifest(FareId, Inputs);

    FPinkCabFareLoopCoordinator Coordinator;
    TestTrue(TEXT("coordinator created"), FPinkCabFareLoopCoordinator::TryCreate(
        FareId, Order, FPinkCabFarePricingTerms::OffMeter(777), Manifest, Coordinator));
    FPinkCabVehicleLoadState Load;
    TestTrue(TEXT("pickup"), Coordinator.TryAcceptPickup(true, true));
    TestTrue(TEXT("board"), Coordinator.TryBoard(true, true, Load));
    TestTrue(TEXT("start"), Coordinator.TryStartRide(true));
    Coordinator.TickFare(9.0, 900.0, false);
    TestTrue(TEXT("arrive"), Coordinator.TryArrive(true, true));
    TestEqual(TEXT("off-meter amount fixed"), Coordinator.GetFareMinor(), int64(777));

    FPinkCabFareRuntimeSnapshot Snapshot;
    TestTrue(TEXT("capture succeeds"),
        FPinkCabFareRuntimeSnapshotCodec::Capture(Coordinator, Snapshot));
    TestFalse(TEXT("meter stopped at payment boundary"), Snapshot.Taximeter.bRunning);
    TestEqual(TEXT("session awaiting payment"),
        Snapshot.Session.State, EPinkCabFareState::AwaitingPayment);

    FPinkCabFareLoopCoordinator Restored;
    TestTrue(TEXT("restore succeeds"),
        FPinkCabFareRuntimeSnapshotCodec::Restore(Snapshot, Restored));
    TestEqual(TEXT("awaiting state restored"),
        Restored.GetState(), EPinkCabFareLoopState::AwaitingPayment);
    FPinkCabEconomyLedger Ledger(0, 0);
    FPinkCabFareSettlementService Settlement;
    TestTrue(TEXT("ordinary payment continues after restore"),
        Restored.TryPay(0, Ledger, Settlement));
    TestEqual(TEXT("restored fare credits once"), Ledger.GetBalanceMinor(), int64(777));
    TestEqual(TEXT("paid state reached"), Restored.GetState(), EPinkCabFareLoopState::Paid);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabFareRuntimeAtomicRejectTest,
    "PinkCab.Persistence.FareRuntimeSnapshot.AtomicInvalidCoherence",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabFareRuntimeAtomicRejectTest::RunTest(const FString& Parameters)
{
    using namespace PinkCabFareRuntimePersistenceTests;
    const FPinkCabStableId FareId(TEXT("fare:persistence:invalid"));
    const FPinkCabStableId PassengerId(TEXT("passenger:persistence:invalid"));
    const TArray<FPinkCabStableId> PassengerIds = {PassengerId};
    const FPinkCabOrder Order = MakeOrder(EPinkCabFareMode::Metered, PassengerIds);
    const TArray<FPinkCabFarePassengerInput> Inputs = {{PassengerId, 70.0f}};
    const FPinkCabFarePassengerManifest Manifest = MakeManifest(FareId, Inputs);

    FPinkCabFareLoopCoordinator Source;
    TestTrue(TEXT("source created"), FPinkCabFareLoopCoordinator::TryCreate(
        FareId, Order, FPinkCabFarePricingTerms::Metered(100, 10, 5), Manifest, Source));
    FPinkCabVehicleLoadState SourceLoad;
    Source.TryAcceptPickup(true, true);
    Source.TryBoard(true, true, SourceLoad);
    Source.TryStartRide(true);

    FPinkCabFareRuntimeSnapshot Snapshot;
    TestTrue(TEXT("capture valid active source"),
        FPinkCabFareRuntimeSnapshotCodec::Capture(Source, Snapshot));
    Snapshot.Session.State = EPinkCabFareState::Paid;

    const FPinkCabStableId SentinelFareId(TEXT("fare:persistence:sentinel"));
    const FPinkCabOrder SentinelOrder = MakeOrder(EPinkCabFareMode::OffMeter, PassengerIds);
    const FPinkCabFarePassengerManifest SentinelManifest = MakeManifest(SentinelFareId, Inputs);
    FPinkCabFareLoopCoordinator Target;
    TestTrue(TEXT("target created"), FPinkCabFareLoopCoordinator::TryCreate(
        SentinelFareId, SentinelOrder, FPinkCabFarePricingTerms::OffMeter(333),
        SentinelManifest, Target));
    const EPinkCabFareLoopState BeforeState = Target.GetState();
    const int64 BeforeFare = Target.GetFareMinor();

    TestFalse(TEXT("incoherent state rejected"),
        FPinkCabFareRuntimeSnapshotCodec::Restore(Snapshot, Target));
    TestEqual(TEXT("target state remains atomic"), Target.GetState(), BeforeState);
    TestEqual(TEXT("target fare remains atomic"), Target.GetFareMinor(), BeforeFare);
    return true;
}
#endif
