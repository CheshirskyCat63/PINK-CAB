#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Persistence/PinkCabEconomySnapshot.h"
#include "Economy/PinkCabEconomyLedger.h"
#include "Economy/PinkCabFareSettlementService.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabEconomySnapshotPaidReplayTest,
    "PinkCab.Persistence.EconomySnapshot.PaidReplay",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabEconomySnapshotPaidReplayTest::RunTest(const FString& Parameters)
{
    FPinkCabEconomyLedger Ledger(1000, 500, 8);
    FPinkCabFareSettlementService Settlement(8);
    const FPinkCabStableId FareId(TEXT("fare:economy:snapshot:paid"));
    TestEqual(TEXT("paid fare commits"),
        Settlement.CommitPaid(FareId, 1250, 250, Ledger).Disposition,
        EPinkCabFareSettlementDisposition::Committed);

    FPinkCabEconomySnapshot Snapshot;
    TestTrue(TEXT("capture succeeds"),
        FPinkCabEconomySnapshotCodec::Capture(Ledger, Settlement, Snapshot));
    TestEqual(TEXT("balance captured"), Snapshot.BalanceMinor, int64(2500));
    TestEqual(TEXT("debt limit captured"), Snapshot.DebtLimitMinor, int64(500));
    TestEqual(TEXT("transaction capacity captured"), Snapshot.MaxCommittedTransactionIds, 8);
    TestEqual(TEXT("fare capacity captured"), Snapshot.MaxResolvedFares, 8);
    FPinkCabEconomyLedger RestoredLedger(1, 0, 1);
    FPinkCabFareSettlementService RestoredSettlement(1);
    TestTrue(TEXT("restore succeeds"),
        FPinkCabEconomySnapshotCodec::Restore(Snapshot, RestoredLedger, RestoredSettlement));
    TestEqual(TEXT("balance restored"), RestoredLedger.GetBalanceMinor(), int64(2500));
    TestEqual(TEXT("resolved fare restored"), RestoredSettlement.NumResolvedFares(), 1);

    const int64 BeforeReplay = RestoredLedger.GetBalanceMinor();
    const FPinkCabFareSettlementResult Replay = RestoredSettlement.CommitPaid(
        FareId, 1250, 250, RestoredLedger);
    TestEqual(TEXT("paid retry replays"), Replay.Disposition,
        EPinkCabFareSettlementDisposition::Replayed);
    TestEqual(TEXT("retry cannot double credit"),
        RestoredLedger.GetBalanceMinor(), BeforeReplay);

    const EPinkCabSettlementResult RecoveryDebit = RestoredLedger.Commit(
        FPinkCabEconomyTransaction::Debit(
            FPinkCabTransactionId(TEXT("economy:recovery:debit")),
            EPinkCabTransactionType::Repair,
            2800,
            EPinkCabEconomyPurpose::EssentialRecovery));
    TestEqual(TEXT("restored debt limit remains usable"),
        RecoveryDebit, EPinkCabSettlementResult::Committed);
    TestEqual(TEXT("debt balance"), RestoredLedger.GetBalanceMinor(), int64(-300));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabEconomySnapshotEvasionTest,
    "PinkCab.Persistence.EconomySnapshot.EvasionReplayConflict",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPinkCabEconomySnapshotEvasionTest::RunTest(const FString& Parameters)
{
    FPinkCabEconomyLedger Ledger(900, 0, 8);
    FPinkCabFareSettlementService Settlement(8);
    const FPinkCabStableId FareId(TEXT("fare:economy:snapshot:evasion"));
    const FPinkCabFareSettlementResult First = Settlement.RecordEvasion(FareId);
    TestEqual(TEXT("evasion commits"), First.Disposition,
        EPinkCabFareSettlementDisposition::Committed);
    TestTrue(TEXT("consequence id valid"), First.ConsequenceId.IsValid());

    FPinkCabEconomySnapshot Snapshot;
    TestTrue(TEXT("capture succeeds"),
        FPinkCabEconomySnapshotCodec::Capture(Ledger, Settlement, Snapshot));
    FPinkCabEconomyLedger RestoredLedger(0, 0, 1);
    FPinkCabFareSettlementService RestoredSettlement(1);
    TestTrue(TEXT("restore succeeds"),
        FPinkCabEconomySnapshotCodec::Restore(Snapshot, RestoredLedger, RestoredSettlement));

    const FPinkCabFareSettlementResult Replay = RestoredSettlement.RecordEvasion(FareId);
    TestEqual(TEXT("evasion retry replays"), Replay.Disposition,
        EPinkCabFareSettlementDisposition::Replayed);
    TestEqual(TEXT("same consequence survives"),
        Replay.ConsequenceId.Serialize(), First.ConsequenceId.Serialize());
    TestEqual(TEXT("paid after evasion remains conflict"),
        RestoredSettlement.CommitPaid(FareId, 500, 0, RestoredLedger).Disposition,
        EPinkCabFareSettlementDisposition::Conflict);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabEconomySnapshotAtomicRejectTest,
    "PinkCab.Persistence.EconomySnapshot.AtomicInvalidReplay",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPinkCabEconomySnapshotAtomicRejectTest::RunTest(const FString& Parameters)
{
    FPinkCabEconomyLedger SourceLedger(1200, 300, 4);
    FPinkCabFareSettlementService SourceSettlement(4);
    SourceSettlement.CommitPaid(
        FPinkCabStableId(TEXT("fare:economy:atomic")), 400, 0, SourceLedger);
    FPinkCabEconomySnapshot Snapshot;
    TestTrue(TEXT("capture valid source"),
        FPinkCabEconomySnapshotCodec::Capture(SourceLedger, SourceSettlement, Snapshot));
    const FString DuplicateTransactionId = Snapshot.CommittedTransactionIds[0];
    Snapshot.CommittedTransactionIds.Add(DuplicateTransactionId);

    FPinkCabEconomyLedger TargetLedger(77, 0, 2);
    FPinkCabFareSettlementService TargetSettlement(2);
    TargetSettlement.RecordEvasion(FPinkCabStableId(TEXT("fare:sentinel:evasion")));
    const int64 BeforeBalance = TargetLedger.GetBalanceMinor();
    const int32 BeforeResolved = TargetSettlement.NumResolvedFares();

    TestFalse(TEXT("duplicate replay id rejected"),
        FPinkCabEconomySnapshotCodec::Restore(Snapshot, TargetLedger, TargetSettlement));
    TestEqual(TEXT("ledger remains atomic"), TargetLedger.GetBalanceMinor(), BeforeBalance);
    TestEqual(TEXT("settlement remains atomic"),
        TargetSettlement.NumResolvedFares(), BeforeResolved);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabEconomySnapshotCapacityTest,
    "PinkCab.Persistence.EconomySnapshot.BoundedReplay",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabEconomySnapshotCapacityTest::RunTest(const FString& Parameters)
{
    FPinkCabEconomyLedger Ledger(100, 0, 1);
    TestEqual(TEXT("first transaction commits"), Ledger.Commit(
        FPinkCabEconomyTransaction::Credit(
            FPinkCabTransactionId(TEXT("economy:bounded:1")),
            EPinkCabTransactionType::FareIncome, 10)),
        EPinkCabSettlementResult::Committed);
    TestEqual(TEXT("ledger journal fails closed"), Ledger.Commit(
        FPinkCabEconomyTransaction::Credit(
            FPinkCabTransactionId(TEXT("economy:bounded:2")),
            EPinkCabTransactionType::FareIncome, 10)),
        EPinkCabSettlementResult::CapacityExceeded);
    TestEqual(TEXT("capacity failure cannot mutate balance"), Ledger.GetBalanceMinor(), int64(110));

    FPinkCabFareSettlementService Settlement(1);
    TestEqual(TEXT("first fare resolution commits"),
        Settlement.RecordEvasion(FPinkCabStableId(TEXT("fare:bounded:1"))).Disposition,
        EPinkCabFareSettlementDisposition::Committed);
    TestEqual(TEXT("fare journal fails closed"),
        Settlement.RecordEvasion(FPinkCabStableId(TEXT("fare:bounded:2"))).Disposition,
        EPinkCabFareSettlementDisposition::CapacityExceeded);
    TestEqual(TEXT("fare capacity remains bounded"), Settlement.NumResolvedFares(), 1);

    FPinkCabEconomyLedger TightLedger(100, 0, 1);
    FPinkCabFareSettlementService TightSettlement(2);
    const FPinkCabFareSettlementResult AtomicCapacity = TightSettlement.CommitPaid(
        FPinkCabStableId(TEXT("fare:bounded:atomic")), 10, 5, TightLedger);
    TestEqual(TEXT("insufficient ledger replay space rejects fare atomically"),
        AtomicCapacity.Disposition, EPinkCabFareSettlementDisposition::CapacityExceeded);
    TestEqual(TEXT("failed fare cannot partially credit"), TightLedger.GetBalanceMinor(), int64(100));
    TestEqual(TEXT("failed fare cannot record settlement"), TightSettlement.NumResolvedFares(), 0);
    return true;
}
#endif
