#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Vehicle/PinkCabWallrideController.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabWallrideMassTimeoutTest,
    "PinkCab.Vehicle.Wallride.MassTimeout",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabWallrideMassTimeoutTest::RunTest(const FString& Parameters)
{
    TestEqual(TEXT("lighter legal state caps at five seconds"),
        FPinkCabWallrideController::ComputeResidualTimeoutSeconds(1500.0f), 5.0f);
    TestEqual(TEXT("reference mass is five seconds"),
        FPinkCabWallrideController::ComputeResidualTimeoutSeconds(1657.0f), 5.0f);
    TestTrue(TEXT("midpoint interpolates to 4.5 seconds"),
        FMath::IsNearlyEqual(FPinkCabWallrideController::ComputeResidualTimeoutSeconds(1882.0f), 4.5f));
    TestEqual(TEXT("declared max load is four seconds"),
        FPinkCabWallrideController::ComputeResidualTimeoutSeconds(2107.0f), 4.0f);
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabWallrideResidualTest,
    "PinkCab.Vehicle.Wallride.ResidualCountdown",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabWallrideResidualTest::RunTest(const FString& Parameters)
{
    FPinkCabWallrideController Controller;
    Controller.Update(true, 0.1f, 1657.0f);
    TestEqual(TEXT("valid contact enters contact"), Controller.GetState(), EPinkCabWallrideState::Contact);
    Controller.Update(false, 1.0f, 1657.0f);
    TestEqual(TEXT("contact loss enters residual"), Controller.GetState(), EPinkCabWallrideState::Residual);
    TestTrue(TEXT("one second consumed"), FMath::IsNearlyEqual(Controller.GetResidualSeconds(), 4.0f));
    Controller.Update(false, 4.1f, 1657.0f);
    TestEqual(TEXT("residual expires"), Controller.GetState(), EPinkCabWallrideState::Expired);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabWallrideReacquisitionTest,
    "PinkCab.Vehicle.Wallride.ReacquisitionReset",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPinkCabWallrideReacquisitionTest::RunTest(const FString& Parameters)
{
    FPinkCabWallrideController Controller;
    Controller.Update(true, 0.1f, 1657.0f);
    Controller.Update(false, 2.0f, 1657.0f);
    TestTrue(TEXT("three seconds remain"), FMath::IsNearlyEqual(Controller.GetResidualSeconds(), 3.0f));

    Controller.Update(true, 0.24f, 1657.0f);
    Controller.Update(false, 0.0f, 1657.0f);
    TestTrue(TEXT("short reacquisition does not refresh"), FMath::IsNearlyEqual(Controller.GetResidualSeconds(), 3.0f));

    Controller.Update(true, 0.25f, 1657.0f);
    TestTrue(TEXT("continuous quarter-second contact refreshes"), FMath::IsNearlyEqual(Controller.GetResidualSeconds(), 5.0f));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabWallrideConstraintBoundaryTest,
    "PinkCab.Vehicle.Wallride.ConstraintBoundary",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabWallrideConstraintBoundaryTest::RunTest(const FString& Parameters)
{
    FPinkCabWallrideController Controller;
    TestFalse(TEXT("detached emits no request"), Controller.BuildConstraintRequest(1657.0f).bRequested);
    Controller.Update(true, 0.1f, 1657.0f);
    const FPinkCabWallrideConstraintRequest Request = Controller.BuildConstraintRequest(1657.0f);
    TestTrue(TEXT("contact emits external request only"), Request.bRequested);
    TestEqual(TEXT("request carries authoritative mass"), Request.TotalMassKg, 1657.0f);
    TestFalse(TEXT("foundation never reports force applied"), Request.bForceApplied);
    return true;
}

#endif
