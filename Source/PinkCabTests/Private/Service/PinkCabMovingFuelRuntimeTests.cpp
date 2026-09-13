#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Service/PinkCabMovingFuelSession.h"

namespace PinkCabMovingFuelRuntimeTests
{
FPinkCabMovingFuelPolicyInputs Policy()
{
    FPinkCabMovingFuelPolicyInputs P;
    P.QueuePolicyId = TEXT("queue.runtime");
    P.SettlementPolicyId = TEXT("settlement.runtime");
    P.InsufficientFundsPolicyId = TEXT("funds.runtime");
    P.TargetLongitudinalGapCm = 40.0f;
    P.GapToleranceCm = 10.0f;
    P.MaxConnectionSpeedKmh = 20.0f;
    return P;
}

FPinkCabRoadGraph Graph(FPinkCabLaneId& OutLane)
{
    FPinkCabRoadGraph G;
    OutLane = FPinkCabLaneId(TEXT("lane:service:runtime"));
    FPinkCabLogicalLane Lane;
    Lane.LaneId = OutLane;
    Lane.FromNode = FPinkCabRoadNodeId(TEXT("node:a"));
    Lane.ToNode = FPinkCabRoadNodeId(TEXT("node:b"));
    Lane.LengthCm = 1000.0;
    Lane.Layer = 0;
    G.AddLane(Lane);
    return G;
}FPinkCabVehicleTelemetry Telemetry(float Speed = 10.0f)
{
    FPinkCabVehicleTelemetry T;
    T.SpeedKmh = Speed;
    return T;
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabMovingFuelGraphEligibilityTest,
    "PinkCab.Service.MovingFuel.Runtime.GraphEligibility",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPinkCabMovingFuelGraphEligibilityTest::RunTest(const FString& Parameters)
{
    FPinkCabLaneId Lane;
    FPinkCabRoadGraph Graph = PinkCabMovingFuelRuntimeTests::Graph(Lane);
    FPinkCabMovingFuelSession Valid(PinkCabMovingFuelRuntimeTests::Policy());
    FPinkCabMovingFuelSession Invalid(PinkCabMovingFuelRuntimeTests::Policy());
    TestTrue(TEXT("graph lane connects"), Valid.TryConnect(Graph,
        PinkCabMovingFuelRuntimeTests::Telemetry(), 45.0f, Lane));
    TestFalse(TEXT("unknown lane rejected"), Invalid.TryConnect(Graph,
        PinkCabMovingFuelRuntimeTests::Telemetry(), 45.0f, FPinkCabLaneId(TEXT("lane:missing"))));
    TestEqual(TEXT("connected state"), Valid.GetState(), EPinkCabMovingFuelState::Connected);
    TestEqual(TEXT("lane retained"), Valid.GetServiceLaneId().Serialize(), Lane.Serialize());
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabMovingFuelToleranceLossTest,
    "PinkCab.Service.MovingFuel.Runtime.ToleranceLoss",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPinkCabMovingFuelToleranceLossTest::RunTest(const FString& Parameters)
{
    FPinkCabLaneId Lane;
    FPinkCabRoadGraph Graph = PinkCabMovingFuelRuntimeTests::Graph(Lane);
    FPinkCabMovingFuelSession Session(PinkCabMovingFuelRuntimeTests::Policy());
    FPinkCabFuelTank Tank(100.0f, 20.0f);
    TestTrue(TEXT("connect"), Session.TryConnect(Graph,
        PinkCabMovingFuelRuntimeTests::Telemetry(), 40.0f, Lane));
    TestTrue(TEXT("fueling begins"), Session.BeginFueling());
    TestFalse(TEXT("gap outside tolerance aborts"), Session.MaintainConnection(Graph,
        PinkCabMovingFuelRuntimeTests::Telemetry(), 80.0f));
    TestEqual(TEXT("aborted"), Session.GetState(), EPinkCabMovingFuelState::Aborted);
    TestEqual(TEXT("reason"), Session.GetAbortReason(), EPinkCabMovingFuelAbortReason::ToleranceLost);
    TestEqual(TEXT("aborted cannot credit"), Session.ApplySettledFuel(
        Tank, FPinkCabTransactionId(TEXT("fuel-after-gap")), 10.0f, EPinkCabSettlementResult::Committed),
        EPinkCabFuelCreditResult::InvalidState);
    TestEqual(TEXT("tank unchanged"), Tank.GetLiters(), 20.0f);
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabMovingFuelCollisionAbortTest,
    "PinkCab.Service.MovingFuel.Runtime.CollisionAbort",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPinkCabMovingFuelCollisionAbortTest::RunTest(const FString& Parameters)
{
    FPinkCabLaneId Lane;
    FPinkCabRoadGraph Graph = PinkCabMovingFuelRuntimeTests::Graph(Lane);
    FPinkCabMovingFuelSession Session(PinkCabMovingFuelRuntimeTests::Policy());
    TestTrue(TEXT("connect"), Session.TryConnect(Graph,
        PinkCabMovingFuelRuntimeTests::Telemetry(), 40.0f, Lane));
    TestTrue(TEXT("fueling begins"), Session.BeginFueling());
    TestTrue(TEXT("collision abort accepted"), Session.NotifyCollision());
    TestEqual(TEXT("collision reason"), Session.GetAbortReason(), EPinkCabMovingFuelAbortReason::Collision);
    TestFalse(TEXT("cannot complete after collision"), Session.Complete());
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabMovingFuelExplicitAbortTest,
    "PinkCab.Service.MovingFuel.Runtime.ExplicitAbort",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPinkCabMovingFuelExplicitAbortTest::RunTest(const FString& Parameters)
{
    FPinkCabMovingFuelSession Session(PinkCabMovingFuelRuntimeTests::Policy());
    TestTrue(TEXT("legacy connect setup"), Session.MarkConnected());
    TestTrue(TEXT("explicit abort"), Session.Abort());
    TestEqual(TEXT("reason"), Session.GetAbortReason(), EPinkCabMovingFuelAbortReason::Explicit);
    TestFalse(TEXT("abort is terminal"), Session.BeginFueling());
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabMovingFuelCompleteTest,
    "PinkCab.Service.MovingFuel.Runtime.Complete",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPinkCabMovingFuelCompleteTest::RunTest(const FString& Parameters)
{
    FPinkCabLaneId Lane;
    FPinkCabRoadGraph Graph = PinkCabMovingFuelRuntimeTests::Graph(Lane);
    FPinkCabMovingFuelSession Session(PinkCabMovingFuelRuntimeTests::Policy());
    TestTrue(TEXT("connect"), Session.TryConnect(Graph,
        PinkCabMovingFuelRuntimeTests::Telemetry(), 40.0f, Lane));
    TestTrue(TEXT("fueling begins"), Session.BeginFueling());
    TestTrue(TEXT("complete from fueling"), Session.Complete());
    TestEqual(TEXT("completed state"), Session.GetState(), EPinkCabMovingFuelState::Completed);
    TestEqual(TEXT("no abort reason"), Session.GetAbortReason(), EPinkCabMovingFuelAbortReason::None);
    TestFalse(TEXT("completed is terminal"), Session.Abort());
    return true;
}

#endif