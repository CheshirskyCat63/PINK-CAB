#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Core/PinkCabBuildIdentity.h"
#include "Core/PinkCabDeterministicSeed.h"
#include "Core/PinkCabEventEnvelope.h"
#include "Core/PinkCabExactlyOnceStore.h"
#include "Core/PinkCabStateService.h"
#include "Core/PinkCabStateKernel.h"
#include "Economy/PinkCabTransaction.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabDeterministicSeedTest,
    "PinkCab.Core.Foundation.DeterministicSeed",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabDeterministicSeedTest::RunTest(const FString& Parameters)
{
    const uint64 RootA = FPinkCabDeterministicSeed::FromText(TEXT("FIRST-EURO"));
    const uint64 RootB = FPinkCabDeterministicSeed::FromText(TEXT("FIRST-EURO"));
    TestEqual(TEXT("same text produces same seed"), RootA, RootB);
    TestTrue(TEXT("seed is non-zero"), RootA != 0);

    const FPinkCabStableId Passenger(TEXT("passenger:repeat:001"));
    const uint64 FareSeed = FPinkCabDeterministicSeed::Derive(RootA, Passenger, TEXT("fare"));
    const uint64 NeuralSeed = FPinkCabDeterministicSeed::Derive(RootA, Passenger, TEXT("neural"));
    TestTrue(TEXT("domain separation changes derived seed"), FareSeed != NeuralSeed);
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabEventEnvelopeTest,
    "PinkCab.Core.Foundation.EventEnvelope",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabEventEnvelopeTest::RunTest(const FString& Parameters)
{
    const FPinkCabEventEnvelope Envelope = FPinkCabEventEnvelope::Create(
        FPinkCabStableId(TEXT("event:fare:001")),
        FPinkCabStableId(TEXT("fare:001")),
        TEXT("FareStarted"),
        FPinkCabSchemaVersion(1, 0, 0),
        7,
        123456789ull);

    TestTrue(TEXT("event envelope validates"), Envelope.IsValid());
    TestEqual(TEXT("sequence is preserved"), Envelope.GetSequence(), uint64(7));
    TestEqual(TEXT("deterministic seed is preserved"), Envelope.GetDeterministicSeed(), uint64(123456789));
    TestEqual(TEXT("event type is preserved"), Envelope.GetEventType(), FString(TEXT("FareStarted")));
    return true;
}

struct FPinkCabReplayReceipt
{
    int32 Value = 0;
    bool operator==(const FPinkCabReplayReceipt& Other) const { return Value == Other.Value; }
};
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabExactlyOnceStoreTest,
    "PinkCab.Core.Foundation.ExactlyOnce",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabExactlyOnceStoreTest::RunTest(const FString& Parameters)
{
    TPinkCabExactlyOnceStore<FPinkCabReplayReceipt> Store;
    int32 Executions = 0;
    const FPinkCabStableId OperationId(TEXT("operation:fare-settlement:001"));

    const auto First = Store.Execute(OperationId, [&Executions]()
    {
        ++Executions;
        return FPinkCabReplayReceipt{42};
    });
    const auto Replay = Store.Execute(OperationId, [&Executions]()
    {
        ++Executions;
        return FPinkCabReplayReceipt{99};
    });

    TestEqual(TEXT("first call executes"), First.Disposition, EPinkCabReplayDisposition::Executed);
    TestEqual(TEXT("duplicate replays"), Replay.Disposition, EPinkCabReplayDisposition::Replayed);
    TestEqual(TEXT("original result is replayed"), Replay.Result.Value, 42);
    TestEqual(TEXT("mutation executes once"), Executions, 1);
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabStateKernelResetTest,
    "PinkCab.Core.Foundation.StateKernelReset",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabStateKernelResetTest::RunTest(const FString& Parameters)
{
    FPinkCabStateKernel Kernel(777ull);
    TestEqual(TEXT("root seed stored"), Kernel.GetRootSeed(), uint64(777));
    TestEqual(TEXT("first sequence"), Kernel.NextSequence(), uint64(1));
    TestEqual(TEXT("second sequence"), Kernel.NextSequence(), uint64(2));

    Kernel.ResetRuntime();
    Kernel.ResetRuntime();
    TestEqual(TEXT("runtime reset is idempotent"), Kernel.NextSequence(), uint64(1));
    TestEqual(TEXT("runtime reset preserves root seed"), Kernel.GetRootSeed(), uint64(777));

    Kernel.ResetWorkday(999ull);
    Kernel.ResetWorkday(999ull);
    TestEqual(TEXT("workday reset replaces seed deterministically"), Kernel.GetRootSeed(), uint64(999));
    TestEqual(TEXT("workday reset also resets sequence"), Kernel.NextSequence(), uint64(1));
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabFoundationIdentityTest,
    "PinkCab.Core.Foundation.IdentityAndTransactionId",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabFoundationIdentityTest::RunTest(const FString& Parameters)
{
    const FPinkCabBuildIdentity Identity;
    TestTrue(TEXT("active build channel is native Chaos"), Identity.BuildChannel.Contains(TEXT("native-chaos")));
    TestFalse(TEXT("active build identity no longer says pre-fgear"), Identity.BuildChannel.Contains(TEXT("pre-fgear")));
    TestFalse(TEXT("runtime identity no longer says pre-fgear"), Identity.RuntimeVersion.Contains(TEXT("pre-fgear")));

    const FPinkCabTransactionId Valid(TEXT("transaction:fare:001"));
    const FPinkCabTransactionId Whitespace(TEXT("   "));
    const FPinkCabTransactionId Multiline(TEXT("transaction:bad\nline"));
    TestTrue(TEXT("normal transaction ID is valid"), Valid.IsValid());
    TestFalse(TEXT("whitespace transaction ID is rejected"), Whitespace.IsValid());
    TestFalse(TEXT("multiline transaction ID is rejected by Core ID rules"), Multiline.IsValid());
    return true;
}

#endif
#if WITH_DEV_AUTOMATION_TESTS
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabExactlyOnceInvalidAndResetTest,
    "PinkCab.Core.Foundation.ExactlyOnceInvalidAndReset",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabExactlyOnceInvalidAndResetTest::RunTest(const FString& Parameters)
{
    TPinkCabExactlyOnceStore<FPinkCabReplayReceipt> Store;
    int32 Executions = 0;
    const auto Invalid = Store.Execute(FPinkCabStableId(), [&Executions]()
    {
        ++Executions;
        return FPinkCabReplayReceipt{7};
    });
    TestEqual(TEXT("invalid ID is rejected"), Invalid.Disposition, EPinkCabReplayDisposition::InvalidId);
    TestEqual(TEXT("invalid ID never executes mutation"), Executions, 0);

    const FPinkCabStableId ValidId(TEXT("operation:reset-check"));
    Store.Execute(ValidId, [&Executions]() { ++Executions; return FPinkCabReplayReceipt{1}; });
    Store.Clear();
    const auto AfterClear = Store.Execute(ValidId, [&Executions]() { ++Executions; return FPinkCabReplayReceipt{2}; });
    TestEqual(TEXT("clear permits fresh execution"), AfterClear.Disposition, EPinkCabReplayDisposition::Executed);
    TestEqual(TEXT("clear causes exactly one new mutation"), Executions, 2);
    return true;
}

#endif
