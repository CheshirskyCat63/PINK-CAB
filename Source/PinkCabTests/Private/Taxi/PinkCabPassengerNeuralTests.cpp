#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Taxi/PinkCabPassengerRecord.h"
#include "Taxi/PinkCabPassengerTemplate.h"

namespace PinkCabPassengerNeuralTests
{
FPinkCabPassengerRecord* CreateRecord(FPinkCabPassengerRegistry& Registry)
{
    FPinkCabPassengerRecord* Record = nullptr;
    Registry.TryCreate(
        FPinkCabStableId(TEXT("passenger:neural:1")),
        FPinkCabPassengerTemplate(FName(TEXT("Commuter")), 72.0f),
        TEXT("city:first-euro:neural"), {}, Record);
    return Record;
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabPassengerNeuralPermissionBlockTest,
    "PinkCab.Taxi.PassengerNeural.PermissionBlock",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabPassengerNeuralPermissionBlockTest::RunTest(const FString& Parameters)
{
    using namespace PinkCabPassengerNeuralTests;
    FPinkCabPassengerRegistry Registry(4, 4, 3, 16, 2, 8);
    FPinkCabPassengerRecord* Record = CreateRecord(Registry);
    TestNotNull(TEXT("record exists"), Record);
    TestFalse(TEXT("permission defaults closed"), Record->HasNeuralPermission());
    TestEqual(TEXT("message denied without permission"),
        Record->AddMessageOnce(FPinkCabStableId(TEXT("message:1")), TEXT("hello")),
        EPinkCabPassengerMutationResult::PermissionDenied);

    Record->SetNeuralPermission(true);
    TestTrue(TEXT("permission enabled"), Record->HasNeuralPermission());
    TestEqual(TEXT("message applies with permission"),
        Record->AddMessageOnce(FPinkCabStableId(TEXT("message:1")), TEXT(" hello ")),
        EPinkCabPassengerMutationResult::Applied);
    TestEqual(TEXT("message normalized"), Record->GetNeuralMessages()[0].Text, FString(TEXT("hello")));

    Record->SetNeuralBlocked(true);
    TestTrue(TEXT("contact blocked"), Record->IsNeuralBlocked());
    TestEqual(TEXT("blocked contact rejects new message"),
        Record->AddMessageOnce(FPinkCabStableId(TEXT("message:2")), TEXT("blocked")),
        EPinkCabPassengerMutationResult::Blocked);
    TestEqual(TEXT("block retains prior history"), Record->GetNeuralMessages().Num(), 1);

    Record->SetNeuralBlocked(false);
    TestEqual(TEXT("unblocked contact accepts message"),
        Record->AddMessageOnce(FPinkCabStableId(TEXT("message:2")), TEXT("back")),
        EPinkCabPassengerMutationResult::Applied);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabPassengerNeuralExactlyOnceValidationTest,
    "PinkCab.Taxi.PassengerNeural.ExactlyOnceValidation",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPinkCabPassengerNeuralExactlyOnceValidationTest::RunTest(const FString& Parameters)
{
    using namespace PinkCabPassengerNeuralTests;
    FPinkCabPassengerRegistry Registry(4, 4, 3, 16, 3, 8);
    FPinkCabPassengerRecord* Record = CreateRecord(Registry);
    Record->SetNeuralPermission(true);

    const FPinkCabStableId MessageId(TEXT("message:once"));
    TestEqual(TEXT("first message applies"), Record->AddMessageOnce(MessageId, TEXT("one")),
        EPinkCabPassengerMutationResult::Applied);
    TestEqual(TEXT("duplicate id rejected"), Record->AddMessageOnce(MessageId, TEXT("changed")),
        EPinkCabPassengerMutationResult::Duplicate);
    TestEqual(TEXT("duplicate does not mutate history"), Record->GetNeuralMessages().Num(), 1);
    TestEqual(TEXT("blank text invalid"),
        Record->AddMessageOnce(FPinkCabStableId(TEXT("message:blank")), TEXT("   ")),
        EPinkCabPassengerMutationResult::Invalid);
    TestEqual(TEXT("invalid message id rejected"),
        Record->AddMessageOnce(FPinkCabStableId(), TEXT("hello")),
        EPinkCabPassengerMutationResult::Invalid);

    FPinkCabPassengerRecord InvalidRecord;
    InvalidRecord.SetNeuralPermission(true);
    TestEqual(TEXT("invalid passenger identity rejects message"),
        InvalidRecord.AddMessageOnce(FPinkCabStableId(TEXT("message:invalid-owner")), TEXT("hello")),
        EPinkCabPassengerMutationResult::Invalid);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabPassengerNeuralBoundedHistoryTest,
    "PinkCab.Taxi.PassengerNeural.BoundedHistory",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPinkCabPassengerNeuralBoundedHistoryTest::RunTest(const FString& Parameters)
{
    using namespace PinkCabPassengerNeuralTests;
    FPinkCabPassengerRegistry Registry(4, 4, 3, 16, 2, 8);
    FPinkCabPassengerRecord* Record = CreateRecord(Registry);
    Record->SetNeuralPermission(true);

    for (int32 Index = 1; Index <= 3; ++Index)
    {
        TestEqual(TEXT("message applies"), Record->AddMessageOnce(
            FPinkCabStableId(FString::Printf(TEXT("message:%d"), Index)),
            FString::Printf(TEXT("text %d"), Index)),
            EPinkCabPassengerMutationResult::Applied);
    }
    TestEqual(TEXT("message history bounded"), Record->GetNeuralMessages().Num(), 2);
    TestEqual(TEXT("oldest message evicted"),
        Record->GetNeuralMessages()[0].MessageId.Serialize(), FString(TEXT("message:2")));
    TestEqual(TEXT("latest message retained"),
        Record->GetNeuralMessages()[1].MessageId.Serialize(), FString(TEXT("message:3")));
    TestEqual(TEXT("evicted message remains replay protected"),
        Record->AddMessageOnce(FPinkCabStableId(TEXT("message:1")), TEXT("again")),
        EPinkCabPassengerMutationResult::Duplicate);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabPassengerNeuralReplayCapacityTest,
    "PinkCab.Taxi.PassengerNeural.ReplayCapacity",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPinkCabPassengerNeuralReplayCapacityTest::RunTest(const FString& Parameters)
{
    using namespace PinkCabPassengerNeuralTests;
    FPinkCabPassengerRegistry Registry(4, 4, 3, 16, 2, 2);
    FPinkCabPassengerRecord* Record = CreateRecord(Registry);
    Record->SetNeuralPermission(true);
    TestEqual(TEXT("first message"),
        Record->AddMessageOnce(FPinkCabStableId(TEXT("message:a")), TEXT("a")),
        EPinkCabPassengerMutationResult::Applied);
    TestEqual(TEXT("second message"),
        Record->AddMessageOnce(FPinkCabStableId(TEXT("message:b")), TEXT("b")),
        EPinkCabPassengerMutationResult::Applied);
    TestEqual(TEXT("full replay journal fails closed"),
        Record->AddMessageOnce(FPinkCabStableId(TEXT("message:c")), TEXT("c")),
        EPinkCabPassengerMutationResult::CapacityExceeded);
    TestEqual(TEXT("failed message does not mutate history"), Record->GetNeuralMessages().Num(), 2);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabPassengerNeuralActorIndependentTest,
    "PinkCab.Taxi.PassengerNeural.ActorIndependent",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabPassengerNeuralActorIndependentTest::RunTest(const FString& Parameters)
{
    using namespace PinkCabPassengerNeuralTests;
    FPinkCabPassengerRegistry Registry(4, 4, 3, 16, 2, 8);
    FPinkCabPassengerRecord* Record = CreateRecord(Registry);
    Record->SetNeuralPermission(true);
    TestEqual(TEXT("message stored on logical record"),
        Record->AddMessageOnce(FPinkCabStableId(TEXT("message:logical")), TEXT("persist me")),
        EPinkCabPassengerMutationResult::Applied);

    const FPinkCabPassengerRecord* Found = Registry.Find(FPinkCabStableId(TEXT("passenger:neural:1")));
    TestNotNull(TEXT("logical record remains in registry without actor"), Found);
    TestTrue(TEXT("permission remains logical state"), Found->HasNeuralPermission());
    TestEqual(TEXT("message remains logical state"), Found->GetNeuralMessages().Num(), 1);
    TestEqual(TEXT("message text survives lookup"), Found->GetNeuralMessages()[0].Text,
        FString(TEXT("persist me")));
    return true;
}
#endif
