#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "World/PinkCabCityDeltaState.h"
#include "World/PinkCabCityLocationRegistry.h"
#include "World/PinkCabCityIdentity.h"
#include "World/PinkCabChunkId.h"
#include "World/PinkCabRoadGraph.h"

namespace
{
struct FPinkCabCityRuntimeFixture
{
    FPinkCabCityIdentity City = FPinkCabCityIdentity::Create(TEXT("EURO-63"), TEXT("gen-1"), TEXT("content-1"));
    FPinkCabChunkId Chunk = FPinkCabChunkId::From(City, {4, 0, 0});
    FPinkCabLaneId LaneA = FPinkCabRoadGraph::MakeLaneId(City, Chunk, 0, 0);
    FPinkCabLaneId LaneB = FPinkCabRoadGraph::MakeLaneId(City, Chunk, 0, 1);
};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabCityDeltaIdentityTest,
    "PinkCab.World.CityRuntime.Identity.DeltaIds",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabCityDeltaIdentityTest::RunTest(const FString& Parameters)
{
    const FPinkCabCityRuntimeFixture F;
    const FString A = FPinkCabCityDeltaState::MakeDeltaId(
        F.City, EPinkCabCityDeltaKind::LaneClosure, F.LaneA.Serialize(), TEXT("roadworks-01"));
    const FString B = FPinkCabCityDeltaState::MakeDeltaId(
        F.City, EPinkCabCityDeltaKind::LaneClosure, F.LaneA.Serialize(), TEXT("roadworks-01"));
    const FString Other = FPinkCabCityDeltaState::MakeDeltaId(
        F.City, EPinkCabCityDeltaKind::LaneClosure, F.LaneA.Serialize(), TEXT("roadworks-02"));
    TestEqual(TEXT("same logical delta inputs reproduce stable id"), A, B);
    TestNotEqual(TEXT("distinct operation key changes stable id"), A, Other);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabCityDeltaClosureTest,
    "PinkCab.World.CityRuntime.Identity.ClosureStore",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabCityDeltaClosureTest::RunTest(const FString& Parameters)
{
    const FPinkCabCityRuntimeFixture F;
    FPinkCabCityDeltaState State;
    FString DeltaId;
    TestTrue(TEXT("first closure accepted"),
        State.TryAddLaneClosure(F.City, F.LaneA, TEXT("roadworks-01"), DeltaId));
    TestFalse(TEXT("duplicate closure rejected"),
        State.TryAddLaneClosure(F.City, F.LaneA, TEXT("roadworks-01"), DeltaId));
    TestTrue(TEXT("closed lane resolves from pure data"), State.IsLaneClosed(F.LaneA));
    TestFalse(TEXT("other lane remains open"), State.IsLaneClosed(F.LaneB));
    TestEqual(TEXT("duplicate did not grow store"), State.Num(), 1);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabCityAnchorRegistryTest,
    "PinkCab.World.CityRuntime.Identity.AnchorRegistry",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabCityAnchorRegistryTest::RunTest(const FString& Parameters)
{
    const FPinkCabCityRuntimeFixture F;
    FPinkCabCityLocationRegistry Registry;
    const FString AnchorId = FPinkCabCityLocationRegistry::MakeAnchorId(
        F.City, F.LaneA, EPinkCabCityLocationKind::Pickup, TEXT("airport-pickup-a"));
    const FPinkCabCityLocationAnchor Anchor{
        AnchorId, EPinkCabCityLocationKind::Pickup, F.LaneA, 1250.0, 0};

    TestTrue(TEXT("first anchor accepted"), Registry.TryRegister(Anchor));
    TestFalse(TEXT("duplicate stable anchor id rejected"), Registry.TryRegister(Anchor));
    TestEqual(TEXT("duplicate did not grow registry"), Registry.Num(), 1);

    FPinkCabCityLocationAnchor Restored;
    TestTrue(TEXT("anchor resolves by stable id"), Registry.TryGet(AnchorId, Restored));
    TestEqual(TEXT("resolved lane identity preserved"),
        Restored.LaneId.Serialize(), F.LaneA.Serialize());
    TestEqual(TEXT("resolved longitudinal position preserved"), Restored.LongitudinalCm, 1250.0);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabCityRuntimeReconstructionSignatureTest,
    "PinkCab.World.CityRuntime.Identity.ReconstructionSignature",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabCityRuntimeReconstructionSignatureTest::RunTest(const FString& Parameters)
{
    const FPinkCabCityRuntimeFixture F;
    FPinkCabCityDeltaState DeltasA;
    FPinkCabCityDeltaState DeltasB;
    FString Ignored;
    DeltasA.TryAddLaneClosure(F.City, F.LaneA, TEXT("a"), Ignored);
    DeltasA.TryAddLaneClosure(F.City, F.LaneB, TEXT("b"), Ignored);
    DeltasB.TryAddLaneClosure(F.City, F.LaneB, TEXT("b"), Ignored);
    DeltasB.TryAddLaneClosure(F.City, F.LaneA, TEXT("a"), Ignored);
    TestEqual(TEXT("delta signature ignores insertion order"),
        DeltasA.GetReconstructionSignature(), DeltasB.GetReconstructionSignature());

    FPinkCabCityLocationRegistry AnchorsA;
    FPinkCabCityLocationRegistry AnchorsB;
    const FString AId = FPinkCabCityLocationRegistry::MakeAnchorId(
        F.City, F.LaneA, EPinkCabCityLocationKind::Pickup, TEXT("a"));
    const FString BId = FPinkCabCityLocationRegistry::MakeAnchorId(
        F.City, F.LaneB, EPinkCabCityLocationKind::Service, TEXT("b"));
    const FPinkCabCityLocationAnchor AnchorA{
        AId, EPinkCabCityLocationKind::Pickup, F.LaneA, 500.0, 0};
    const FPinkCabCityLocationAnchor AnchorB{
        BId, EPinkCabCityLocationKind::Service, F.LaneB, 900.0, 0};
    AnchorsA.TryRegister(AnchorA);
    AnchorsA.TryRegister(AnchorB);
    AnchorsB.TryRegister(AnchorB);
    AnchorsB.TryRegister(AnchorA);
    TestEqual(TEXT("anchor signature ignores insertion order"),
        AnchorsA.GetReconstructionSignature(), AnchorsB.GetReconstructionSignature());
    return true;
}

#endif

// Capacity is part of the runtime contract: persistent logical state must stay bounded.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabCityRuntimeCapacityTest,
    "PinkCab.World.CityRuntime.Identity.BoundedStores",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabCityRuntimeCapacityTest::RunTest(const FString& Parameters)
{
    const FPinkCabCityRuntimeFixture F;
    FPinkCabCityDeltaState Deltas(1);
    FString Id;
    TestTrue(TEXT("first bounded delta accepted"),
        Deltas.TryAddLaneClosure(F.City, F.LaneA, TEXT("one"), Id));
    TestFalse(TEXT("delta capacity rejects growth"),
        Deltas.TryAddLaneClosure(F.City, F.LaneB, TEXT("two"), Id));

    FPinkCabCityLocationRegistry Anchors(1);
    const FString AId = FPinkCabCityLocationRegistry::MakeAnchorId(
        F.City, F.LaneA, EPinkCabCityLocationKind::Pickup, TEXT("a"));
    const FString BId = FPinkCabCityLocationRegistry::MakeAnchorId(
        F.City, F.LaneB, EPinkCabCityLocationKind::Service, TEXT("b"));
    TestTrue(TEXT("first bounded anchor accepted"), Anchors.TryRegister(
        {AId, EPinkCabCityLocationKind::Pickup, F.LaneA, 100.0, 0}));
    TestFalse(TEXT("anchor capacity rejects growth"), Anchors.TryRegister(
        {BId, EPinkCabCityLocationKind::Service, F.LaneB, 200.0, 0}));
    return true;
}
