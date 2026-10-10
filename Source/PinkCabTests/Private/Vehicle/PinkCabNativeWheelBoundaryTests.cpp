#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "WheelSystem.h"
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPinkCabNativeWheelBoundaryTest,
    "PinkCab.Vehicle.Actuation.NativeWheelRollingBoundary", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPinkCabNativeWheelBoundaryTest::RunTest(const FString& Parameters)
{
    for (float TorqueNm : {0.0f, 200.0f, 2000.0f})
    for (bool Grounded : {false, true})
    {
        Chaos::FSimpleWheelConfig Config;
        Config.WheelRadius = 32.0f;
        Config.FrictionMultiplier = 1.0f;
        Config.EngineEnabled = true;
        Chaos::FSimpleWheelSim Wheel(&Config);
        Wheel.SetMassPerWheel(414.25f);
        Wheel.SetMaxOmega(2000.0f);
        Wheel.SetVehicleGroundSpeed(FVector(700.0, 0, 0));
        Wheel.SetWheelLoadForce(Grounded ? 450000.0f : 0.0f);
        Wheel.SetSurfaceFriction(1.0f);
        Wheel.SetAngularVelocity(700.0f / 32.0f);
        Wheel.SetDriveTorque(Chaos::TorqueMToCm(TorqueNm));
        Wheel.SetBrakeTorque(0.0f);
        const double BeforeAngle = Wheel.GetAngularPosition();
        Wheel.Simulate(1.0f / 60.0f);
        TestTrue(TEXT("native wheel angle uses the same end-step angular rate as the clutch power port"),
            FMath::Abs(Wheel.GetAngularPosition() - BeforeAngle - Wheel.GetAngularVelocity() / 60.0) < 0.0001);
        const float Rolling = Wheel.GetRoadSpeed() / Wheel.GetEffectiveRadius();
        AddInfo(FString::Printf(TEXT("T6_WHEEL_BOUNDARY ground=%d torque=%.1f omega=%.6f rolling=%.6f clipping=%d force=%.3f spin=%.6f inertia=%.3f"),
            Grounded, TorqueNm, Wheel.GetAngularVelocity(), Rolling, Wheel.bClipping,
            Wheel.GetForceFromFriction().X, Wheel.Spin, Wheel.Inertia));
        if (Grounded && TorqueNm < 1000.0f)
            TestTrue(TEXT("native gripping wheel follows supplied current ground velocity"), FMath::IsNearlyEqual(Wheel.GetAngularVelocity(), Rolling, 0.001f));
        if (!Grounded && TorqueNm > 0.0f)
            TestTrue(TEXT("native airborne wheel is dynamically accelerated by drive torque"), Wheel.GetAngularVelocity() > Rolling);
    }
    return true;
}
#endif
