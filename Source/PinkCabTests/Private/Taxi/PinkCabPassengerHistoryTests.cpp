#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Taxi/PinkCabPassengerHistory.h"
#include "Taxi/PinkCabNeuralContactState.h"
#include "Taxi/PinkCabFareSession.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabPassengerHistoryPersistenceTest,
    "PinkCab.Taxi.NeuralFoundation.HistoryPersistence",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabPassengerHistoryPersistenceTest::RunTest(const FString& Parameters)
{
    FPinkCabPassengerHistory History(FPinkCabStableId(TEXT("passenger:repeat:1")));
    History.Relationship.Trust = 0.25f;
    History.Relationship.Satisfaction = 0.5f;
    History.Relationship.RiskTolerance = -0.1f;
    TestFalse(TEXT("one paid fare is not repeat promotion"), History.RegisterPaidFare());
    TestTrue(TEXT("two paid fares promote repeat client"), History.RegisterPaidFare());

    TArray<uint8> Bytes;
    TestTrue(TEXT("history serializes"), FPinkCabPassengerHistoryCodec::Serialize(History, Bytes));
    FPinkCabPassengerHistory Restored;
    TestTrue(TEXT("history deserializes"), FPinkCabPassengerHistoryCodec::Deserialize(Bytes, Restored));
    TestEqual(TEXT("identity survives"), Restored.IdentityId.Serialize(), History.IdentityId.Serialize());
    TestEqual(TEXT("paid fares survive"), Restored.GetPaidFareCount(), 2);
    TestTrue(TEXT("repeat eligibility survives without Actor"), Restored.IsRepeatEligible());
    TestEqual(TEXT("trust survives"), Restored.Relationship.Trust, 0.25f);
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabRepeatOrderNormalPipelineTest,
    "PinkCab.Taxi.NeuralFoundation.RepeatOrderNormalPipeline",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabRepeatOrderNormalPipelineTest::RunTest(const FString& Parameters)
{
    const FPinkCabStableId PassengerId(TEXT("passenger:repeat:2"));
    FPinkCabPassengerHistory History(PassengerId);
    History.RegisterAuthoredRelationshipEvent();
    TestTrue(TEXT("authored relationship event promotes repeat client"), History.IsRepeatEligible());

    const FPinkCabOrder Order = FPinkCabRepeatOrderFactory::Create(
        FPinkCabStableId(TEXT("order:repeat:1")),
        FPinkCabStableId(TEXT("pickup:1")),
        FPinkCabStableId(TEXT("destination:1")),
        History,
        EPinkCabFareMode::Metered);
    TestTrue(TEXT("repeat order is ordinary valid order"), Order.IsValid());
    TestEqual(TEXT("normal order carries persistent passenger identity"),
        Order.PassengerIdentityIds[0].Serialize(), PassengerId.Serialize());

    FPinkCabFareSession Fare(Order.OrderId);
    TestEqual(TEXT("repeat order uses normal fare session start state"),
        Fare.GetState(), EPinkCabFareState::Boarding);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(    FPinkCabNeuralContactLogicalTest,
    "PinkCab.Taxi.NeuralFoundation.LogicalContact",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabNeuralContactLogicalTest::RunTest(const FString& Parameters)
{
    FPinkCabNeuralContactState Contact(FPinkCabStableId(TEXT("passenger:logical:1")));
    TestTrue(TEXT("contact accepts simple message"), Contact.AddSimpleMessage(TEXT("Call me when you're nearby")));
    TestTrue(TEXT("second message accepted"), Contact.AddSimpleMessage(TEXT("Same pickup as last time")));
    TestFalse(TEXT("blank message rejected"), Contact.AddSimpleMessage(TEXT("   ")));
    TestEqual(TEXT("identity remains logical stable id"),
        Contact.IdentityId.Serialize(), FString(TEXT("passenger:logical:1")));
    TestEqual(TEXT("simple message history stored as data"), Contact.GetMessages().Num(), 2);
    return true;
}
#endif