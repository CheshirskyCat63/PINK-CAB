#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Enforcement/PinkCabEnforcementEvent.h"
#include "Enforcement/PinkCabEnforcementLedger.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabEnforcementEventRoundTripTest,
    "PinkCab.Enforcement.Foundation.EventRoundTrip",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabEnforcementEventRoundTripTest::RunTest(const FString& Parameters)
{
    FPinkCabEnforcementEvent Event;
    Event.EventId = FPinkCabStableId(TEXT("event:fixture:1"));
    Event.TypeId = FName(TEXT("SpeedCamera"));
    Event.TimestampMs = 123456789;
    Event.CityLocationId = FPinkCabStableId(TEXT("location:fixture:1"));
    Event.VehicleId = FPinkCabStableId(TEXT("vehicle:tatra:hero"));
    Event.PassengerId = FPinkCabStableId(TEXT("passenger:fixture:1"));
    Event.Severity = 0.75f;
    Event.EvidenceFlags = 5u;
    Event.Context = TEXT("fixture-context");

    TArray<uint8> Bytes;
    TestTrue(TEXT("event serializes"), FPinkCabEnforcementEventCodec::Serialize(Event, Bytes));
    TestTrue(TEXT("serialized payload is nonempty"), Bytes.Num() > 0);
    FPinkCabEnforcementEvent Restored;
    TestTrue(TEXT("event deserializes"), FPinkCabEnforcementEventCodec::Deserialize(Bytes, Restored));
    TestEqual(TEXT("stable event id"), Restored.EventId.Serialize(), Event.EventId.Serialize());
    TestEqual(TEXT("type survives"), Restored.TypeId, Event.TypeId);
    TestEqual(TEXT("timestamp survives"), Restored.TimestampMs, Event.TimestampMs);
    TestEqual(TEXT("location survives"), Restored.CityLocationId.Serialize(), Event.CityLocationId.Serialize());
    TestEqual(TEXT("vehicle survives"), Restored.VehicleId.Serialize(), Event.VehicleId.Serialize());
    TestEqual(TEXT("optional passenger survives"), Restored.PassengerId.Serialize(), Event.PassengerId.Serialize());
    TestEqual(TEXT("evidence flags survive"), Restored.EvidenceFlags, Event.EvidenceFlags);
    TestEqual(TEXT("context survives"), Restored.Context, Event.Context);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabEnforcementLedgerBoundedTest,
    "PinkCab.Enforcement.Foundation.BoundedLedger",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabEnforcementLedgerBoundedTest::RunTest(const FString& Parameters)
{
    FPinkCabEnforcementLedger Ledger(2);
    for (int32 Index = 0; Index < 3; ++Index)
    {
        FPinkCabEnforcementEvent Event;
        Event.EventId = FPinkCabStableId(FString::Printf(TEXT("event:%d"), Index));
        Event.TypeId = FName(TEXT("FixtureRule"));
        Event.CityLocationId = FPinkCabStableId(TEXT("loc"));        Event.VehicleId = FPinkCabStableId(TEXT("vehicle"));
        Event.Severity = 0.5f;
        TestTrue(TEXT("event recorded"), Ledger.Record(Event, -3));
    }

    TestEqual(TEXT("recent ledger bounded"), Ledger.GetRecentCount(), 2);
    TestEqual(TEXT("aggregate count keeps history"), Ledger.GetTotalEventCount(), 3);
    TestEqual(TEXT("aggregate score persists beyond trim"), Ledger.GetReputationScore(), -9);

    FPinkCabReputationBandPolicy Bands;
    Bands.CleanMinimum = 0;
    Bands.WatchMinimum = -10;
    TestEqual(TEXT("qualitative presentation comes from caller policy"),
        Ledger.ResolveBand(Bands), EPinkCabReputationBand::Watch);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabEnforcementRuleProfileTest,
    "PinkCab.Enforcement.Foundation.DataDrivenRuleProfile",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabEnforcementRuleProfileTest::RunTest(const FString& Parameters)
{
    FPinkCabEnforcementRuleProfile Profile;
    Profile.SetRule(FName(TEXT("SpeedCamera")), {1250, -4});
    Profile.SetRule(FName(TEXT("Parking")), {500, -1});
    FPinkCabEnforcementRule Rule;
    TestTrue(TEXT("speed rule resolved"), Profile.TryGetRule(FName(TEXT("SpeedCamera")), Rule));
    TestEqual(TEXT("fine is data supplied"), Rule.FineAmountMinor, static_cast<int64>(1250));
    TestEqual(TEXT("reputation delta is data supplied"), Rule.ReputationDelta, -4);
    TestTrue(TEXT("parking shares same profile"), Profile.TryGetRule(FName(TEXT("Parking")), Rule));
    return true;
}
#endif