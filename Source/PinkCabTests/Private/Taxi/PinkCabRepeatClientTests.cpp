#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Taxi/PinkCabFareSession.h"
#include "Taxi/PinkCabPassengerHistory.h"
#include "Taxi/PinkCabPassengerRecord.h"

namespace PinkCabRepeatClientTests
{
FPinkCabPassengerRecord* CreateRecord(FPinkCabPassengerRegistry& Registry)
{
    FPinkCabPassengerRecord* Record = nullptr;
    Registry.TryCreate(
        FPinkCabStableId(TEXT("passenger:repeat:canonical")),
        FPinkCabPassengerTemplate(FName(TEXT("Commuter")), 72.0f),
        TEXT("city:first-euro:repeat"), {}, Record);
    return Record;
}

FPinkCabOrder CreateRepeatOrder(const FPinkCabPassengerRecord& Record, const TCHAR* Suffix)
{
    return FPinkCabRepeatOrderFactory::Create(
        FPinkCabStableId(FString::Printf(TEXT("order:%s"), Suffix)),
        FPinkCabStableId(TEXT("pickup:repeat")),
        FPinkCabStableId(TEXT("destination:repeat")),
        Record,
        EPinkCabFareMode::Metered);
}
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabRepeatClientEligibilityGateTest,
    "PinkCab.Taxi.RepeatClient.EligibilityGate",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabRepeatClientEligibilityGateTest::RunTest(const FString& Parameters)
{
    using namespace PinkCabRepeatClientTests;
    FPinkCabPassengerRegistry Registry(4, 4, 3, 16, 4, 16);
    FPinkCabPassengerRecord* Record = CreateRecord(Registry);
    TestNotNull(TEXT("record exists"), Record);

    const FPinkCabOrder Rejected = CreateRepeatOrder(*Record, TEXT("not-eligible"));
    TestFalse(TEXT("non eligible record cannot create repeat order"), Rejected.IsValid());

    FPinkCabPassengerRelationship Delta;
    TestEqual(TEXT("first paid fare applies"),
        Record->RegisterPaidFareOnce(FPinkCabStableId(TEXT("repeat:paid:1")), Delta),
        EPinkCabPassengerMutationResult::Applied);
    TestEqual(TEXT("second paid fare applies"),
        Record->RegisterPaidFareOnce(FPinkCabStableId(TEXT("repeat:paid:2")), Delta),
        EPinkCabPassengerMutationResult::Applied);
    TestTrue(TEXT("canonical record becomes repeat eligible"), Record->IsRepeatEligible());
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabRepeatClientOrdinaryPipelineTest,
    "PinkCab.Taxi.RepeatClient.OrdinaryPipeline",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabRepeatClientOrdinaryPipelineTest::RunTest(const FString& Parameters)
{
    using namespace PinkCabRepeatClientTests;
    FPinkCabPassengerRegistry Registry(4, 4, 3, 16, 4, 16);
    FPinkCabPassengerRecord* Record = CreateRecord(Registry);
    FPinkCabPassengerRelationship Delta;
    Record->RegisterAuthoredEventOnce(FPinkCabStableId(TEXT("repeat:authored:1")), Delta);

    const FPinkCabOrder Order = CreateRepeatOrder(*Record, TEXT("ordinary"));
    TestTrue(TEXT("eligible canonical record creates valid ordinary order"), Order.IsValid());
    TestEqual(TEXT("same stable passenger identity used"),
        Order.PassengerIdentityIds[0].Serialize(), Record->IdentityId.Serialize());
    TestEqual(TEXT("fare mode preserved"), Order.FareMode, EPinkCabFareMode::Metered);

    FPinkCabFareSession Fare(Order.OrderId);
    TestEqual(TEXT("repeat order enters ordinary boarding state"),
        Fare.GetState(), EPinkCabFareState::Boarding);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabRepeatClientLegacyCompatibilityTest,
    "PinkCab.Taxi.RepeatClient.LegacyCompatibility",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabRepeatClientLegacyCompatibilityTest::RunTest(const FString& Parameters)
{
    const FPinkCabStableId PassengerId(TEXT("passenger:repeat:legacy"));
    FPinkCabPassengerHistory History(PassengerId);
    History.RegisterAuthoredRelationshipEvent();
    const FPinkCabOrder Order = FPinkCabRepeatOrderFactory::Create(
        FPinkCabStableId(TEXT("order:legacy")),
        FPinkCabStableId(TEXT("pickup:legacy")),
        FPinkCabStableId(TEXT("destination:legacy")),
        History,
        EPinkCabFareMode::Metered);
    TestTrue(TEXT("legacy overload remains valid"), Order.IsValid());
    TestEqual(TEXT("legacy overload preserves passenger"),
        Order.PassengerIdentityIds[0].Serialize(), PassengerId.Serialize());
    return true;
}
#endif
