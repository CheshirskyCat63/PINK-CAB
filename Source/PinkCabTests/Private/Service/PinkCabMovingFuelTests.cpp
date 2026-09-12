#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Service/PinkCabFuelTank.h"
#include "Service/PinkCabMovingFuelSession.h"
#include "Economy/PinkCabEconomyLedger.h"
static FPinkCabMovingFuelPolicyInputs MakeFuelPolicyFixture(const TCHAR* Suffix)
{
    FPinkCabMovingFuelPolicyInputs Policy;
    Policy.QueuePolicyId = FName(*FString::Printf(TEXT("fixture_queue_%s"), Suffix));
    Policy.SettlementPolicyId = FName(*FString::Printf(TEXT("fixture_settlement_%s"), Suffix));
    Policy.InsufficientFundsPolicyId = FName(*FString::Printf(TEXT("fixture_funds_%s"), Suffix));
    Policy.TargetLongitudinalGapCm = 40.0f;
    Policy.GapToleranceCm = 15.0f;
    Policy.MaxConnectionSpeedKmh = 12.0f;
    return Policy;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabFuelTankExactlyOnceTest,
    "PinkCab.Service.MovingFuel.FuelTankExactlyOnce",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabFuelTankExactlyOnceTest::RunTest(const FString& Parameters)
{
    FPinkCabFuelTank Tank(100.0f, 20.0f);
    const FPinkCabTransactionId Tx(TEXT("fuel-fixture-1"));

    TestEqual(TEXT("first credit applies"), Tank.CreditFuelOnce(Tx, 15.0f), EPinkCabFuelCreditResult::Applied);
    TestEqual(TEXT("fuel increased once"), Tank.GetLiters(), 35.0f);
    TestEqual(TEXT("same transaction is duplicate"), Tank.CreditFuelOnce(Tx, 15.0f), EPinkCabFuelCreditResult::Duplicate);
    TestEqual(TEXT("duplicate does not add fuel"), Tank.GetLiters(), 35.0f);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabMovingFuelEligibilityTest,
    "PinkCab.Service.MovingFuel.Eligibility",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabMovingFuelEligibilityTest::RunTest(const FString& Parameters)
{
    FPinkCabMovingFuelPolicyInputs Policy;
    Policy.QueuePolicyId = FName(TEXT("fixture_queue"));
    Policy.SettlementPolicyId = FName(TEXT("fixture_settlement"));
    Policy.InsufficientFundsPolicyId = FName(TEXT("fixture_funds"));
    Policy.TargetLongitudinalGapCm = 40.0f;
    Policy.GapToleranceCm = 15.0f;
    Policy.MaxConnectionSpeedKmh = 12.0f;

    FPinkCabVehicleTelemetry Telemetry;
    Telemetry.SpeedKmh = 10.0f;
    const FPinkCabLaneId Lane(TEXT("lane:fixture"));

    FPinkCabMovingFuelSession Session(Policy);
    TestTrue(TEXT("caller-supplied envelope allows connection"), Session.CanConnect(Telemetry, 50.0f, Lane));
    Telemetry.SpeedKmh = 13.0f;
    TestFalse(TEXT("speed outside caller envelope rejects connection"), Session.CanConnect(Telemetry, 50.0f, Lane));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabMovingFuelStateGateTest,
    "PinkCab.Service.MovingFuel.StateGate",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabMovingFuelStateGateTest::RunTest(const FString& Parameters)
{
    FPinkCabMovingFuelPolicyInputs Policy = MakeFuelPolicyFixture(TEXT("state-gate"));
    FPinkCabMovingFuelSession Session(Policy);
    FPinkCabFuelTank Tank(100.0f, 20.0f);
    const FPinkCabTransactionId Tx(TEXT("fuel-state-gate"));

    TestEqual(TEXT("no fuel before connection"),
        Session.ApplySettledFuel(Tank, Tx, 5.0f, EPinkCabSettlementResult::Committed),
        EPinkCabFuelCreditResult::InvalidState);
    Session.MarkConnected();
    TestEqual(TEXT("connected but not fueling still blocked"),
        Session.ApplySettledFuel(Tank, Tx, 5.0f, EPinkCabSettlementResult::Committed),
        EPinkCabFuelCreditResult::InvalidState);
    Session.BeginFueling();
    TestEqual(TEXT("fueling permits committed credit"),
        Session.ApplySettledFuel(Tank, Tx, 5.0f, EPinkCabSettlementResult::Committed),
        EPinkCabFuelCreditResult::Applied);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabMovingFuelSettlementReplayTest,
    "PinkCab.Service.MovingFuel.SettlementReplay",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabMovingFuelSettlementReplayTest::RunTest(const FString& Parameters)
{
    FPinkCabEconomyLedger Ledger(5000, 0);
    FPinkCabFuelTank Tank(100.0f, 20.0f);
    FPinkCabMovingFuelSession Session(MakeFuelPolicyFixture(TEXT("replay")));
    Session.MarkConnected();
    Session.BeginFueling();

    const FPinkCabTransactionId Tx(TEXT("fuel-replay"));
    const FPinkCabEconomyTransaction Purchase = FPinkCabEconomyTransaction::Debit(
        Tx, EPinkCabTransactionType::FuelPurchase, 1200, EPinkCabEconomyPurpose::OrdinaryPurchase);

    TestEqual(TEXT("economy commits once"), Ledger.Commit(Purchase), EPinkCabSettlementResult::Committed);
    TestEqual(TEXT("fuel applies once"), Session.ApplySettledFuel(Tank, Tx, 12.0f, EPinkCabSettlementResult::Committed), EPinkCabFuelCreditResult::Applied);
    TestEqual(TEXT("economy retry is duplicate"), Ledger.Commit(Purchase), EPinkCabSettlementResult::Duplicate);
    TestEqual(TEXT("fuel retry is duplicate"), Session.ApplySettledFuel(Tank, Tx, 12.0f, EPinkCabSettlementResult::Duplicate), EPinkCabFuelCreditResult::Duplicate);
    TestEqual(TEXT("balance debited once"), Ledger.GetBalanceMinor(), static_cast<int64>(3800));
    TestEqual(TEXT("tank credited once"), Tank.GetLiters(), 32.0f);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabMovingFuelDeniedSettlementTest,
    "PinkCab.Service.MovingFuel.DeniedSettlement",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabMovingFuelDeniedSettlementTest::RunTest(const FString& Parameters)
{
    FPinkCabFuelTank Tank(100.0f, 20.0f);
    FPinkCabMovingFuelSession Session(MakeFuelPolicyFixture(TEXT("denied")));
    Session.MarkConnected();
    Session.BeginFueling();
    const FPinkCabTransactionId Tx(TEXT("fuel-denied"));

    TestEqual(TEXT("denied settlement cannot create fuel"),
        Session.ApplySettledFuel(Tank, Tx, 12.0f, EPinkCabSettlementResult::InsufficientFunds),
        EPinkCabFuelCreditResult::SettlementNotCommitted);
    TestEqual(TEXT("tank unchanged"), Tank.GetLiters(), 20.0f);
    return true;
}

#endif

