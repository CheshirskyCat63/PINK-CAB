#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Vehicle/PinkCabChaosNativeClutchJoint.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPinkCabNativeClutchHoldingTest,
    "PinkCab.Vehicle.Actuation.NativeClutchHolding",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPinkCabNativeClutchHoldingTest::RunTest(const FString& Parameters)
{
    int32 Cases = 0;
    double WorstSlipRpm = 0.0;
    for (double Dt : {1.0 / 30.0, 1.0 / 60.0, 1.0 / 120.0})
    for (double Speed : {100.0, 400.0, 890.0})
    for (double Direction : {-1.0, 1.0})
    for (double Coupling : {0.25, 0.949, 1.0})
    {
        FPinkCabChaosNativeClutchJoint Joint;
        constexpr double EngineInertia = 0.17, ShaftInertia = 0.0126;
        const double Capacity = 390.0 * Coupling;
        const double AppliedTorque = 0.4 * Capacity * Direction;
        double Engine = Speed, Shaft = Speed, MaxSlip = 0.0;
        bool Valid = true, Passive = true, Bounded = true;
        for (int32 Step = 0; Step < FMath::RoundToInt(3.0 / Dt); ++Step)
        {
            // Test-only equal and opposite external loading; no vehicle velocity writes.
            const double FreeEngine = Engine + AppliedTorque * Dt / EngineInertia;
            const double FreeShaft = Shaft - AppliedTorque * Dt / ShaftInertia;
            const auto Result = Joint.Solve(FreeEngine, FreeShaft, EngineInertia,
                ShaftInertia, Capacity, 0.2, Dt);
            Valid &= Result.bValid;
            Passive &= Result.DissipatedEnergyJ >= -0.01 && FMath::Abs(Result.MomentumResidual) < 0.001;
            Bounded &= FMath::Abs(Result.TransferredTorqueNm) <= Capacity + 0.01;
            Engine = Result.EngineOmega;
            Shaft = FreeShaft + Result.TransferredTorqueNm * Dt / ShaftInertia;
            if (Step * Dt >= 2.0) MaxSlip = FMath::Max(MaxSlip, FMath::Abs(Engine - Shaft) * 60.0 / (2.0 * PI));
        }
        WorstSlipRpm = FMath::Max(WorstSlipRpm, MaxSlip);
        TestTrue(TEXT("native holding is valid, passive and capacity limited"), Valid && Passive && Bounded);
        if (MaxSlip > 25.0)
            AddError(FString::Printf(TEXT("sub-capacity load must hold without sustained slip: dt=%.6f speed=%.1f direction=%.0f c=%.3f slip=%.3fRPM"), Dt, Speed, Direction, Coupling, MaxSlip));
        const auto Open = Joint.Solve(Engine, Shaft, EngineInertia, ShaftInertia, 0.0, 0.2, Dt);
        TestTrue(TEXT("release clears holding without shaft impulse"), Open.bValid
            && FMath::Abs(Open.TransferredTorqueNm) < 0.001 && FMath::Abs(Open.EngineOmega - Engine) < 0.001);
        ++Cases;
    }
    AddInfo(FString::Printf(TEXT("T6_NATIVE_HOLDING cases=%d worst_slip_rpm=%.6f"), Cases, WorstSlipRpm));
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPinkCabNativeRoadShaftHoldingTest,
    "PinkCab.Vehicle.Actuation.NativeRoadShaftHolding",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPinkCabNativeRoadShaftHoldingTest::RunTest(const FString& Parameters)
{
    for (double Dt : {1.0/30.0, 1.0/60.0, 1.0/120.0})
    for (double Speed : {-890.0, -100.0, 100.0, 890.0})
    for (double ExternalTorque : {-200.0, -20.0, 20.0, 200.0})
    {
        FPinkCabChaosNativeClutchJoint Joint;
        const double FreeEngine = Speed + ExternalTorque * Dt / 0.17;
        const auto Result = Joint.Solve(FreeEngine, Speed, 0.17, 0.0126, 390.0, 0.2, Dt, true);
        TestTrue(TEXT("prescribed road shaft retains its actual nonzero speed"),
            Result.bValid && FMath::Abs(Result.EngineOmega - Speed) < 0.001);
        TestTrue(TEXT("holding reaction balances external engine torque"), FMath::Abs(Result.TransferredTorqueNm - ExternalTorque) < 0.01);
        TestTrue(TEXT("holding has no energy source"), Result.DissipatedEnergyJ >= -0.01);
    }
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPinkCabNativeClutchOverloadTest,
    "PinkCab.Vehicle.Actuation.NativeClutchOverload",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPinkCabNativeClutchOverloadTest::RunTest(const FString& Parameters)
{
    for (double Dt : {1.0/30.0, 1.0/60.0, 1.0/120.0})
    for (double Capacity : {97.5, 390.0})
    for (double Direction : {-1.0, 1.0})
    {
        FPinkCabChaosNativeClutchJoint Joint;
        const double Engine = 350.0 + Direction * 2.0 * Capacity * Dt / 0.17;
        const double Shaft = 350.0 - Direction * 2.0 * Capacity * Dt / 0.0126;
        const auto Result = Joint.Solve(Engine, Shaft, 0.17, 0.0126, Capacity, 0.2, Dt);
        const double DrivenAfter = Shaft + Result.TransferredTorqueNm * Dt / 0.0126;
        TestTrue(TEXT("overload slips instead of becoming an unlimited rigid connection"),
            Result.bValid && (Result.EngineOmega - DrivenAfter) * Direction > 0.0);
        TestTrue(TEXT("overload transmits only the current pressure-limited torque"),
            FMath::Abs(Result.TransferredTorqueNm - Direction * Capacity) < 0.01);
        TestTrue(TEXT("sliding remains passive and transfers equal opposite impulse"),
            Result.DissipatedEnergyJ >= -0.01 && FMath::Abs(Result.MomentumResidual) < 0.001);
    }
    return true;
}
#endif
