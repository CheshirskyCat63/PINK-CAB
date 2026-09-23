#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "World/PinkCabWorldMaterializationPolicy.h"
#include "World/PinkCabCityLocationRegistry.h"

namespace
{
FPinkCabWorldChunkCandidate Candidate(
    const FPinkCabCityIdentity& City,
    int32 Longitudinal,
    int32 Lateral,
    int32 Layer)
{
    FPinkCabWorldChunkCandidate Result;
    Result.Coord = {Longitudinal, Lateral, Layer};
    Result.ChunkId = FPinkCabChunkId::From(City, Result.Coord);
    return Result;
}

FPinkCabWorldMaterializationSettings Settings(int32 MaxChunks = 3)
{
    FPinkCabWorldMaterializationSettings Result;
    Result.MaxMaterializedChunks = MaxChunks;
    Result.MaxDistanceSquared = 25;
    Result.AllowedLayers = {0, 1};
    Result.MaxAnchorRequests = 4;
    return Result;
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabWorldMaterializationNearStableTest,
    "PinkCab.World.Materialization.NearStableBounded",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabWorldMaterializationNearStableTest::RunTest(const FString& Parameters)
{
    const FPinkCabCityIdentity City = FPinkCabCityIdentity::Create(TEXT("MAT"), TEXT("gen-1"), TEXT("content-1"));
    TArray<FPinkCabWorldChunkCandidate> Candidates;
    Candidates.Add(Candidate(City, 5, 0, 0));
    Candidates.Add(Candidate(City, 0, 1, 0));
    Candidates.Add(Candidate(City, 1, 0, 0));
    Candidates.Add(Candidate(City, 0, 0, 0));
    Candidates.Add(Candidate(City, 2, 0, 0));

    FPinkCabWorldMaterializationResult Result;
    TestTrue(TEXT("window builds"), FPinkCabWorldMaterializationPolicy::BuildWindow(
        {0, 0, 0}, Candidates, Settings(3), {}, Result));
    TestEqual(TEXT("hard chunk cap obeyed"), Result.MaterializeChunkIds.Num(), 3);
    TestEqual(TEXT("nearest origin selected first"), Result.MaterializeChunkIds[0].Serialize(), Candidates[3].ChunkId.Serialize());
    TestEqual(TEXT("equal-distance tie is stable by chunk id"),
        Result.MaterializeChunkIds[1].Serialize() < Result.MaterializeChunkIds[2].Serialize(), true);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabWorldMaterializationLayerAndDematerializeTest,
    "PinkCab.World.Materialization.LayerAndDematerialize",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabWorldMaterializationLayerAndDematerializeTest::RunTest(const FString& Parameters)
{
    const FPinkCabCityIdentity City = FPinkCabCityIdentity::Create(TEXT("MAT"), TEXT("gen-1"), TEXT("content-1"));
    TArray<FPinkCabWorldChunkCandidate> Candidates;
    Candidates.Add(Candidate(City, 0, 0, 0));
    Candidates.Add(Candidate(City, 1, 0, 1));
    Candidates.Add(Candidate(City, 0, 0, 2));
    Candidates.Add(Candidate(City, 8, 0, 0));

    TArray<FPinkCabChunkId> Previous;
    Previous.Add(Candidates[0].ChunkId);
    Previous.Add(Candidates[3].ChunkId);
    FPinkCabWorldMaterializationResult Result;
    TestTrue(TEXT("window builds"), FPinkCabWorldMaterializationPolicy::BuildWindow(
        {1, 0, 0}, Candidates, Settings(4), Previous, Result));
    TestEqual(TEXT("L1 and L2 logical layers accepted"), Result.MaterializeChunkIds.Num(), 2);
    TestEqual(TEXT("leaving previous chunk dematerialized"), Result.DematerializeChunkIds.Num(), 1);
    TestEqual(TEXT("far previous id dematerialized deterministically"),
        Result.DematerializeChunkIds[0].Serialize(), Candidates[3].ChunkId.Serialize());
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabWorldMaterializationForwardReverseTest,
    "PinkCab.World.Materialization.ForwardReverseStableIdentity",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabWorldMaterializationForwardReverseTest::RunTest(const FString& Parameters)
{
    const FPinkCabCityIdentity City = FPinkCabCityIdentity::Create(TEXT("MAT"), TEXT("gen-1"), TEXT("content-1"));
    TArray<FPinkCabWorldChunkCandidate> Candidates;
    for (int32 X = -2; X <= 4; ++X)
    {
        Candidates.Add(Candidate(City, X, 0, 0));
    }

    FPinkCabWorldMaterializationResult StartA;
    FPinkCabWorldMaterializationResult Forward;
    FPinkCabWorldMaterializationResult StartB;
    const FPinkCabWorldMaterializationSettings Policy = Settings(3);
    TestTrue(TEXT("start A"), FPinkCabWorldMaterializationPolicy::BuildWindow({0, 0, 0}, Candidates, Policy, {}, StartA));
    TestTrue(TEXT("forward"), FPinkCabWorldMaterializationPolicy::BuildWindow({3, 0, 0}, Candidates, Policy, StartA.MaterializeChunkIds, Forward));
    TestTrue(TEXT("return"), FPinkCabWorldMaterializationPolicy::BuildWindow({0, 0, 0}, Candidates, Policy, Forward.MaterializeChunkIds, StartB));
    TestEqual(TEXT("return reproduces exact logical module signature"), StartA.RequestSignature, StartB.RequestSignature);
    TestEqual(TEXT("request count never grows"), StartB.MaterializeChunkIds.Num(), StartA.MaterializeChunkIds.Num());
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabWorldMaterializationAnchorRequestTest,
    "PinkCab.World.Materialization.GraphAnchors",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabWorldMaterializationAnchorRequestTest::RunTest(const FString& Parameters)
{
    const FPinkCabCityIdentity City = FPinkCabCityIdentity::Create(TEXT("MAT"), TEXT("gen-1"), TEXT("content-1"));
    FPinkCabCityLocationRegistry Registry;
    auto AddAnchor = [&](const TCHAR* Lane, EPinkCabCityLocationKind Kind, const TCHAR* Key, int32 Layer)
    {
        FPinkCabCityLocationAnchor Anchor;
        Anchor.AnchorId = FPinkCabCityLocationRegistry::MakeAnchorId(City, FPinkCabLaneId(Lane), Kind, Key);
        Anchor.Kind = Kind;
        Anchor.LaneId = FPinkCabLaneId(Lane);
        Anchor.LongitudinalCm = 100.0;
        Anchor.Layer = Layer;
        return Registry.TryRegister(Anchor);
    };
    TestTrue(TEXT("L1 service registered"), AddAnchor(TEXT("lane:s0"), EPinkCabCityLocationKind::Service, TEXT("garage"), 0));
    TestTrue(TEXT("L2 service registered"), AddAnchor(TEXT("lane:s1"), EPinkCabCityLocationKind::Service, TEXT("metro"), 1));
    TestTrue(TEXT("unsupported layer registered logically"), AddAnchor(TEXT("lane:s2"), EPinkCabCityLocationKind::Rule, TEXT("future"), 2));

    TArray<FString> Requests;
    TestTrue(TEXT("anchor requests build"), FPinkCabWorldMaterializationPolicy::BuildAnchorRequests(Registry, Settings(3), Requests));
    TestEqual(TEXT("only allowed layers requested"), Requests.Num(), 2);
    TestTrue(TEXT("anchor requests sorted deterministically"), Requests[0] < Requests[1]);
    return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabWorldMaterializationLongRunBoundedTest,
    "PinkCab.World.Materialization.LongRunBoundedTraversal",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabWorldMaterializationLongRunBoundedTest::RunTest(const FString& Parameters)
{
    constexpr int32 TraversalChunks = 2048;
    constexpr int32 MaxMaterialized = 3;
    const FPinkCabCityIdentity City = FPinkCabCityIdentity::Create(TEXT("MAT-LONG"), TEXT("gen-1"), TEXT("content-1"));
    TArray<FPinkCabWorldChunkCandidate> Candidates;
    Candidates.Reserve(TraversalChunks + 11);
    for (int32 X = -5; X <= TraversalChunks + 5; ++X)
    {
        Candidates.Add(Candidate(City, X, 0, 0));
    }

    const FPinkCabWorldMaterializationSettings Policy = Settings(MaxMaterialized);
    FPinkCabWorldMaterializationResult Start;
    TestTrue(TEXT("long-run start window builds"),
        FPinkCabWorldMaterializationPolicy::BuildWindow({0, 0, 0}, Candidates, Policy, {}, Start));

    TArray<FPinkCabChunkId> Previous = Start.MaterializeChunkIds;
    TSet<FString> SeenChunkIds;
    for (const FPinkCabChunkId& Id : Previous)
    {
        SeenChunkIds.Add(Id.Serialize());
    }

    int32 MaxActiveSeen = Previous.Num();
    int32 MaxDematerializedSeen = 0;
    bool bAllWindowsBuilt = true;
    bool bAllSetsUnique = true;
    for (int32 X = 1; X <= TraversalChunks; ++X)
    {
        FPinkCabWorldMaterializationResult Step;
        if (!FPinkCabWorldMaterializationPolicy::BuildWindow({X, 0, 0}, Candidates, Policy, Previous, Step))
        {
            bAllWindowsBuilt = false;
            break;
        }

        MaxActiveSeen = FMath::Max(MaxActiveSeen, Step.MaterializeChunkIds.Num());
        MaxDematerializedSeen = FMath::Max(MaxDematerializedSeen, Step.DematerializeChunkIds.Num());
        TSet<FString> ActiveIds;
        for (const FPinkCabChunkId& Id : Step.MaterializeChunkIds)
        {
            const FString Serialized = Id.Serialize();
            bAllSetsUnique &= !ActiveIds.Contains(Serialized);
            ActiveIds.Add(Serialized);
            SeenChunkIds.Add(Serialized);
        }
        Previous = MoveTemp(Step.MaterializeChunkIds);
    }

    FPinkCabWorldMaterializationResult Returned;
    const bool bReturned = FPinkCabWorldMaterializationPolicy::BuildWindow(
        {0, 0, 0}, Candidates, Policy, Previous, Returned);

    TestTrue(TEXT("all 2048 sequential streaming windows build"), bAllWindowsBuilt);
    TestTrue(TEXT("active chunk ids remain unique"), bAllSetsUnique);
    TestTrue(TEXT("active materialization never exceeds hard cap"), MaxActiveSeen <= MaxMaterialized);
    TestTrue(TEXT("dematerialization work remains bounded by previous active set"), MaxDematerializedSeen <= MaxMaterialized);
    TestTrue(TEXT("long traversal visits more than one thousand stable chunk ids"), SeenChunkIds.Num() > 1000);
    TestTrue(TEXT("return window builds after long traversal"), bReturned);
    if (bReturned)
    {
        TestEqual(TEXT("return after long traversal reproduces origin signature"), Returned.RequestSignature, Start.RequestSignature);
        TestTrue(TEXT("return active set remains bounded"), Returned.MaterializeChunkIds.Num() <= MaxMaterialized);
    }
    return true;
}

#endif
