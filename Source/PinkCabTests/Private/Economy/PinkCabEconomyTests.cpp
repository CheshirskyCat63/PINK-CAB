#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Economy/PinkCabTransaction.h"
#include "Economy/PinkCabEconomyLedger.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabEconomyExactlyOnceTest,
    "PinkCab.Economy.ExactlyOnce",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabEconomyExactlyOnceTest::RunTest(const FString& Parameters)
{
    FPinkCabEconomyLedger Ledger(1000, 0);
    const FPinkCabEconomyTransaction Fare = FPinkCabEconomyTransaction::Credit(
        FPinkCabTransactionId(TEXT("fare-001")), EPinkCabTransactionType::FareIncome, 1800);
    TestEqual(TEXT("first fare commits"), Ledger.Commit(Fare), EPinkCabSettlementResult::Committed);
    TestEqual(TEXT("balance after fare"), Ledger.GetBalanceMinor(), int64(2800));
    TestEqual(TEXT("duplicate is detected"), Ledger.Commit(Fare), EPinkCabSettlementResult::Duplicate);
    TestEqual(TEXT("duplicate does not credit twice"), Ledger.GetBalanceMinor(), int64(2800));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabEconomyTypedTransactionsTest,
    "PinkCab.Economy.TypedTransactions",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabEconomyTypedTransactionsTest::RunTest(const FString& Parameters)
{
    FPinkCabEconomyLedger Ledger(5000, 0);
    TestEqual(TEXT("fare credit"), Ledger.Commit(FPinkCabEconomyTransaction::Credit(FPinkCabTransactionId(TEXT("fare")), EPinkCabTransactionType::FareIncome, 2000)), EPinkCabSettlementResult::Committed);
    TestEqual(TEXT("tip separate credit"), Ledger.Commit(FPinkCabEconomyTransaction::Credit(FPinkCabTransactionId(TEXT("tip")), EPinkCabTransactionType::Tip, 400)), EPinkCabSettlementResult::Committed);
    TestEqual(TEXT("fuel debit"), Ledger.Commit(FPinkCabEconomyTransaction::Debit(FPinkCabTransactionId(TEXT("fuel")), EPinkCabTransactionType::FuelPurchase, 600)), EPinkCabSettlementResult::Committed);
    TestEqual(TEXT("part debit"), Ledger.Commit(FPinkCabEconomyTransaction::Debit(FPinkCabTransactionId(TEXT("part")), EPinkCabTransactionType::PartPurchase, 700)), EPinkCabSettlementResult::Committed);
    TestEqual(TEXT("repair debit"), Ledger.Commit(FPinkCabEconomyTransaction::Debit(FPinkCabTransactionId(TEXT("repair")), EPinkCabTransactionType::Repair, 500)), EPinkCabSettlementResult::Committed);
    TestEqual(TEXT("parking debit"), Ledger.Commit(FPinkCabEconomyTransaction::Debit(FPinkCabTransactionId(TEXT("parking")), EPinkCabTransactionType::Parking, 200)), EPinkCabSettlementResult::Committed);
    TestEqual(TEXT("fine debit"), Ledger.Commit(FPinkCabEconomyTransaction::Debit(FPinkCabTransactionId(TEXT("fine")), EPinkCabTransactionType::Fine, 300)), EPinkCabSettlementResult::Committed);
    TestEqual(TEXT("typed sequence balance"), Ledger.GetBalanceMinor(), int64(5100));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabEconomyReplayTest,
    "PinkCab.Economy.ReplayJournal",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabEconomyReplayTest::RunTest(const FString& Parameters)
{
    TArray<FPinkCabEconomyTransaction> Journal;
    Journal.Add(FPinkCabEconomyTransaction::Credit(FPinkCabTransactionId(TEXT("fare-a")), EPinkCabTransactionType::FareIncome, 1200));
    Journal.Add(FPinkCabEconomyTransaction::Credit(FPinkCabTransactionId(TEXT("tip-a")), EPinkCabTransactionType::Tip, 300));
    Journal.Add(FPinkCabEconomyTransaction::Debit(FPinkCabTransactionId(TEXT("fine-a")), EPinkCabTransactionType::Fine, 250));

    FPinkCabEconomyLedger Ledger(1000, 0);
    for (const FPinkCabEconomyTransaction& Tx : Journal) { Ledger.Commit(Tx); }
    const int64 Once = Ledger.GetBalanceMinor();
    for (const FPinkCabEconomyTransaction& Tx : Journal) { Ledger.Commit(Tx); }
    TestEqual(TEXT("replaying committed journal is idempotent"), Ledger.GetBalanceMinor(), Once);
    TestEqual(TEXT("expected balance"), Once, int64(2250));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabEconomyDebtBoundaryTest,
    "PinkCab.Economy.DebtBoundary",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabEconomyDebtBoundaryTest::RunTest(const FString& Parameters)
{
    FPinkCabEconomyLedger Ledger(500, 1000);
    const auto MinimumRepair = FPinkCabEconomyTransaction::Debit(
        FPinkCabTransactionId(TEXT("repair-min")), EPinkCabTransactionType::Repair, 1200,
        EPinkCabEconomyPurpose::MinimumRoadworthyRepair);
    TestEqual(TEXT("minimum roadworthy repair may use configured debt"), Ledger.Commit(MinimumRepair), EPinkCabSettlementResult::Committed);
    TestEqual(TEXT("debt bounded by configured floor"), Ledger.GetBalanceMinor(), int64(-700));

    const auto Tuning = FPinkCabEconomyTransaction::Debit(
        FPinkCabTransactionId(TEXT("tuning")), EPinkCabTransactionType::PartPurchase, 100,
        EPinkCabEconomyPurpose::DiscretionaryTuning);
    TestEqual(TEXT("discretionary tuning cannot deepen debt"), Ledger.Commit(Tuning), EPinkCabSettlementResult::InsufficientFunds);
    TestEqual(TEXT("rejected tuning does not mutate balance"), Ledger.GetBalanceMinor(), int64(-700));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabEconomyFuelPartsPolicyTest,
    "PinkCab.Economy.FuelPartsPolicy",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabEconomyFuelPartsPolicyTest::RunTest(const FString& Parameters)
{
    FPinkCabEconomyLedger Ledger(650, 1500);
    TestEqual(TEXT("fuel affordable cap ignores debt"), Ledger.GetAffordableFuelSpendMinor(1000), int64(650));
    const auto Part = FPinkCabEconomyTransaction::Debit(
        FPinkCabTransactionId(TEXT("ordinary-part")), EPinkCabTransactionType::PartPurchase, 700,
        EPinkCabEconomyPurpose::OrdinaryPurchase);
    TestEqual(TEXT("ordinary unaffordable part denied"), Ledger.Commit(Part), EPinkCabSettlementResult::InsufficientFunds);
    TestEqual(TEXT("ordinary part denial keeps balance"), Ledger.GetBalanceMinor(), int64(650));

    const auto Mandatory = FPinkCabEconomyTransaction::Debit(
        FPinkCabTransactionId(TEXT("mandatory-day")), EPinkCabTransactionType::Fine, 900,
        EPinkCabEconomyPurpose::MandatoryDayObligation);
    TestEqual(TEXT("mandatory obligation may use bounded debt"), Ledger.Commit(Mandatory), EPinkCabSettlementResult::Committed);
    TestEqual(TEXT("mandatory obligation result"), Ledger.GetBalanceMinor(), int64(-250));
    return true;
}

#endif
