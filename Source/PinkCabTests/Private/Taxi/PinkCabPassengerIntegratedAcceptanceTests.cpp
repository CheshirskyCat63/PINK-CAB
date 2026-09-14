#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Taxi/PinkCabFareSession.h"
#include "Taxi/PinkCabPassengerHistory.h"
#include "Taxi/PinkCabPassengerRecord.h"
#include "Taxi/PinkCabPassengerSnapshot.h"

namespace PinkCabPassengerAcceptanceTests
{
FPinkCabPassengerRecord* AddPassenger(
    FPinkCabPassengerRegistry& Registry,
    const TCHAR* Id,
    const TCHAR* Context)
{
    FPinkCabPassengerRecord* Record = nullptr;
    Registry.TryCreate(
        FPinkCabStableId(Id),
        FPinkCabPassengerTemplate(FName(TEXT("Commuter")), 72.0f),
        Context,
        {FName(TEXT("quiet")), FName(TEXT("airport"))},
        Record);
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
    FPinkCabPassengerIntegratedEndToEndTest,
    "PinkCab.Taxi.PassengerAcceptance.EndToEnd",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabPassengerIntegratedEndToEndTest::RunTest(const FString& Parameters)
{
    using namespace PinkCabPassengerAcceptanceTests;
    const FPinkCabStableId PassengerId(TEXT("passenger:acceptance:1"));
    FPinkCabPassengerRegistry Registry(4, 4, 3, 16, 3, 16);
    FPinkCabPassengerRecord* Record = AddPassenger(
        Registry, TEXT("passenger:acceptance:1"), TEXT("city:first-euro:acceptance"));
    TestNotNull(TEXT("canonical passenger created"), Record);
    TestEqual(TEXT("first paid fare applies"), Record->RegisterPaidFareOnce(
        FPinkCabStableId(TEXT("social:acceptance:1")), Delta(0.2f, 0.3f, 0.0f)),
        EPinkCabPassengerMutationResult::Applied);
    TestEqual(TEXT("ride memory applies"), Record->AddRideMemoryOnce(
        FPinkCabStableId(TEXT("memory:acceptance:1")),
        FPinkCabStableId(TEXT("fare:acceptance:1")), FName(TEXT("paid"))),
        EPinkCabPassengerMutationResult::Applied);
    TestFalse(TEXT("one paid fare not repeat yet"), Record->IsRepeatEligible());

    Record = nullptr;
    Record = Registry.Find(PassengerId);
    TestNotNull(TEXT("actor stream-out does not delete logical record"), Record);
    Record->SetNeuralPermission(true);
    TestEqual(TEXT("neural message applies after stream-out"), Record->AddMessageOnce(
        FPinkCabStableId(TEXT("message:acceptance:1")), TEXT("same pickup tomorrow")),
        EPinkCabPassengerMutationResult::Applied);
    FPinkCabPassengerSnapshot Snapshot;
    TestTrue(TEXT("canonical passenger snapshot captured"),
        FPinkCabPassengerSnapshotCodec::Capture(Registry, Snapshot));
    FPinkCabPassengerRegistry Restored(1, 1, 1, 1, 1, 1);
    TestTrue(TEXT("canonical passenger snapshot restored"),
        FPinkCabPassengerSnapshotCodec::Restore(Snapshot, Restored));
    Record = Restored.Find(PassengerId);
    TestNotNull(TEXT("restored passenger exists"), Record);
    TestEqual(TEXT("memory survived restore"), Record->GetRideMemories().Num(), 1);
    TestEqual(TEXT("message survived restore"), Record->GetNeuralMessages().Num(), 1);

    TestEqual(TEXT("second paid fare promotes repeat client"), Record->RegisterPaidFareOnce(
        FPinkCabStableId(TEXT("social:acceptance:2")), Delta(0.2f, 0.1f, 0.0f)),
        EPinkCabPassengerMutationResult::Applied);
    TestTrue(TEXT("repeat eligibility promoted"), Record->IsRepeatEligible());
    const FPinkCabOrder Order = FPinkCabRepeatOrderFactory::Create(
        FPinkCabStableId(TEXT("order:acceptance:repeat")),
        FPinkCabStableId(TEXT("pickup:acceptance")),
        FPinkCabStableId(TEXT("destination:acceptance")),
        *Record,
        EPinkCabFareMode::Metered);
    TestTrue(TEXT("repeat client creates ordinary valid order"), Order.IsValid());
    TestEqual(TEXT("order retains canonical passenger id"),
        Order.PassengerIdentityIds[0].Serialize(), PassengerId.Serialize());
    FPinkCabFareSession Fare(Order.OrderId);
    TestEqual(TEXT("ordinary fare starts in boarding"), Fare.GetState(), EPinkCabFareState::Boarding);
    TestEqual(TEXT("ordinary fare starts meter normally"), Fare.TryStartMeter(true, true),
        EPinkCabFareTransitionResult::Applied);
    TestEqual(TEXT("ordinary fare becomes active"), Fare.GetState(), EPinkCabFareState::Active);
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabPassengerIntegratedReplayAndBlockTest,
    "PinkCab.Taxi.PassengerAcceptance.ReplayAndBlockFailures",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabPassengerIntegratedReplayAndBlockTest::RunTest(const FString& Parameters)
{
    using namespace PinkCabPassengerAcceptanceTests;
    FPinkCabPassengerRegistry Registry(4, 4, 3, 16, 3, 16);
    FPinkCabPassengerRecord* Record = AddPassenger(
        Registry, TEXT("passenger:acceptance:replay"), TEXT("acceptance:replay"));
    const FPinkCabStableId SocialId(TEXT("social:acceptance:dup"));
    TestEqual(TEXT("social event applies"),
        Record->RegisterPaidFareOnce(SocialId, Delta(0.2f, 0.0f, 0.0f)),
        EPinkCabPassengerMutationResult::Applied);
    TestEqual(TEXT("duplicate social event rejected"),
        Record->RegisterPaidFareOnce(SocialId, Delta(1.0f, 1.0f, 1.0f)),
        EPinkCabPassengerMutationResult::Duplicate);
    TestEqual(TEXT("duplicate did not increment fare count"), Record->GetPaidFareCount(), 1);

    Record->SetNeuralPermission(true);
    const FPinkCabStableId MessageId(TEXT("message:acceptance:dup"));
    TestEqual(TEXT("message applies"), Record->AddMessageOnce(MessageId, TEXT("hello")),
        EPinkCabPassengerMutationResult::Applied);
    TestEqual(TEXT("duplicate message rejected"), Record->AddMessageOnce(MessageId, TEXT("changed")),
        EPinkCabPassengerMutationResult::Duplicate);
    Record->SetNeuralBlocked(true);
    TestEqual(TEXT("blocked contact rejects new message"), Record->AddMessageOnce(
        FPinkCabStableId(TEXT("message:acceptance:blocked")), TEXT("blocked")),
        EPinkCabPassengerMutationResult::Blocked);
    TestEqual(TEXT("blocked contact retains prior history"), Record->GetNeuralMessages().Num(), 1);
    Record->SetNeuralBlocked(false);
    TestEqual(TEXT("unblocked contact accepts fresh message"), Record->AddMessageOnce(
        FPinkCabStableId(TEXT("message:acceptance:blocked")), TEXT("back")),
        EPinkCabPassengerMutationResult::Applied);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabPassengerIntegratedCapacityTest,
    "PinkCab.Taxi.PassengerAcceptance.CapacityFailures",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabPassengerIntegratedCapacityTest::RunTest(const FString& Parameters)
{
    using namespace PinkCabPassengerAcceptanceTests;
    FPinkCabPassengerRegistry Registry(1, 4, 1, 2, 1, 2);
    FPinkCabPassengerRecord* Record = AddPassenger(
        Registry, TEXT("passenger:acceptance:capacity"), TEXT("acceptance:capacity"));
    FPinkCabPassengerRecord* Overflow = nullptr;
    TestFalse(TEXT("registry record capacity enforced"), Registry.TryCreate(
        FPinkCabStableId(TEXT("passenger:acceptance:overflow")),
        FPinkCabPassengerTemplate(FName(TEXT("Commuter")), 72.0f),
        TEXT("acceptance:overflow"), {}, Overflow));

    TestEqual(TEXT("first social journal entry applies"), Record->RegisterPaidFareOnce(
        FPinkCabStableId(TEXT("social:capacity:1")), Delta(0.1f, 0.0f, 0.0f)),
        EPinkCabPassengerMutationResult::Applied);
    TestEqual(TEXT("second social journal entry applies"), Record->RegisterPaidFareOnce(
        FPinkCabStableId(TEXT("social:capacity:2")), Delta(0.1f, 0.0f, 0.0f)),
        EPinkCabPassengerMutationResult::Applied);
    TestEqual(TEXT("full social replay journal fails closed"), Record->RegisterPaidFareOnce(
        FPinkCabStableId(TEXT("social:capacity:3")), Delta(0.9f, 0.9f, 0.9f)),
        EPinkCabPassengerMutationResult::CapacityExceeded);
    TestEqual(TEXT("failed social mutation did not increment count"), Record->GetPaidFareCount(), 2);

    Record->SetNeuralPermission(true);
    TestEqual(TEXT("first neural replay entry applies"), Record->AddMessageOnce(
        FPinkCabStableId(TEXT("message:capacity:1")), TEXT("one")),
        EPinkCabPassengerMutationResult::Applied);
    TestEqual(TEXT("second neural replay entry applies"), Record->AddMessageOnce(
        FPinkCabStableId(TEXT("message:capacity:2")), TEXT("two")),
        EPinkCabPassengerMutationResult::Applied);
    TestEqual(TEXT("message ring remains bounded"), Record->GetNeuralMessages().Num(), 1);
    TestEqual(TEXT("full neural replay journal fails closed"), Record->AddMessageOnce(
        FPinkCabStableId(TEXT("message:capacity:3")), TEXT("three")),
        EPinkCabPassengerMutationResult::CapacityExceeded);
    TestEqual(TEXT("failed neural mutation keeps bounded ring"), Record->GetNeuralMessages().Num(), 1);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabPassengerIntegratedInvalidSnapshotTest,
    "PinkCab.Taxi.PassengerAcceptance.InvalidSnapshotAtomicity",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabPassengerIntegratedInvalidSnapshotTest::RunTest(const FString& Parameters)
{
    using namespace PinkCabPassengerAcceptanceTests;
    FPinkCabPassengerRegistry Source(2, 4, 2, 8, 2, 8);
    FPinkCabPassengerRecord* SourceRecord = AddPassenger(
        Source, TEXT("passenger:acceptance:snapshot"), TEXT("acceptance:snapshot"));
    TestNotNull(TEXT("source passenger created"), SourceRecord);
    FPinkCabPassengerSnapshot Snapshot;
    TestTrue(TEXT("valid snapshot captured"),
        FPinkCabPassengerSnapshotCodec::Capture(Source, Snapshot));
    Snapshot.SchemaVersion = FPinkCabPassengerSnapshot::CurrentSchemaVersion + 1;

    FPinkCabPassengerRegistry Target(2, 4, 2, 8, 2, 8);
    const FPinkCabStableId SentinelId(TEXT("passenger:acceptance:sentinel"));
    TestNotNull(TEXT("sentinel passenger created"), AddPassenger(
        Target, TEXT("passenger:acceptance:sentinel"), TEXT("acceptance:sentinel")));
    const uint64 Before = Target.GetReconstructionSignature();
    TestFalse(TEXT("invalid snapshot rejected"),
        FPinkCabPassengerSnapshotCodec::Restore(Snapshot, Target));
    TestEqual(TEXT("invalid restore is atomic"), Target.GetReconstructionSignature(), Before);
    TestNotNull(TEXT("sentinel remains after invalid restore"), Target.Find(SentinelId));
    return true;
}
#endif
