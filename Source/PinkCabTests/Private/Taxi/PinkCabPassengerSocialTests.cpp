#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Taxi/PinkCabPassengerRecord.h"
#include "Taxi/PinkCabPassengerTemplate.h"

namespace PinkCabPassengerSocialTests
{
FPinkCabPassengerRecord* CreateRecord(FPinkCabPassengerRegistry& Registry)
{
    FPinkCabPassengerRecord* Record = nullptr;
    Registry.TryCreate(
        FPinkCabStableId(TEXT("passenger:social:1")),
        FPinkCabPassengerTemplate(FName(TEXT("Commuter")), 72.0f),
        TEXT("city:first-euro:social"), {}, Record);
    return Record;
}

FPinkCabPassengerRelationship Delta(float Trust, float Satisfaction, float Risk)
{
    FPinkCabPassengerRelationship Result;
    Result.Trust = Trust;
    Result.Satisfaction = Satisfaction;
    Result.RiskTolerance = Risk;
    return Result;
}
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabPassengerPaidFareExactlyOnceTest,
    "PinkCab.Taxi.PassengerSocial.PaidFareExactlyOnce",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabPassengerPaidFareExactlyOnceTest::RunTest(const FString& Parameters)
{
    using namespace PinkCabPassengerSocialTests;
    FPinkCabPassengerRegistry Registry(4, 4, 3, 8);
    FPinkCabPassengerRecord* Record = CreateRecord(Registry);
    TestNotNull(TEXT("record exists"), Record);
    const FPinkCabStableId EventA(TEXT("social:paid:a"));
    TestEqual(TEXT("first paid event applied"),
        Record->RegisterPaidFareOnce(EventA, Delta(0.1f, 0.2f, 0.0f)),
        EPinkCabPassengerMutationResult::Applied);
    TestEqual(TEXT("duplicate paid event rejected"),
        Record->RegisterPaidFareOnce(EventA, Delta(0.5f, 0.5f, 0.5f)),
        EPinkCabPassengerMutationResult::Duplicate);
    TestEqual(TEXT("paid count remains one"), Record->GetPaidFareCount(), 1);
    TestEqual(TEXT("trust changed once"), Record->Relationship.Trust, 0.1f);
    TestFalse(TEXT("one paid fare not repeat"), Record->IsRepeatEligible());
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabPassengerRepeatPromotionClampTest,
    "PinkCab.Taxi.PassengerSocial.RepeatPromotionClamp",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabPassengerRepeatPromotionClampTest::RunTest(const FString& Parameters)
{
    using namespace PinkCabPassengerSocialTests;
    FPinkCabPassengerRegistry Registry(4, 4, 3, 8);
    FPinkCabPassengerRecord* Record = CreateRecord(Registry);
    TestEqual(TEXT("first paid"), Record->RegisterPaidFareOnce(
        FPinkCabStableId(TEXT("social:paid:1")), Delta(0.8f, 0.7f, -0.8f)),
        EPinkCabPassengerMutationResult::Applied);
    TestEqual(TEXT("second paid"), Record->RegisterPaidFareOnce(
        FPinkCabStableId(TEXT("social:paid:2")), Delta(0.8f, 0.7f, -0.8f)),
        EPinkCabPassengerMutationResult::Applied);
    TestTrue(TEXT("two paid fares promote repeat"), Record->IsRepeatEligible());
    TestEqual(TEXT("paid count two"), Record->GetPaidFareCount(), 2);
    TestEqual(TEXT("trust clamped"), Record->Relationship.Trust, 1.0f);
    TestEqual(TEXT("satisfaction clamped"), Record->Relationship.Satisfaction, 1.0f);
    TestEqual(TEXT("risk clamped"), Record->Relationship.RiskTolerance, -1.0f);
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabPassengerAuthoredPromotionTest,
    "PinkCab.Taxi.PassengerSocial.AuthoredPromotion",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabPassengerAuthoredPromotionTest::RunTest(const FString& Parameters)
{
    using namespace PinkCabPassengerSocialTests;
    FPinkCabPassengerRegistry Registry(4, 4, 3, 8);
    FPinkCabPassengerRecord* Record = CreateRecord(Registry);
    const FPinkCabStableId EventId(TEXT("social:authored:1"));
    TestEqual(TEXT("authored event applies"), Record->RegisterAuthoredEventOnce(
        EventId, Delta(0.2f, 0.1f, 0.0f)), EPinkCabPassengerMutationResult::Applied);
    TestEqual(TEXT("authored event duplicate"), Record->RegisterAuthoredEventOnce(
        EventId, Delta(0.5f, 0.5f, 0.5f)), EPinkCabPassengerMutationResult::Duplicate);
    TestTrue(TEXT("authored event promotes repeat"), Record->IsRepeatEligible());
    TestEqual(TEXT("authored count one"), Record->GetAuthoredEventCount(), 1);
    TestEqual(TEXT("trust applied once"), Record->Relationship.Trust, 0.2f);
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabPassengerRideMemoryBoundTest,
    "PinkCab.Taxi.PassengerSocial.RideMemoryBounded",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabPassengerRideMemoryBoundTest::RunTest(const FString& Parameters)
{
    using namespace PinkCabPassengerSocialTests;
    FPinkCabPassengerRegistry Registry(4, 4, 3, 8);
    FPinkCabPassengerRecord* Record = CreateRecord(Registry);
    for (int32 Index = 1; Index <= 4; ++Index)
    {
        TestEqual(TEXT("memory applies"), Record->AddRideMemoryOnce(
            FPinkCabStableId(FString::Printf(TEXT("memory:%d"), Index)),
            FPinkCabStableId(FString::Printf(TEXT("fare:%d"), Index)),
            FName(TEXT("paid"))), EPinkCabPassengerMutationResult::Applied);
    }
    TestEqual(TEXT("memory ring bounded"), Record->GetRideMemories().Num(), 3);
    TestEqual(TEXT("oldest evicted"), Record->GetRideMemories()[0].MemoryId.Serialize(), FString(TEXT("memory:2")));
    TestEqual(TEXT("latest retained"), Record->GetRideMemories()[2].MemoryId.Serialize(), FString(TEXT("memory:4")));
    TestEqual(TEXT("evicted memory id still replay protected"), Record->AddRideMemoryOnce(
        FPinkCabStableId(TEXT("memory:1")), FPinkCabStableId(TEXT("fare:1")), FName(TEXT("paid"))),
        EPinkCabPassengerMutationResult::Duplicate);
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabPassengerReplayCapacityTest,
    "PinkCab.Taxi.PassengerSocial.ReplayCapacity",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabPassengerReplayCapacityTest::RunTest(const FString& Parameters)
{
    using namespace PinkCabPassengerSocialTests;
    FPinkCabPassengerRegistry Registry(4, 4, 3, 2);
    FPinkCabPassengerRecord* Record = CreateRecord(Registry);
    TestEqual(TEXT("first event"), Record->RegisterPaidFareOnce(
        FPinkCabStableId(TEXT("event:1")), Delta(0.1f, 0.0f, 0.0f)), EPinkCabPassengerMutationResult::Applied);
    TestEqual(TEXT("second event"), Record->RegisterPaidFareOnce(
        FPinkCabStableId(TEXT("event:2")), Delta(0.1f, 0.0f, 0.0f)), EPinkCabPassengerMutationResult::Applied);
    const float TrustBefore = Record->Relationship.Trust;
    TestEqual(TEXT("journal full fails closed"), Record->RegisterPaidFareOnce(
        FPinkCabStableId(TEXT("event:3")), Delta(0.5f, 0.5f, 0.5f)),
        EPinkCabPassengerMutationResult::CapacityExceeded);
    TestEqual(TEXT("full journal no count mutation"), Record->GetPaidFareCount(), 2);
    TestEqual(TEXT("full journal no relationship mutation"), Record->Relationship.Trust, TrustBefore);
    return true;
}
#endif