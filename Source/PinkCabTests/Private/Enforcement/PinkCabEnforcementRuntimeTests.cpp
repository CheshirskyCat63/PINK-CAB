#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Enforcement/PinkCabRoadRuleProfile.h"
#include "Enforcement/PinkCabEnforcementService.h"
#include "Enforcement/PinkCabEnforcementLedger.h"
#include "Economy/PinkCabEconomyLedger.h"

namespace
{
FPinkCabRoadRuleDefinition Rule(
    EPinkCabRoadRuleKind Kind,
    const TCHAR* TypeId,
    int64 Fine,
    int32 Reputation)
{
    FPinkCabRoadRuleDefinition R;
    R.Kind = Kind;
    R.TypeId = FName(TypeId);
    R.FineAmountMinor = Fine;
    R.ReputationDelta = Reputation;
    R.BaseSeverity = 0.5f;
    return R;
}

FPinkCabRoadObservation Observation(double Speed)
{
    FPinkCabRoadObservation O;
    O.ObservationKey = TEXT("route-sample-001");
    O.LaneId = FPinkCabLaneId(TEXT("lane:rules"));
    O.CityLocationId = FPinkCabStableId(TEXT("location:rules"));
    O.VehicleId = FPinkCabStableId(TEXT("vehicle:hero"));
    O.TimestampMs = 424242;
    O.SpeedCmPerSec = Speed;
    return O;
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabRoadRuleProfileLookupTest,
    "PinkCab.Enforcement.Runtime.RuleLookup",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabRoadRuleProfileLookupTest::RunTest(const FString& Parameters)
{
    FPinkCabRoadRuleProfile Profile;
    const FPinkCabLaneId Lane(TEXT("lane:rules"));
    FPinkCabRoadRuleDefinition Speed = Rule(EPinkCabRoadRuleKind::SpeedLimit, TEXT("Speed"), 1000, -3);
    Speed.SpeedLimitCmPerSec = 100.0;
    TestTrue(TEXT("speed rule added"), Profile.TrySetRule(Lane, Speed));
    TestTrue(TEXT("lane rule added"), Profile.TrySetRule(Lane, Rule(EPinkCabRoadRuleKind::LaneRestriction, TEXT("Lane"), 800, -2)));
    TestTrue(TEXT("turn rule added"), Profile.TrySetRule(Lane, Rule(EPinkCabRoadRuleKind::TurnRestriction, TEXT("Turn"), 700, -2)));
    TestTrue(TEXT("parking rule added"), Profile.TrySetRule(Lane, Rule(EPinkCabRoadRuleKind::ParkingRestriction, TEXT("Parking"), 500, -1)));
    TestTrue(TEXT("transit rule added"), Profile.TrySetRule(Lane, Rule(EPinkCabRoadRuleKind::TransitRestriction, TEXT("Transit"), 900, -2)));

    FPinkCabRoadRuleDefinition Found;
    TestTrue(TEXT("speed lookup"), Profile.TryGetRule(Lane, EPinkCabRoadRuleKind::SpeedLimit, Found));
    TestEqual(TEXT("speed threshold retained"), Found.SpeedLimitCmPerSec, 100.0);
    TestTrue(TEXT("lane lookup"), Profile.TryGetRule(Lane, EPinkCabRoadRuleKind::LaneRestriction, Found));
    TestTrue(TEXT("turn lookup"), Profile.TryGetRule(Lane, EPinkCabRoadRuleKind::TurnRestriction, Found));
    TestTrue(TEXT("parking lookup"), Profile.TryGetRule(Lane, EPinkCabRoadRuleKind::ParkingRestriction, Found));
    TestTrue(TEXT("transit lookup"), Profile.TryGetRule(Lane, EPinkCabRoadRuleKind::TransitRestriction, Found));
    TestFalse(TEXT("rules are graph-id scoped"), Profile.TryGetRule(
        FPinkCabLaneId(TEXT("lane:other")), EPinkCabRoadRuleKind::SpeedLimit, Found));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabEnforcementCalmRecklessTest,
    "PinkCab.Enforcement.Runtime.CalmVsReckless",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabEnforcementCalmRecklessTest::RunTest(const FString& Parameters)
{
    FPinkCabRoadRuleProfile Profile;
    const FPinkCabLaneId Lane(TEXT("lane:rules"));
    FPinkCabRoadRuleDefinition Speed = Rule(EPinkCabRoadRuleKind::SpeedLimit, TEXT("Speed"), 1000, -4);
    Speed.SpeedLimitCmPerSec = 100.0;
    TestTrue(TEXT("speed rule added"), Profile.TrySetRule(Lane, Speed));
    TestTrue(TEXT("lane rule added"), Profile.TrySetRule(
        Lane, Rule(EPinkCabRoadRuleKind::LaneRestriction, TEXT("Lane"), 600, -2)));

    TArray<FPinkCabEnforcementEvent> CalmEvents;
    const FPinkCabRoadObservation Calm = Observation(90.0);
    TestTrue(TEXT("calm observation evaluated"), FPinkCabEnforcementService::Evaluate(Profile, Calm, CalmEvents));
    TestEqual(TEXT("calm route has no violations"), CalmEvents.Num(), 0);

    FPinkCabRoadObservation Reckless = Observation(160.0);
    Reckless.bLaneViolation = true;
    TArray<FPinkCabEnforcementEvent> RecklessA;
    TArray<FPinkCabEnforcementEvent> RecklessB;
    TestTrue(TEXT("reckless observation evaluated"), FPinkCabEnforcementService::Evaluate(Profile, Reckless, RecklessA));
    TestTrue(TEXT("same reckless observation repeat evaluated"), FPinkCabEnforcementService::Evaluate(Profile, Reckless, RecklessB));
    TestEqual(TEXT("reckless emits two deterministic violations"), RecklessA.Num(), 2);
    TestEqual(TEXT("repeat count stable"), RecklessB.Num(), RecklessA.Num());
    TestEqual(TEXT("first event id repeatable"), RecklessA[0].EventId.Serialize(), RecklessB[0].EventId.Serialize());
    TestEqual(TEXT("second event id repeatable"), RecklessA[1].EventId.Serialize(), RecklessB[1].EventId.Serialize());
    TestTrue(TEXT("reckless severity positive"), RecklessA[0].Severity > 0.0f);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabEnforcementExactlyOnceSettlementTest,
    "PinkCab.Enforcement.Runtime.ExactlyOnceSettlement",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPinkCabEnforcementExactlyOnceSettlementTest::RunTest(const FString& Parameters)
{
    FPinkCabRoadRuleProfile Profile;
    const FPinkCabLaneId Lane(TEXT("lane:rules"));
    FPinkCabRoadRuleDefinition Speed = Rule(EPinkCabRoadRuleKind::SpeedLimit, TEXT("Speed"), 1000, -4);
    Speed.SpeedLimitCmPerSec = 100.0;
    TestTrue(TEXT("speed rule added"), Profile.TrySetRule(Lane, Speed));

    TArray<FPinkCabEnforcementEvent> Events;
    TestTrue(TEXT("reckless event evaluated"), FPinkCabEnforcementService::Evaluate(Profile, Observation(160.0), Events));
    TestEqual(TEXT("single speed event emitted"), Events.Num(), 1);

    FPinkCabEconomyLedger Economy(5000, 2000);
    FPinkCabEnforcementLedger Reputation(8);
    FPinkCabEnforcementSettlement First;
    TestEqual(TEXT("first settlement commits"),
        FPinkCabEnforcementService::Settle(Profile, Events[0], Economy, Reputation, First),
        EPinkCabEnforcementSettlementResult::Committed);
    TestEqual(TEXT("fine debits economy once"), Economy.GetBalanceMinor(), int64(4000));
    TestEqual(TEXT("reputation changes once"), Reputation.GetReputationScore(), -4);
    TestEqual(TEXT("one event recorded"), Reputation.GetTotalEventCount(), 1);
    TestEqual(TEXT("transaction is typed fine"), First.TransactionType, EPinkCabTransactionType::Fine);
    TestTrue(TEXT("fine transaction id populated"), First.TransactionId.IsValid());

    FPinkCabEnforcementSettlement Replay;
    TestEqual(TEXT("replay reports duplicate"),
        FPinkCabEnforcementService::Settle(Profile, Events[0], Economy, Reputation, Replay),
        EPinkCabEnforcementSettlementResult::Duplicate);
    TestEqual(TEXT("duplicate does not fine twice"), Economy.GetBalanceMinor(), int64(4000));
    TestEqual(TEXT("duplicate does not change reputation twice"), Reputation.GetReputationScore(), -4);
    TestEqual(TEXT("duplicate event not counted twice"), Reputation.GetTotalEventCount(), 1);
    TestEqual(TEXT("fine id derived stably from event"), Replay.TransactionId.GetValue(), First.TransactionId.GetValue());
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabEnforcementLedgerDuplicateGuardTest,
    "PinkCab.Enforcement.Runtime.LedgerDuplicateGuard",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabEnforcementLedgerDuplicateGuardTest::RunTest(const FString& Parameters)
{
    FPinkCabEnforcementEvent Event;
    Event.EventId = FPinkCabStableId(TEXT("event:dupe"));
    Event.TypeId = FName(TEXT("Speed"));
    Event.CityLocationId = FPinkCabStableId(TEXT("location:rules"));
    Event.VehicleId = FPinkCabStableId(TEXT("vehicle:hero"));
    Event.Severity = 0.5f;
    FPinkCabEnforcementLedger Ledger(4);
    TestTrue(TEXT("first event recorded"), Ledger.Record(Event, -3));
    TestFalse(TEXT("duplicate EventId rejected"), Ledger.Record(Event, -3));
    TestEqual(TEXT("duplicate reputation ignored"), Ledger.GetReputationScore(), -3);
    TestEqual(TEXT("duplicate count ignored"), Ledger.GetTotalEventCount(), 1);
    return true;
}

#endif
