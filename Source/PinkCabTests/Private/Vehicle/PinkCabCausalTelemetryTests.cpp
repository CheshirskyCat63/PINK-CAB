#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Vehicle/PinkCabCausalTelemetry.h"
#include "Vehicle/PinkCabEngineActuationResolver.h"
#include "Vehicle/PinkCabVehicleControlRuntime.h"

namespace
{
FPinkCabVehicleTelemetry CausalTelemetryFixture(float SpeedKmh = 0.0f, float EngineRpm = 750.0f)
{
    FPinkCabVehicleTelemetry Result;
    Result.SpeedKmh = SpeedKmh;
    Result.EngineRpm = EngineRpm;
    return Result;
}

FPinkCabVehicleControlTickInput CausalDigital(
    bool bClutch,
    bool bBrake,
    bool bThrottle,
    float SteeringMouseX = 0.0f,
    float DeltaSeconds = 1.0f / 60.0f)
{
    FPinkCabVehicleControlTickInput Input;
    Input.Frame = FPinkCabVehicleInputFrame::FromDigital(
        false, bClutch, bBrake, bThrottle);
    Input.SteeringMouseX = SteeringMouseX;
    Input.DeltaSeconds = DeltaSeconds;
    return Input;
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabCausalTelemetryTracePreservesFirstFramesTest,
    "PinkCab.Vehicle.Physics.Telemetry.TracePreservesFirstFrames",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabCausalTelemetryTracePreservesFirstFramesTest::RunTest(const FString& Parameters)
{
    FPinkCabCausalTelemetryTrace Trace(2);

    FPinkCabCausalTelemetryFrame A;
    A.TimestampSeconds = 10.0;
    FPinkCabCausalTelemetryFrame B;
    B.TimestampSeconds = 11.0;
    FPinkCabCausalTelemetryFrame C;
    C.TimestampSeconds = 12.0;

    TestTrue(TEXT("first frame records"), Trace.Record(A));
    TestTrue(TEXT("second frame records"), Trace.Record(B));
    TestFalse(TEXT("full trace refuses overwrite"), Trace.Record(C));
    TestEqual(TEXT("bounded trace keeps original count"), Trace.GetNum(), 2);
    TestEqual(TEXT("overflow is counted"), Trace.GetDroppedFrameCount(), 1);
    TestEqual(TEXT("first frame is preserved"), Trace.GetFrames()[0].TimestampSeconds, 10.0);
    TestEqual(TEXT("second frame is preserved"), Trace.GetFrames()[1].TimestampSeconds, 11.0);
    TestEqual(TEXT("first assigned sequence is zero"), Trace.GetFrames()[0].Sequence, 0ull);
    TestEqual(TEXT("second assigned sequence increments"), Trace.GetFrames()[1].Sequence, 1ull);

    Trace.Reset();
    TestEqual(TEXT("reset clears frames"), Trace.GetNum(), 0);
    TestEqual(TEXT("reset clears drop count"), Trace.GetDroppedFrameCount(), 0);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabCausalControlStageTelemetryTest,
    "PinkCab.Vehicle.Physics.Telemetry.ControlStages",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabCausalControlStageTelemetryTest::RunTest(const FString& Parameters)
{
    FPinkCabVehicleControlRuntime Runtime;
    FPinkCabCockpitState Cockpit;
    FPinkCabVehicleHealthState Health;
    Cockpit.StartEngine();

    FPinkCabVehicleControlTickInput Input =
        CausalDigital(true, false, true, 320.0f, 0.20f);
    Input.WheelSteps = 1;
    Runtime.Update(Input, CausalTelemetryFixture(12.0f, 1200.0f), Cockpit, Health);

    const FPinkCabCausalControlTelemetry& Telemetry =
        Runtime.GetCausalControlTelemetry();

    TestEqual(TEXT("raw digital throttle is captured before dosing"),
        Telemetry.RawThrottle01, 1.0f);
    TestTrue(TEXT("prepared throttle captures the dosed/smoothed command"),
        Telemetry.PreparedThrottle01 > 0.0f && Telemetry.PreparedThrottle01 < 1.0f);
    TestTrue(TEXT("post-drivetrain throttle is bounded"),
        Telemetry.PostDrivetrainThrottle01 >= 0.0f
            && Telemetry.PostDrivetrainThrottle01 <= Telemetry.PreparedThrottle01);
    TestEqual(TEXT("raw steering mouse delta is retained"),
        Telemetry.RawSteeringMouseDelta, 320.0f);
    TestTrue(TEXT("virtual cursor records pre-target integration"),
        FMath::Abs(Telemetry.SteeringVirtualCursor) > 0.0f);
    TestTrue(TEXT("shaped steering target is recorded"),
        FMath::Abs(Telemetry.SteeringTarget) > 0.0f);
    TestTrue(TEXT("final steering command is recorded"),
        FMath::Abs(Telemetry.FinalSteeringCommand) > 0.0f);
    TestTrue(TEXT("expected coupled rpm is non-negative"),
        Telemetry.ExpectedCoupledRpm >= 0.0f);
    TestTrue(TEXT("drivetrain torque capacity is bounded"),
        Telemetry.DrivetrainTorqueCapacity01 >= 0.0f
            && Telemetry.DrivetrainTorqueCapacity01 <= 1.0f);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabCausalFrameAvailabilityContractTest,
    "PinkCab.Vehicle.Physics.Telemetry.AvailabilityContract",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabCausalFrameAvailabilityContractTest::RunTest(const FString& Parameters)
{
    FPinkCabCausalWheelTelemetry Wheel;
    TestFalse(TEXT("exact normal load is not fabricated by default"), Wheel.bHasNormalLoad);
    TestFalse(TEXT("longitudinal tire force is not fabricated by default"), Wheel.bHasLongitudinalForce);
    TestFalse(TEXT("lateral tire force is not fabricated by default"), Wheel.bHasLateralForce);

    FPinkCabCausalTelemetryFrame Frame;
    Frame.bCombustionPermission = false;
    Frame.RequestedEngineTorqueAfterLimiterHealthNm = 140.0f;
    Frame.RefreshDerivedFields();

    TestEqual(TEXT("permission-gated available torque is zero while engine permission is false"),
        Frame.PermissionGatedAvailableEngineTorqueNm, 0.0f);
    return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabCausalActuationResolverTest,
    "PinkCab.Vehicle.Physics.Telemetry.ActuationResolver",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabCausalActuationResolverTest::RunTest(const FString& Parameters)
{
    FPinkCabEngineActuationInput Input;
    Input.HealthClampedControlThrottle01 = 0.25f;
    Input.EngineRpm = 3000.0f;
    Input.MaxRpm = 8500.0f;
    Input.EngineTorqueCurveNm = 220.0f;

    const FPinkCabEngineActuationResult Normal =
        FPinkCabEngineActuationResolver::Resolve(Input);
    TestTrue(TEXT("mechanical throttle curve is captured before limiter"),
        Normal.EngineThrottlePreLimiter01 > Input.HealthClampedControlThrottle01);
    TestEqual(TEXT("below limiter final throttle equals pre-limiter throttle"),
        Normal.EngineThrottleFinal01, Normal.EngineThrottlePreLimiter01);
    TestTrue(TEXT("available torque is torque curve times final throttle"),
        FMath::IsNearlyEqual(
            Normal.RequestedEngineTorqueAfterLimiterHealthNm,
            Input.EngineTorqueCurveNm * Normal.EngineThrottleFinal01,
            1.0e-5f));

    Input.EngineRpm = Input.MaxRpm;
    const FPinkCabEngineActuationResult Limited =
        FPinkCabEngineActuationResolver::Resolve(Input);
    TestEqual(TEXT("hard limiter cuts final engine throttle"), Limited.EngineThrottleFinal01, 0.0f);
    TestEqual(TEXT("hard limiter cuts requested engine torque"), Limited.RequestedEngineTorqueAfterLimiterHealthNm, 0.0f);

    Input.HealthClampedControlThrottle01 = 0.0f;
    Input.EngineRpm = 1000.0f;
    const FPinkCabEngineActuationResult Zero =
        FPinkCabEngineActuationResolver::Resolve(Input);
    TestEqual(TEXT("zero health-clamped control remains zero before limiter"), Zero.EngineThrottlePreLimiter01, 0.0f);
    TestEqual(TEXT("zero control has zero requested combustion torque"), Zero.RequestedEngineTorqueAfterLimiterHealthNm, 0.0f);
    return true;
}

#endif
