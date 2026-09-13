#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Vehicle/PinkCabL1TraversalState.h"
#include "World/PinkCabL1RouteHookRuntime.h"
#include "World/PinkCabRouteService.h"

namespace PinkCabL1RouteHookTests
{
FPinkCabLogicalLane Lane(
    const TCHAR* Id, const TCHAR* From, const TCHAR* To, double Length = 10.0)
{
    FPinkCabLogicalLane Result;
    Result.LaneId = FPinkCabLaneId(Id);
    Result.FromNode = FPinkCabRoadNodeId(From);
    Result.ToNode = FPinkCabRoadNodeId(To);
    Result.LengthCm = Length;
    Result.Layer = 0;
    return Result;
}

FPinkCabL1TraversalInput Contact(const FPinkCabL1RouteHook& Hook, float MassKg)
{
    FPinkCabL1TraversalInput Input;
    Input.bHasContact = true;
    Input.ContactKind = Hook.Kind;
    Input.ContactId = Hook.ContactId;
    Input.TotalMassKg = MassKg;
    Input.DeltaSeconds = 0.25f;
    return Input;
}
bool BuildFixture(
    FPinkCabRoadGraph& Graph,
    FPinkCabVerticalContactRegistry& Contacts,
    FPinkCabL1RouteHookRuntime& Hooks,
    FPinkCabCityIdentity& City)
{
    City = FPinkCabCityIdentity::Create(TEXT("VERT-ROUTE"), TEXT("gen-1"), TEXT("content-1"));
    Graph.AddLane(Lane(TEXT("lane:wall"), TEXT("n0"), TEXT("n1")));
    Graph.AddLane(Lane(TEXT("lane:freight"), TEXT("n1"), TEXT("n2"), 100.0));
    Graph.AddLane(Lane(TEXT("lane:gap"), TEXT("n1"), TEXT("n3"), 10.0));
    Graph.AddLane(Lane(TEXT("lane:gap-return"), TEXT("n3"), TEXT("n2"), 10.0));
    Graph.AddLane(Lane(TEXT("lane:receive"), TEXT("n2"), TEXT("n4")));

    auto Add = [&](const TCHAR* LaneId, EPinkCabVerticalContactKind Kind,
                   const TCHAR* Semantic, int32 Order)
    {
        FString ContactId;
        if (!Contacts.TryAdd(City, FPinkCabLaneId(LaneId), Kind, Semantic, 0, ContactId)) return false;
        FString HookId;
        return Hooks.TryAddHook(Graph, Contacts, ContactId, Order, HookId);
    };
    return Add(TEXT("lane:wall"), EPinkCabVerticalContactKind::WallLeft, TEXT("wall"), 0)
        && Add(TEXT("lane:freight"), EPinkCabVerticalContactKind::FreightCeiling, TEXT("freight"), 0)
        && Add(TEXT("lane:gap"), EPinkCabVerticalContactKind::PoplarGapHook, TEXT("gap"), 0)
        && Add(TEXT("lane:receive"), EPinkCabVerticalContactKind::ReceivingStrip, TEXT("receive"), 0);
}
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabL1RouteHookOrderingTest,
    "PinkCab.Vertical.L1.RouteHooks.Ordering",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabL1RouteHookOrderingTest::RunTest(const FString& Parameters)
{
    FPinkCabRoadGraph Graph;
    FPinkCabVerticalContactRegistry Contacts;
    FPinkCabL1RouteHookRuntime Hooks;
    FPinkCabCityIdentity City;
    TestTrue(TEXT("fixture builds"), PinkCabL1RouteHookTests::BuildFixture(Graph, Contacts, Hooks, City));

    FPinkCabCityDeltaState Deltas;
    FPinkCabRouteRequest Request;
    Request.StartLaneId = FPinkCabLaneId(TEXT("lane:wall"));
    Request.GoalLaneId = FPinkCabLaneId(TEXT("lane:receive"));
    Request.MaxVisitedNodes = 16;
    FPinkCabRoute Route;
    TestTrue(TEXT("route found"), FPinkCabRouteService::FindRoute(Graph, Request, Deltas, Route));

    TArray<FPinkCabL1RouteHook> Ordered;
    TestTrue(TEXT("hooks resolve for route"), Hooks.BuildOrderedHooksForRoute(Route, Ordered));
    TestEqual(TEXT("shortest route has three hooks"), Ordered.Num(), 3);
    TestEqual(TEXT("first wall"), Ordered[0].Kind, EPinkCabVerticalContactKind::WallLeft);
    TestEqual(TEXT("second gap"), Ordered[1].Kind, EPinkCabVerticalContactKind::PoplarGapHook);
    TestEqual(TEXT("last receiving"), Ordered[2].Kind, EPinkCabVerticalContactKind::ReceivingStrip);
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabL1RouteHookMassFixturesTest,
    "PinkCab.Vertical.L1.RouteHooks.MassFixtures",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabL1RouteHookMassFixturesTest::RunTest(const FString& Parameters)
{
    FPinkCabRoadGraph Graph;
    FPinkCabVerticalContactRegistry Contacts;
    FPinkCabL1RouteHookRuntime Hooks;
    FPinkCabCityIdentity City;
    TestTrue(TEXT("fixture builds"), PinkCabL1RouteHookTests::BuildFixture(Graph, Contacts, Hooks, City));

    FPinkCabCityDeltaState Deltas;
    FString ClosureId;
    TestTrue(TEXT("gap can close"), Deltas.TryAddLaneClosure(
        City, FPinkCabLaneId(TEXT("lane:gap")), TEXT("mass-fixture-direct"), ClosureId));
    FPinkCabRouteRequest Request;
    Request.StartLaneId = FPinkCabLaneId(TEXT("lane:wall"));
    Request.GoalLaneId = FPinkCabLaneId(TEXT("lane:receive"));
    Request.MaxVisitedNodes = 16;
    FPinkCabRoute Route;
    TestTrue(TEXT("direct freight route found"), FPinkCabRouteService::FindRoute(Graph, Request, Deltas, Route));
    TArray<FPinkCabL1RouteHook> Ordered;
    TestTrue(TEXT("direct hooks resolve"), Hooks.BuildOrderedHooksForRoute(Route, Ordered));
    TestEqual(TEXT("wall freight receive"), Ordered.Num(), 3);

    const float Masses[] = {1500.0f, 1657.0f, 1882.0f, 2107.0f};
    for (float MassKg : Masses)
    {
        FPinkCabL1TraversalState State;
        State.Step(PinkCabL1RouteHookTests::Contact(Ordered[0], MassKg));
        FPinkCabL1TraversalInput Air;
        Air.TotalMassKg = MassKg;
        Air.DeltaSeconds = 0.5f;
        FPinkCabL1TraversalResult Result = State.Step(Air);
        TestEqual(TEXT("mass fixture enters residual"), Result.Phase,
            EPinkCabL1TraversalPhase::WallResidual);
        Result = State.Step(PinkCabL1RouteHookTests::Contact(Ordered[0], MassKg));
        TestEqual(TEXT("mass fixture reacquires wall"), Result.Phase,
            EPinkCabL1TraversalPhase::WallContact);
        Result = State.Step(PinkCabL1RouteHookTests::Contact(Ordered[1], MassKg));
        TestEqual(TEXT("mass fixture reaches freight"), Result.Phase,
            EPinkCabL1TraversalPhase::FreightContact);
        Result = State.Step(PinkCabL1RouteHookTests::Contact(Ordered[2], MassKg));
        TestEqual(TEXT("mass fixture completes"), Result.Phase,
            EPinkCabL1TraversalPhase::Completed);
        TestTrue(TEXT("completion remains explicit"), Result.bCompleted);
    }
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabL1RouteHookReconstructionTest,
    "PinkCab.Vertical.L1.RouteHooks.Reconstruction",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPinkCabL1RouteHookReconstructionTest::RunTest(const FString& Parameters)
{
    FPinkCabRoadGraph GraphA;
    FPinkCabVerticalContactRegistry ContactsA;
    FPinkCabL1RouteHookRuntime HooksA;
    FPinkCabCityIdentity CityA;
    TestTrue(TEXT("fixture A builds"), PinkCabL1RouteHookTests::BuildFixture(GraphA, ContactsA, HooksA, CityA));

    FPinkCabRoadGraph GraphB;
    FPinkCabVerticalContactRegistry ContactsB;
    FPinkCabL1RouteHookRuntime HooksB;
    FPinkCabCityIdentity CityB;
    TestTrue(TEXT("fixture B builds"), PinkCabL1RouteHookTests::BuildFixture(GraphB, ContactsB, HooksB, CityB));

    TestEqual(TEXT("contact signatures reconstruct"),
        ContactsA.GetReconstructionSignature(), ContactsB.GetReconstructionSignature());
    TestEqual(TEXT("hook signatures reconstruct"),
        HooksA.GetReconstructionSignature(), HooksB.GetReconstructionSignature());
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabL1RouteHookValidationTest,
    "PinkCab.Vertical.L1.RouteHooks.Validation",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPinkCabL1RouteHookValidationTest::RunTest(const FString& Parameters)
{
    const FPinkCabCityIdentity City = FPinkCabCityIdentity::Create(
        TEXT("VERT-ROUTE"), TEXT("gen-1"), TEXT("content-1"));
    FPinkCabRoadGraph Graph;
    Graph.AddLane(PinkCabL1RouteHookTests::Lane(
        TEXT("lane:valid"), TEXT("n0"), TEXT("n1")));
    FPinkCabVerticalContactRegistry Contacts;
    FString MissingLaneContact;
    TestTrue(TEXT("logical missing-lane contact can exist"), Contacts.TryAdd(
        City, FPinkCabLaneId(TEXT("lane:missing")),
        EPinkCabVerticalContactKind::WallLeft, TEXT("missing"), 0, MissingLaneContact));

    FPinkCabL1RouteHookRuntime Hooks(1);
    FString HookId;
    TestFalse(TEXT("hook rejects lane absent from graph"),
        Hooks.TryAddHook(Graph, Contacts, MissingLaneContact, 0, HookId));

    FString ValidContact;
    TestTrue(TEXT("valid contact registers"), Contacts.TryAdd(
        City, FPinkCabLaneId(TEXT("lane:valid")),
        EPinkCabVerticalContactKind::WallLeft, TEXT("valid"), 0, ValidContact));
    TestTrue(TEXT("valid hook registers"),
        Hooks.TryAddHook(Graph, Contacts, ValidContact, 0, HookId));
    FString DuplicateOut = TEXT("preserve");
    TestFalse(TEXT("duplicate hook rejected"),
        Hooks.TryAddHook(Graph, Contacts, ValidContact, 0, DuplicateOut));
    TestEqual(TEXT("failed add does not mutate output"), DuplicateOut, FString(TEXT("preserve")));
    return true;
}

#endif
