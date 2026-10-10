#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Vehicle/PinkCabChaosNativeClutchJoint.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPinkCabNativeClutchJointCapacityTest,
    "PinkCab.Vehicle.Actuation.NativeJointCapacity",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPinkCabNativeClutchJointCapacityTest::RunTest(const FString& Parameters)
{
    int32 Cases = 0;
    double MaximumError = 0.0;
    for (const double Dt : {1.0/30.0, 1.0/60.0, 1.0/120.0})
    for (const double Coupling : {0.0, 0.25, 0.5, 0.949, 0.95, 0.999, 1.0})
    for (const double Speed : {100.0, 400.0, 890.0})
    for (const bool BackDrive : {false, true})
    {
        FPinkCabChaosNativeClutchJoint Joint;
        const double W0 = BackDrive ? 20.0 : Speed;
        const double W1 = BackDrive ? Speed : 20.0;
        const double I0 = 0.17, I1 = 0.0126, Time = 0.2;
        const double Capacity = 390.0 * Coupling;
        // Test oracle: dry capacity-limited lock, not the rejected viscous-drive law.
        const double Expected = FMath::Clamp((W0 - W1) / (Dt * (1.0 / I0 + 1.0 / I1)), -Capacity, Capacity);
        const auto Result = Joint.Solve(W0, W1, I0, I1, Capacity, Time, Dt);
        const double Error = FMath::Abs(Result.TransferredTorqueNm - Expected);
        MaximumError = FMath::Max(MaximumError, Error);
        TestTrue(TEXT("native joint returns valid shaft state"), Result.bValid);
        if (Error > 0.1)
            AddError(FString::Printf(TEXT("native bounded holding torque must not alias with shaft rotation: dt=%.8f coupling=%.3f omega=%.1f back=%d got=%.5f expected=%.5f"),
                Dt, Coupling, Speed, BackDrive, Result.TransferredTorqueNm, Expected));
        TestTrue(TEXT("native joint cannot add energy"), Result.DissipatedEnergyJ >= -0.001);
        TestTrue(TEXT("native joint has equal/opposite angular impulse"), FMath::Abs(Result.MomentumResidual) < 0.0001);
        const auto Open = Joint.Solve(Result.EngineOmega, W1, I0, I1, 0.0, Time, Dt);
        TestTrue(TEXT("opening loaded joint removes both drive and shaft correction"),
            Open.bValid && FMath::Abs(Open.TransferredTorqueNm) < 0.001
            && FMath::Abs(Open.EngineOmega - Result.EngineOmega) < 0.0001);
        ++Cases;
    }
    AddInfo(FString::Printf(TEXT("T6_NATIVE_JOINT_CASES=%d MAX_TORQUE_ERROR_NM=%.6f"), Cases, MaximumError));
    return true;
}
#endif
