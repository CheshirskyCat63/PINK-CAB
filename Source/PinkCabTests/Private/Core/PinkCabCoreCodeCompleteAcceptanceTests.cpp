#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Core/PinkCabCoreCodeCompleteScenario.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabCoreCodeCompleteGameplayChainTest,
    "PinkCab.Core.CodeComplete.Acceptance.GameplayChain",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabCoreCodeCompleteGameplayChainTest::RunTest(const FString& Parameters)
{
    FCoreCodeCompleteResult Result;
    TestTrue(TEXT("canonical scenario executes"),
        FCoreCodeCompleteScenario::RunCanonicalScenario(0xC0802ull, Result));
    TestTrue(TEXT("two successful ordinary fares"), Result.bTwoSuccessfulFares);
    TestTrue(TEXT("metered path exercised"), Result.bMeteredPath);
    TestTrue(TEXT("off-meter path exercised"), Result.bOffMeterPath);
    TestTrue(TEXT("exit blocked before payment then succeeds"), Result.bPaymentBeforeExit);
    TestTrue(TEXT("repeat eligibility promoted canonically"), Result.bRepeatEligible);
    TestTrue(TEXT("repeat order uses ordinary runtime"), Result.bRepeatOrderOrdinary);
    TestTrue(TEXT("decline leaves next-fare eligibility"), Result.bDeclineClean);
    TestTrue(TEXT("eligible evasion committed"), Result.bEvasionCommitted);
    TestTrue(TEXT("evaded fare cannot later become paid"), Result.bEvasionPaidConflict);
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabCoreCodeCompleteOwnerContinuityTest,
    "PinkCab.Core.CodeComplete.Acceptance.OwnerContinuity",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabCoreCodeCompleteOwnerContinuityTest::RunTest(const FString& Parameters)
{
    FCoreCodeCompleteResult Result;
    TestTrue(TEXT("canonical scenario executes"),
        FCoreCodeCompleteScenario::RunCanonicalScenario(0xC0802ull, Result));
    TestTrue(TEXT("Neural state survives absence and restore"), Result.bNeuralContinuity);
    TestTrue(TEXT("enforcement fine commits once"), Result.bFineCommittedOnce);
    TestTrue(TEXT("service operation applies"), Result.bServiceApplied);
    TestTrue(TEXT("moving fuel applies"), Result.bMovingFuelApplied);
    TestTrue(TEXT("L1 traversal completes"), Result.bL1Completed);
    TestTrue(TEXT("L2 suspended-bus window executes"), Result.bL2BusWindow);
    TestTrue(TEXT("L2 metro returns to road"), Result.bL2MetroRoundTrip);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabCoreCodeCompleteRecoveryContinuityTest,
    "PinkCab.Core.CodeComplete.Acceptance.RecoveryContinuity",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPinkCabCoreCodeCompleteRecoveryContinuityTest::RunTest(const FString& Parameters)
{
    FCoreCodeCompleteResult Result;
    TestTrue(TEXT("canonical scenario executes"),
        FCoreCodeCompleteScenario::RunCanonicalScenario(0xC0802ull, Result));
    TestTrue(TEXT("aggregate live-owner restore succeeds"), Result.bSnapshotRestored);
    TestTrue(TEXT("committed IDs replay as no-op"), Result.bReplayNoOps);
    TestTrue(TEXT("terminal recovery fails fare and unloads group"), Result.bTerminalRecovery);
    TestTrue(TEXT("terminal recovery preserves vehicle damage"), Result.bDamagePreserved);
    TestTrue(TEXT("next workday resets only scoped state"), Result.bWorkdayResetScoped);
    TestTrue(TEXT("integrated signature is non-zero"), Result.IntegratedSignature != 0ull);
    return true;
}

#endif
