#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Vehicle/PinkCabL1TraversalState.h"
#include "Vehicle/PinkCabVehicleLoadState.h"
#include "World/PinkCabL1RouteHookRuntime.h"
#include "World/PinkCabMetroTransitRuntime.h"
#include "World/PinkCabSuspendedBusRuntime.h"
#include "World/PinkCabVerticalAcceptanceCourse.h"
#include "FileHelpers.h"
#include "Editor.h"
#include "EngineUtils.h"

namespace PinkCabVerticalAcceptanceTests
{
FPinkCabLogicalLane Lane(const TCHAR* Id, const TCHAR* From, const TCHAR* To)
{
    FPinkCabLogicalLane Lane;
    Lane.LaneId = FPinkCabLaneId(Id);
    Lane.FromNode = FPinkCabRoadNodeId(From);
    Lane.ToNode = FPinkCabRoadNodeId(To);
    Lane.LengthCm = 100.0;
    Lane.Layer = 1;
    return Lane;
}
float BuildAuthoritativeMass(float TargetMassKg)
{
    const FPinkCabTatraProfile Profile = FPinkCabTatraProfile::Canonical();
    FPinkCabVehicleLoadState Load;
    const float ExtraMass = TargetMassKg - Profile.BaseVehicleMassKg;
    if (ExtraMass > 0.0f)
    {
        Load.AddPassenger(FPinkCabVehicleLoadItem(ExtraMass, 0.0f));
    }
    return Load.GetTotalMassKg(Profile);
}

FPinkCabL1TraversalInput Contact(
    EPinkCabVerticalContactKind Kind,
    const FString& ContactId,
    float MassKg)
{
    FPinkCabL1TraversalInput Input;
    Input.bHasContact = true;
    Input.ContactKind = Kind;
    Input.ContactId = ContactId;
    Input.TotalMassKg = MassKg;
    Input.DeltaSeconds = 0.25f;
    return Input;
}
struct FLogicalFixture
{
    FPinkCabCityIdentity City = APinkCabVerticalAcceptanceCourse::GetPresentationCityIdentity();
    FPinkCabRoadGraph Graph;
    FPinkCabVerticalContactRegistry Contacts;
    FPinkCabL1RouteHookRuntime Hooks;
    TArray<FString> ContactIds;

    bool Build()
    {
        Graph.AddLane(Lane(*APinkCabVerticalAcceptanceCourse::GetPresentationLaneId(0).Serialize(), TEXT("n0"), TEXT("n1")));
        Graph.AddLane(Lane(*APinkCabVerticalAcceptanceCourse::GetPresentationLaneId(1).Serialize(), TEXT("n1"), TEXT("n2")));
        Graph.AddLane(Lane(*APinkCabVerticalAcceptanceCourse::GetPresentationLaneId(2).Serialize(), TEXT("n1"), TEXT("n2")));
        Graph.AddLane(Lane(*APinkCabVerticalAcceptanceCourse::GetPresentationLaneId(3).Serialize(), TEXT("n2"), TEXT("n3")));
        ContactIds.SetNum(4);
        return AddContact(0, EPinkCabVerticalContactKind::WallLeft, TEXT("wall"))
            && AddContact(1, EPinkCabVerticalContactKind::FreightCeiling, TEXT("freight"))
            && AddContact(2, EPinkCabVerticalContactKind::PoplarGapHook, TEXT("gap"))
            && AddContact(3, EPinkCabVerticalContactKind::ReceivingStrip, TEXT("receive"));
    }
private:
    bool AddContact(
        int32 Index,
        EPinkCabVerticalContactKind Kind,
        const TCHAR* SemanticKey)
    {
        const FPinkCabLaneId LaneId = APinkCabVerticalAcceptanceCourse::GetPresentationLaneId(Index);
        FString ContactId;
        if (!Contacts.TryAdd(City, LaneId, Kind, SemanticKey, 1, ContactId))
        {
            return false;
        }
        if (ContactId != APinkCabVerticalAcceptanceCourse::GetPresentationContactId(Index))
        {
            return false;
        }
        FString HookId;
        if (!Hooks.TryAddHook(Graph, Contacts, ContactId, 0, HookId))
        {
            return false;
        }
        ContactIds[Index] = ContactId;
        return true;
    }
};
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabVerticalMassAcceptanceTest,
    "PinkCab.Vertical.Acceptance.MassFixtures",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabVerticalMassAcceptanceTest::RunTest(const FString& Parameters)
{
    using namespace PinkCabVerticalAcceptanceTests;
    FLogicalFixture F;
    TestTrue(TEXT("logical fixture builds"), F.Build());
    const float Targets[] = {1500.0f, 1657.0f, 1882.0f, 2107.0f};
    for (const float Target : Targets)
    {
        const float Mass = BuildAuthoritativeMass(Target);
        TestTrue(TEXT("authoritative mass exact"), FMath::IsNearlyEqual(Mass, Target));
        FPinkCabL1TraversalState State;
        State.Step(Contact(EPinkCabVerticalContactKind::WallLeft, F.ContactIds[0], Mass));
        FPinkCabL1TraversalInput Air;
        Air.TotalMassKg = Mass;
        Air.DeltaSeconds = 0.5f;
        TestEqual(TEXT("wall enters residual"), State.Step(Air).Phase,
            EPinkCabL1TraversalPhase::WallResidual);
        TestEqual(TEXT("wall reacquires"),
            State.Step(Contact(EPinkCabVerticalContactKind::WallLeft, F.ContactIds[0], Mass)).Phase,
            EPinkCabL1TraversalPhase::WallContact);
        TestEqual(TEXT("freight entered"),
            State.Step(Contact(EPinkCabVerticalContactKind::FreightCeiling, F.ContactIds[1], Mass)).Phase,
            EPinkCabL1TraversalPhase::FreightContact);
        const FPinkCabL1TraversalResult Completed = State.Step(
            Contact(EPinkCabVerticalContactKind::ReceivingStrip, F.ContactIds[3], Mass));
        TestEqual(TEXT("receiving completes"), Completed.Phase,
            EPinkCabL1TraversalPhase::Completed);
        TestTrue(TEXT("completion explicit"), Completed.bCompleted);
    }
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabVerticalGapAcceptanceTest,
    "PinkCab.Vertical.Acceptance.GapTransfer",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabVerticalGapAcceptanceTest::RunTest(const FString& Parameters)
{
    using namespace PinkCabVerticalAcceptanceTests;
    FLogicalFixture F;
    TestTrue(TEXT("logical fixture builds"), F.Build());
    const float Mass = BuildAuthoritativeMass(1657.0f);
    FPinkCabL1TraversalState State;
    TestEqual(TEXT("wall entered"),
        State.Step(Contact(EPinkCabVerticalContactKind::WallLeft, F.ContactIds[0], Mass)).Phase,
        EPinkCabL1TraversalPhase::WallContact);
    TestEqual(TEXT("gap entered"),
        State.Step(Contact(EPinkCabVerticalContactKind::PoplarGapHook, F.ContactIds[2], Mass)).Phase,
        EPinkCabL1TraversalPhase::GapTransfer);
    const FPinkCabL1TraversalResult Result = State.Step(
        Contact(EPinkCabVerticalContactKind::ReceivingStrip, F.ContactIds[3], Mass));
    TestEqual(TEXT("gap reaches receiving"), Result.Phase,
        EPinkCabL1TraversalPhase::Completed);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabVerticalL2AcceptanceTest,
    "PinkCab.Vertical.Acceptance.L2",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPinkCabVerticalL2AcceptanceTest::RunTest(const FString& Parameters)
{
    FPinkCabSuspendedBusRuntime Bus(3, 2);
    TestTrue(TEXT("bus segment 0"), Bus.TryAddSegment(TEXT("bus:l2-0"), 100.0, false));
    TestTrue(TEXT("bus segment 1"), Bus.TryAddSegment(TEXT("bus:l2-1"), 200.0, true));
    TestTrue(TEXT("bus obstacle"), Bus.TryAddObstacle(
        TEXT("bus:obstacle"), TEXT("bus:l2-1"), 25.0, 75.0));
    TestTrue(TEXT("bus starts"), Bus.Start(TEXT("route:vertical-bus"), 50.0));
    TestTrue(TEXT("bus advances"), Bus.Advance(2.5));
    TestEqual(TEXT("bus reaches contact segment"), Bus.GetCurrentSegmentId(),
        FString(TEXT("bus:l2-1")));
    TestTrue(TEXT("bus contact eligible"), Bus.IsTatraContactEligible());
    TestEqual(TEXT("bus obstacle window"), Bus.GetPhase(),
        EPinkCabSuspendedBusPhase::ObstacleWindow);

    const FPinkCabCityIdentity City = APinkCabVerticalAcceptanceCourse::GetPresentationCityIdentity();
    FPinkCabMetroTransitRuntime Metro(3, 2);
    FString A, B, Segment;
    TestTrue(TEXT("metro station A"), Metro.TryAddStation(City, TEXT("eyes-a"), A));
    TestTrue(TEXT("metro station B"), Metro.TryAddStation(City, TEXT("eyes-b"), B));
    TestTrue(TEXT("metro segment"), Metro.TryAddSegment(
        City, A, B, TEXT("eyes-ab"), 10.0, 2.0, 4.0, Segment));
    TestTrue(TEXT("metro starts"), Metro.Start(TEXT("route:vertical-metro"), Segment));
    TestTrue(TEXT("metro advances into transition window"), Metro.Advance(2.5));
    TestTrue(TEXT("metro transition eligible"), Metro.RefreshTatraTransitionState());
    TestTrue(TEXT("metro contact entered"), Metro.EnterTransitContact());
    TestTrue(TEXT("metro driveable only in contact"), Metro.IsMetroDriveableForTatra());
    TestTrue(TEXT("metro exit requested"), Metro.RequestExit());
    TestTrue(TEXT("metro exit completed"), Metro.CompleteExit());
    TestEqual(TEXT("metro returns road"), Metro.GetTatraState(),
        EPinkCabMetroTatraState::Road);
    TestFalse(TEXT("metro road is non-driveable"), Metro.IsMetroDriveableForTatra());
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabVerticalAbortAcceptanceTest,
    "PinkCab.Vertical.Acceptance.AbortReset",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabVerticalAbortAcceptanceTest::RunTest(const FString& Parameters)
{
    using namespace PinkCabVerticalAcceptanceTests;
    FLogicalFixture F;
    TestTrue(TEXT("fixture builds"), F.Build());
    const float Mass = BuildAuthoritativeMass(1657.0f);
    FPinkCabL1TraversalState L1;
    L1.Step(Contact(EPinkCabVerticalContactKind::WallLeft, F.ContactIds[0], Mass));
    FPinkCabL1TraversalInput AbortInput;
    AbortInput.TotalMassKg = Mass;
    AbortInput.bAbort = true;
    const FPinkCabL1TraversalResult L1Abort = L1.Step(AbortInput);
    TestEqual(TEXT("L1 abort legal"), L1Abort.Phase, EPinkCabL1TraversalPhase::Aborted);
    TestTrue(TEXT("L1 abort clears current contact"), L1Abort.CurrentContactId.IsEmpty());
    L1.Reset();
    FPinkCabL1TraversalInput Air;
    Air.TotalMassKg = Mass;
    TestEqual(TEXT("L1 reset returns road"), L1.Step(Air).Phase,
        EPinkCabL1TraversalPhase::Road);

    FPinkCabSuspendedBusRuntime Bus;
    Bus.TryAddSegment(TEXT("bus:abort"), 100.0, true);
    Bus.Start(TEXT("route:abort"), 25.0);
    Bus.Abort();
    TestEqual(TEXT("bus abort legal"), Bus.GetPhase(), EPinkCabSuspendedBusPhase::Aborted);
    TestTrue(TEXT("bus abort clears segment"), Bus.GetCurrentSegmentId().IsEmpty());
    Bus.Reset();
    TestEqual(TEXT("bus reset idle"), Bus.GetPhase(), EPinkCabSuspendedBusPhase::Idle);
    FPinkCabMetroTransitRuntime Metro;
    FString A, B, Segment;
    Metro.TryAddStation(F.City, TEXT("abort-a"), A);
    Metro.TryAddStation(F.City, TEXT("abort-b"), B);
    Metro.TryAddSegment(F.City, A, B, TEXT("abort-ab"), 10.0, 2.0, 4.0, Segment);
    Metro.Start(TEXT("route:abort"), Segment);
    Metro.Advance(2.5);
    Metro.RefreshTatraTransitionState();
    Metro.EnterTransitContact();
    Metro.Abort();
    TestEqual(TEXT("metro abort legal"), Metro.GetTatraState(), EPinkCabMetroTatraState::Aborted);
    TestTrue(TEXT("metro abort clears segment"), Metro.GetCurrentSegmentId().IsEmpty());
    TestFalse(TEXT("metro abort non-driveable"), Metro.IsMetroDriveableForTatra());
    Metro.Reset();
    TestEqual(TEXT("metro reset road"), Metro.GetTatraState(), EPinkCabMetroTatraState::Road);
    TestTrue(TEXT("metro reset clears route"), Metro.GetActiveRouteId().IsEmpty());
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabVerticalCourseContractTest,
    "PinkCab.Vertical.Acceptance.CourseContract",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabVerticalCourseContractTest::RunTest(const FString& Parameters)
{
    const APinkCabVerticalAcceptanceCourse* Course = GetDefault<APinkCabVerticalAcceptanceCourse>();
    TestNotNull(TEXT("acceptance course class exists"), Course);
    if (!Course) return false;
    TestEqual(TEXT("course is explicitly non-authoritative"), Course->GetAuthorityMarker(),
        FString(TEXT("NON_AUTHORITATIVE_GEOMETRY")));
    TestTrue(TEXT("course presentation only"), Course->IsPresentationOnly());
    TestEqual(TEXT("four L1 semantic primitives"), Course->GetL1PrimitiveCount(), 4);
    TestTrue(TEXT("L2 presentation exists"), Course->GetL2PrimitiveCount() >= 3);
    for (int32 Index = 0; Index < 4; ++Index)
    {
        const FPinkCabLaneId LaneId = APinkCabVerticalAcceptanceCourse::GetPresentationLaneId(Index);
        const FString ContactId = APinkCabVerticalAcceptanceCourse::GetPresentationContactId(Index);
        TestTrue(TEXT("presentation lane stable"), LaneId.IsValid());
        TestFalse(TEXT("presentation contact stable"), ContactId.IsEmpty());
        TestTrue(TEXT("contact resolves to primitive"), Course->HasPrimitiveForContact(ContactId));
        TestEqual(TEXT("saved presentation contact matches logical id"),
            Course->GetPresentedContactId(Index), ContactId);
    }
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabVerticalMapAcceptanceTest,
    "PinkCab.Vertical.Acceptance.Map",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabVerticalMapAcceptanceTest::RunTest(const FString& Parameters)
{
    TestTrue(TEXT("vertical acceptance map loads"),
        FEditorFileUtils::LoadMap(TEXT("/Game/Dev/Maps/L_PinkCab_VerticalAcceptance"), false, true));
    UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
    TestNotNull(TEXT("editor world exists"), World);
    if (!World) return false;
    int32 CourseCount = 0;
    for (TActorIterator<APinkCabVerticalAcceptanceCourse> It(World); It; ++It)
    {
        ++CourseCount;
        TestEqual(TEXT("saved actor keeps authority marker"), It->GetAuthorityMarker(),
            FString(TEXT("NON_AUTHORITATIVE_GEOMETRY")));
        TestEqual(TEXT("saved actor keeps L1 primitives"), It->GetL1PrimitiveCount(), 4);
        TestTrue(TEXT("saved actor keeps L2 presentation"), It->GetL2PrimitiveCount() >= 3);
        for (int32 Index = 0; Index < 4; ++Index)
        {
            TestEqual(TEXT("saved contact ID remains logical"), It->GetPresentedContactId(Index),
                APinkCabVerticalAcceptanceCourse::GetPresentationContactId(Index));
        }
    }
    TestEqual(TEXT("map contains one vertical acceptance course"), CourseCount, 1);
    TestTrue(TEXT("MapCheck executes"), GEditor->Exec(World, TEXT("MAP CHECK"), *GLog));
    return true;
}

#endif
