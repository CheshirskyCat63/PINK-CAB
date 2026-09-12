#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Taxi/PinkCabFareSession.h"
#include "Taxi/PinkCabOrder.h"
#include "Taxi/PinkCabPassengerTemplate.h"
#include "Taxi/PinkCabPassengerIdentity.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabPassengerGroupSeatsTest,
    "PinkCab.Taxi.Contracts.PassengerGroupSeats",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabPassengerGroupSeatsTest::RunTest(const FString& Parameters)
{
    TArray<EPinkCabPassengerSeat> Seats;
    TestTrue(TEXT("group of five is valid"), FPinkCabPassengerGroup::TryAssignSeats(5, Seats));
    TestEqual(TEXT("five seats assigned"), Seats.Num(), 5);
    TestEqual(TEXT("seat 1 rear"), Seats[0], EPinkCabPassengerSeat::Rear1);
    TestEqual(TEXT("seat 3 rear"), Seats[2], EPinkCabPassengerSeat::Rear3);
    TestEqual(TEXT("seat 4 front"), Seats[3], EPinkCabPassengerSeat::Front1);
    TestEqual(TEXT("seat 5 front"), Seats[4], EPinkCabPassengerSeat::Front2);
    TestFalse(TEXT("zero passengers rejected"), FPinkCabPassengerGroup::TryAssignSeats(0, Seats));
    TestFalse(TEXT("six passengers rejected"), FPinkCabPassengerGroup::TryAssignSeats(6, Seats));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabPassengerIdentityTest,
    "PinkCab.Taxi.Contracts.PassengerIdentity",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabPassengerIdentityTest::RunTest(const FString& Parameters)
{
    const FPinkCabPassengerTemplate Template(FName(TEXT("Commuter")), 72.0f);
    FPinkCabPassengerIdentity Identity(FPinkCabStableId(TEXT("pax-001")), Template.TemplateId);
    TestEqual(TEXT("template remains archetype"), Identity.TemplateId, Template.TemplateId);
    TestTrue(TEXT("identity persists independently"), Identity.IdentityId.IsValid());
    TestFalse(TEXT("not repeat after one paid fare"), Identity.RegisterPaidFare());
    TestTrue(TEXT("repeat after two paid fares"), Identity.RegisterPaidFare());
    TestTrue(TEXT("repeat eligibility persists"), Identity.IsRepeatEligible());

    FPinkCabPassengerIdentity Authored(FPinkCabStableId(TEXT("pax-002")), Template.TemplateId);
    Authored.RegisterAuthoredRelationshipEvent();
    TestTrue(TEXT("authored event promotes repeat eligibility"), Authored.IsRepeatEligible());
    TestEqual(TEXT("trust axis exists"), Authored.Relationship.Trust, 0.0f);
    TestEqual(TEXT("satisfaction axis exists"), Authored.Relationship.Satisfaction, 0.0f);
    TestEqual(TEXT("risk axis exists"), Authored.Relationship.RiskTolerance, 0.0f);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabFareManualStartStopTest,
    "PinkCab.Taxi.Contracts.ManualStartStop",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabFareManualStartStopTest::RunTest(const FString& Parameters)
{
    FPinkCabFareSession Fare(FPinkCabStableId(TEXT("fare-001")));
    TestEqual(TEXT("new fare waits boarding"), Fare.GetState(), EPinkCabFareState::Boarding);
    TestEqual(TEXT("start blocked before seated"), Fare.TryStartMeter(false, true), EPinkCabFareTransitionResult::Blocked);
    TestEqual(TEXT("start blocked with open door"), Fare.TryStartMeter(true, false), EPinkCabFareTransitionResult::Blocked);
    TestEqual(TEXT("manual start succeeds only when ready"), Fare.TryStartMeter(true, true), EPinkCabFareTransitionResult::Applied);
    TestEqual(TEXT("fare is active"), Fare.GetState(), EPinkCabFareState::Active);

    TestEqual(TEXT("stop blocked before destination/full stop"), Fare.TryStopMeter(false, true), EPinkCabFareTransitionResult::Blocked);
    TestEqual(TEXT("manual stop requires full stop"), Fare.TryStopMeter(true, false), EPinkCabFareTransitionResult::Blocked);
    TestEqual(TEXT("manual stop enters awaiting payment"), Fare.TryStopMeter(true, true), EPinkCabFareTransitionResult::Applied);
    TestEqual(TEXT("awaiting payment"), Fare.GetState(), EPinkCabFareState::AwaitingPayment);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabFarePaymentReceiptCycleTest,
    "PinkCab.Taxi.Contracts.PaymentReceiptCycle",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabFarePaymentReceiptCycleTest::RunTest(const FString& Parameters)
{
    FPinkCabFareSession Fare(FPinkCabStableId(TEXT("fare-002")));
    TestEqual(TEXT("start"), Fare.TryStartMeter(true, true), EPinkCabFareTransitionResult::Applied);
    TestEqual(TEXT("stop"), Fare.TryStopMeter(true, true), EPinkCabFareTransitionResult::Applied);
    TestEqual(TEXT("payment swipe commits"), Fare.CommitPaymentSwipe(), EPinkCabFareTransitionResult::Applied);
    TestEqual(TEXT("paid"), Fare.GetState(), EPinkCabFareState::Paid);
    TestEqual(TEXT("receipt may be left"), Fare.ResolveReceipt(false), EPinkCabFareTransitionResult::Applied);
    TestEqual(TEXT("passengers may exit"), Fare.MarkPassengersExited(), EPinkCabFareTransitionResult::Applied);
    TestEqual(TEXT("ready to reset"), Fare.GetState(), EPinkCabFareState::ReadyToReset);
    TestEqual(TEXT("reset completes fare"), Fare.ResetToIdle(), EPinkCabFareTransitionResult::Applied);
    TestEqual(TEXT("idle"), Fare.GetState(), EPinkCabFareState::Idle);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabFareEvasionTest,
    "PinkCab.Taxi.Contracts.FareEvasion",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabFareEvasionTest::RunTest(const FString& Parameters)
{
    FPinkCabFareSession Fare(FPinkCabStableId(TEXT("fare-003")));
    Fare.TryStartMeter(true, true);
    Fare.TryStopMeter(true, true);
    TestEqual(TEXT("open-door escape before payment is evasion"), Fare.TryUnpaidEscape(true), EPinkCabFareTransitionResult::Applied);
    TestEqual(TEXT("fare evaded"), Fare.GetState(), EPinkCabFareState::Evaded);

    FPinkCabFareSession Paid(FPinkCabStableId(TEXT("fare-004")));
    Paid.TryStartMeter(true, true);
    Paid.TryStopMeter(true, true);
    Paid.CommitPaymentSwipe();
    TestEqual(TEXT("payment commit blocks unpaid escape"), Paid.TryUnpaidEscape(true), EPinkCabFareTransitionResult::Blocked);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabFareStopoverAndFailureTest,
    "PinkCab.Taxi.Contracts.StopoverAndFailure",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabFareStopoverAndFailureTest::RunTest(const FString& Parameters)
{
    FPinkCabFareSession Fare(FPinkCabStableId(TEXT("fare-005")));
    Fare.TryStartMeter(true, true);
    TestEqual(TEXT("active-meter stopover allowed"), Fare.BeginPassengerStopover(), EPinkCabFareTransitionResult::Applied);
    TestEqual(TEXT("stopover keeps fare active"), Fare.GetState(), EPinkCabFareState::Active);
    TestTrue(TEXT("fare time keeps accruing during ordinary wait"), Fare.ShouldAccrueFareTime(false));
    TestFalse(TEXT("hard pause excludes fare time"), Fare.ShouldAccrueFareTime(true));
    TestEqual(TEXT("failed return becomes unpaid escape"), Fare.MarkStopoverNoReturn(), EPinkCabFareTransitionResult::Applied);
    TestEqual(TEXT("stopover escape state"), Fare.GetState(), EPinkCabFareState::Evaded);

    FPinkCabFareSession Crash(FPinkCabStableId(TEXT("fare-006")));
    Crash.TryStartMeter(true, true);
    TestEqual(TEXT("serious crash fails active fare"), Crash.FailForSeriousCrash(), EPinkCabFareTransitionResult::Applied);
    TestTrue(TEXT("failure requires workday end"), Crash.RequiresWorkdayEnd());
    TestTrue(TEXT("failure requires repair recovery"), Crash.RequiresRepairRecovery());
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabFareSlotAndPolicyBoundaryTest,
    "PinkCab.Taxi.Contracts.ActiveFareAndPolicyBoundary",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabFareSlotAndPolicyBoundaryTest::RunTest(const FString& Parameters)
{
    FPinkCabActiveFareSlot Slot;
    TestTrue(TEXT("first fare activates"), Slot.TryActivate(FPinkCabStableId(TEXT("fare-a"))));
    TestFalse(TEXT("second fare cannot activate"), Slot.TryActivate(FPinkCabStableId(TEXT("fare-b"))));
    Slot.Clear();
    TestTrue(TEXT("next fare may activate after clear"), Slot.TryActivate(FPinkCabStableId(TEXT("fare-b"))));

    TestTrue(TEXT("full stop at 0.49 kmh after 0.4 sec"), FPinkCabFullStopGate::IsSatisfied(0.49f, 0.4f));
    TestFalse(TEXT("0.5 kmh is not full stop"), FPinkCabFullStopGate::IsSatisfied(0.5f, 0.4f));
    TestFalse(TEXT("dwell below 0.4 sec rejected"), FPinkCabFullStopGate::IsSatisfied(0.1f, 0.39f));
    TestEqual(TEXT("F18 remains explicit policy boundary"), FPinkCabFarePolicy::ResolveOffMeterEconomy(), EPinkCabFareTransitionResult::PolicyRequired);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabPassengerProfileReviewTest,
    "PinkCab.Taxi.Contracts.PassengerProfileReview",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabPassengerProfileReviewTest::RunTest(const FString& Parameters)
{
    FPinkCabPassengerIdentity Identity(FPinkCabStableId(TEXT("pax-profile")), FName(TEXT("Commuter")));
    TestTrue(TEXT("valid local review accepted"), Identity.AddLocalReview(1, TEXT("Лживая воровка")));
    TestFalse(TEXT("zero-star review rejected"), Identity.AddLocalReview(0, TEXT("invalid")));
    const FPinkCabPassengerPublicProfile Profile = Identity.BuildPublicProfile();
    TestTrue(TEXT("profile exposes stable identity before materialization"), Profile.IdentityId.IsValid());
    TestEqual(TEXT("profile exposes template"), Profile.TemplateId, FName(TEXT("Commuter")));
    TestEqual(TEXT("profile carries one local review"), Profile.LocalReviews.Num(), 1);
    TestEqual(TEXT("review stars preserved"), Profile.LocalReviews[0].Stars, 1);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabOrderContractTest,
    "PinkCab.Taxi.Contracts.Order",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabOrderContractTest::RunTest(const FString& Parameters)
{
    FPinkCabOrder Order;
    Order.OrderId = FPinkCabStableId(TEXT("order-1"));
    Order.PickupId = FPinkCabStableId(TEXT("pickup-1"));
    Order.DestinationId = FPinkCabStableId(TEXT("drop-1"));
    Order.PassengerIdentityIds.Add(FPinkCabStableId(TEXT("pax-1")));
    Order.PassengerIdentityIds.Add(FPinkCabStableId(TEXT("pax-2")));
    TestTrue(TEXT("logical order is valid without passenger Actors"), Order.IsValid());
    return true;
}
#endif
