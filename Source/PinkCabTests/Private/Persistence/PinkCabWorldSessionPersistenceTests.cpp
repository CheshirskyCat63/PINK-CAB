#include "Misc/AutomationTest.h"
#include "Persistence/PinkCabWorldSessionSnapshot.h"
#include "Persistence/PinkCabWorkdaySessionState.h"
#include "World/PinkCabCityDeltaState.h"
#include "Core/PinkCabStateKernel.h"

namespace PinkCabWorldSessionPersistenceTests
{
FPinkCabCityIdentity City()
{
    return FPinkCabCityIdentity::Create(TEXT("FIRST-EURO"), TEXT("gen-v1"), TEXT("content-v1"));
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabWorldDeltaSnapshotRoundTripTest,
    "PinkCab.Persistence.WorldSessionSnapshot.CityDeltaRoundTrip",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabWorldDeltaSnapshotRoundTripTest::RunTest(const FString& Parameters)
{
    const FPinkCabCityIdentity City = PinkCabWorldSessionPersistenceTests::City();
    FPinkCabCityDeltaState Source(4);
    FString DeltaId;
    TestTrue(TEXT("lane closure added"), Source.TryAddLaneClosure(City, FPinkCabLaneId(TEXT("lane:persist:1")), TEXT("workday:close"), DeltaId));
    FPinkCabCityDeltaSnapshot Snapshot;
    TestTrue(TEXT("capture succeeds"), FPinkCabWorldSessionSnapshotCodec::CaptureCityDeltas(Source, Snapshot));

    FPinkCabCityDeltaState Restored(1);
    TestTrue(TEXT("restore succeeds"), FPinkCabWorldSessionSnapshotCodec::RestoreCityDeltas(Snapshot, Restored));
    TestEqual(TEXT("signature preserved"), Restored.GetReconstructionSignature(), Source.GetReconstructionSignature());
    TestTrue(TEXT("derived closed-lane lookup rebuilt"), Restored.IsLaneClosed(FPinkCabLaneId(TEXT("lane:persist:1"))));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabStateKernelSnapshotRoundTripTest,
    "PinkCab.Persistence.WorldSessionSnapshot.KernelRoundTrip",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabStateKernelSnapshotRoundTripTest::RunTest(const FString& Parameters)
{
    FPinkCabStateKernel Source(0x12345678ull);
    TestEqual(TEXT("sequence one"), Source.NextSequence(), uint64(1));
    TestEqual(TEXT("sequence two"), Source.NextSequence(), uint64(2));

    FPinkCabStateKernelSnapshot Snapshot;
    TestTrue(TEXT("kernel capture"), FPinkCabWorldSessionSnapshotCodec::CaptureKernel(Source, Snapshot));
    FPinkCabStateKernel Restored(1);
    TestTrue(TEXT("kernel restore"), FPinkCabWorldSessionSnapshotCodec::RestoreKernel(Snapshot, Restored));
    TestEqual(TEXT("root seed preserved"), Restored.GetRootSeed(), uint64(0x12345678ull));
    TestEqual(TEXT("sequence resumes"), Restored.NextSequence(), uint64(3));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabWorldDeltaSnapshotAtomicRejectTest,
    "PinkCab.Persistence.WorldSessionSnapshot.AtomicDuplicateDelta",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabWorldDeltaSnapshotAtomicRejectTest::RunTest(const FString& Parameters)
{
    const FPinkCabCityIdentity City = PinkCabWorldSessionPersistenceTests::City();
    FPinkCabCityDeltaState Source(4);
    FString DeltaId;
    TestTrue(TEXT("source delta"), Source.TryAddLaneClosure(City, FPinkCabLaneId(TEXT("lane:source")), TEXT("op:source"), DeltaId));

    FPinkCabCityDeltaSnapshot Snapshot;
    TestTrue(TEXT("capture"), FPinkCabWorldSessionSnapshotCodec::CaptureCityDeltas(Source, Snapshot));
    const FPinkCabCityDeltaSnapshotRecord Duplicate = Snapshot.Records[0];
    Snapshot.Records.Add(Duplicate);

    FPinkCabCityDeltaState Target(4);
    FString ExistingId;
    TestTrue(TEXT("target baseline"), Target.TryAddLaneClosure(City, FPinkCabLaneId(TEXT("lane:target")), TEXT("op:target"), ExistingId));
    const FString Before = Target.GetReconstructionSignature();
    TestFalse(TEXT("duplicate snapshot rejected"), FPinkCabWorldSessionSnapshotCodec::RestoreCityDeltas(Snapshot, Target));
    TestEqual(TEXT("target unchanged"), Target.GetReconstructionSignature(), Before);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabWorkdaySessionGateResetTest,
    "PinkCab.Persistence.WorldSessionSnapshot.WorkdayGateReset",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabWorkdaySessionGateResetTest::RunTest(const FString& Parameters)
{
    FPinkCabWorkdaySessionState State;
    TestTrue(TEXT("workday created"), FPinkCabWorkdaySessionState::TryCreate(
        FPinkCabStableId(TEXT("workday:004")), 4, 900.0, 8, State));
    TestFalse(TEXT("cannot advance before summary"), State.CanAdvanceToNextWorkday());
    TestTrue(TEXT("elapsed advances"), State.AdvanceElapsedSeconds(30.0));
    TestTrue(TEXT("summary commits"), State.MarkSummaryCommitted());
    TestTrue(TEXT("household transaction commits"), State.RecordHouseholdTransactionOnce(FPinkCabStableId(TEXT("workday:004:household"))));
    TestFalse(TEXT("duplicate household transaction rejected"), State.RecordHouseholdTransactionOnce(FPinkCabStableId(TEXT("workday:004:household"))));
    TestTrue(TEXT("gate now passes"), State.CanAdvanceToNextWorkday());
    State.MarkTerminalRecovery();
    TestTrue(TEXT("terminal marker set"), State.WasTerminalRecovery());

    FPinkCabWorkdaySessionSnapshot Snapshot;
    TestTrue(TEXT("workday capture"), FPinkCabWorldSessionSnapshotCodec::CaptureWorkday(State, Snapshot));
    FPinkCabWorkdaySessionState Restored;
    TestTrue(TEXT("workday restore"), FPinkCabWorldSessionSnapshotCodec::RestoreWorkday(Snapshot, Restored));
    TestEqual(TEXT("elapsed preserved"), Restored.GetElapsedGameSeconds(), 930.0);
    TestTrue(TEXT("restored gate passes"), Restored.CanAdvanceToNextWorkday());

    TestTrue(TEXT("next workday reset"), Restored.ResetForNextWorkday(FPinkCabStableId(TEXT("workday:005")), 5));
    TestEqual(TEXT("ordinal advanced"), Restored.GetOrdinal(), 5);
    TestEqual(TEXT("elapsed reset"), Restored.GetElapsedGameSeconds(), 0.0);
    TestFalse(TEXT("summary reset"), Restored.IsSummaryCommitted());
    TestEqual(TEXT("household count reset"), Restored.GetHouseholdTransactionCount(), 0);
    TestFalse(TEXT("terminal marker reset"), Restored.WasTerminalRecovery());
    return true;
}
