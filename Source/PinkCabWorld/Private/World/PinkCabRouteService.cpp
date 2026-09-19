#include "World/PinkCabRouteService.h"
#include "Algo/Reverse.h"

bool FPinkCabRouteRequest::IsValid() const
{
    return StartLaneId.IsValid() && GoalLaneId.IsValid() && MaxVisitedNodes > 0;
}

void FPinkCabRoute::Reset()
{
    LaneIds.Reset();
    TotalLengthCm = 0.0;
    RouteSignature.Reset();
}

bool FPinkCabRouteService::FindRoute(
    const FPinkCabRoadGraph& Graph,
    const FPinkCabRouteRequest& Request,
    const FPinkCabCityDeltaState& Deltas,
    FPinkCabRoute& OutRoute)
{
    OutRoute.Reset();
    const FPinkCabLogicalLane* StartLane = nullptr;
    if (!ValidateEndpoints(Graph, Request, Deltas, StartLane))
    {
        return false;
    }
    const FString StartKey = Request.StartLaneId.Serialize();
    const FString GoalKey = Request.GoalLaneId.Serialize();
    TMap<FString, double> BestCost;
    TMap<FString, FString> Previous;
    if (!SearchShortestPath(
        Graph,
        Request,
        Deltas,
        *StartLane,
        StartKey,
        GoalKey,
        BestCost,
        Previous))
    {
        return false;
    }
    return BuildRoute(StartKey, GoalKey, BestCost, Previous, OutRoute);
}

bool FPinkCabRouteService::ValidateEndpoints(
    const FPinkCabRoadGraph& Graph,
    const FPinkCabRouteRequest& Request,
    const FPinkCabCityDeltaState& Deltas,
    const FPinkCabLogicalLane*& OutStartLane)
{
    if (!Request.IsValid()
        || Deltas.IsLaneClosed(Request.StartLaneId)
        || Deltas.IsLaneClosed(Request.GoalLaneId))
    {
        return false;
    }
    OutStartLane = Graph.FindLane(Request.StartLaneId);
    return OutStartLane && Graph.FindLane(Request.GoalLaneId);
}

bool FPinkCabRouteService::SearchShortestPath(
    const FPinkCabRoadGraph& Graph,
    const FPinkCabRouteRequest& Request,
    const FPinkCabCityDeltaState& Deltas,
    const FPinkCabLogicalLane& StartLane,
    const FString& StartKey,
    const FString& GoalKey,
    TMap<FString, double>& OutBestCost,
    TMap<FString, FString>& OutPrevious)
{
    TArray<FFrontierEntry> Frontier;
    TMap<FString, FString> BestPathKey;
    Frontier.Add({StartKey, StartLane.LengthCm, StartKey});
    OutBestCost.Add(StartKey, StartLane.LengthCm);
    BestPathKey.Add(StartKey, StartKey);

    int32 VisitedCount = 0;
    while (Frontier.Num() > 0)
    {
        const int32 BestIndex = FindBestFrontierIndex(Frontier);
        const FFrontierEntry Current = Frontier[BestIndex];
        Frontier.RemoveAtSwap(BestIndex, 1, EAllowShrinking::No);

        const double* KnownCost = OutBestCost.Find(Current.LaneKey);
        const FString* KnownPath = BestPathKey.Find(Current.LaneKey);
        const bool bCurrentEntry = KnownCost && KnownPath
            && FMath::IsNearlyEqual(Current.Cost, *KnownCost)
            && Current.PathKey == *KnownPath;
        if (!bCurrentEntry)
        {
            continue;
        }
        if (++VisitedCount > Request.MaxVisitedNodes)
        {
            return false;
        }
        if (Current.LaneKey == GoalKey)
        {
            return true;
        }
        ExpandFrontier(
            Graph,
            Deltas,
            Current,
            Frontier,
            OutBestCost,
            BestPathKey,
            OutPrevious);
    }
    return false;
}

void FPinkCabRouteService::ExpandFrontier(
    const FPinkCabRoadGraph& Graph,
    const FPinkCabCityDeltaState& Deltas,
    const FFrontierEntry& Current,
    TArray<FFrontierEntry>& Frontier,
    TMap<FString, double>& BestCost,
    TMap<FString, FString>& BestPathKey,
    TMap<FString, FString>& Previous)
{
    const TArray<FPinkCabLaneId> NextLanes =
        Graph.GetNextLanes(FPinkCabLaneId(Current.LaneKey));
    for (const FPinkCabLaneId& NextId : NextLanes)
    {
        if (Deltas.IsLaneClosed(NextId))
        {
            continue;
        }
        const FPinkCabLogicalLane* NextLane = Graph.FindLane(NextId);
        if (NextLane)
        {
            TryRelaxLane(
                *NextLane,
                Current,
                Frontier,
                BestCost,
                BestPathKey,
                Previous);
        }
    }
}

void FPinkCabRouteService::TryRelaxLane(
    const FPinkCabLogicalLane& NextLane,
    const FFrontierEntry& Current,
    TArray<FFrontierEntry>& Frontier,
    TMap<FString, double>& BestCost,
    TMap<FString, FString>& BestPathKey,
    TMap<FString, FString>& Previous)
{
    const FString NextKey = NextLane.LaneId.Serialize();
    const double CandidateCost = Current.Cost + NextLane.LengthCm;
    const FString CandidatePathKey = Current.PathKey + TEXT("|") + NextKey;
    const double* ExistingCost = BestCost.Find(NextKey);
    const FString* ExistingPath = BestPathKey.Find(NextKey);
    const bool bBetterCost = !ExistingCost || CandidateCost < *ExistingCost;
    const bool bEqualCostBetterPath = ExistingCost && ExistingPath
        && FMath::IsNearlyEqual(CandidateCost, *ExistingCost)
        && CandidatePathKey < *ExistingPath;
    if (!bBetterCost && !bEqualCostBetterPath)
    {
        return;
    }
    BestCost.Add(NextKey, CandidateCost);
    BestPathKey.Add(NextKey, CandidatePathKey);
    Previous.Add(NextKey, Current.LaneKey);
    Frontier.Add({NextKey, CandidateCost, CandidatePathKey});
}

bool FPinkCabRouteService::BuildRoute(
    const FString& StartKey,
    const FString& GoalKey,
    const TMap<FString, double>& BestCost,
    const TMap<FString, FString>& Previous,
    FPinkCabRoute& OutRoute)
{
    TArray<FString> ReverseKeys;
    FString Cursor = GoalKey;
    ReverseKeys.Add(Cursor);
    while (Cursor != StartKey)
    {
        const FString* Parent = Previous.Find(Cursor);
        if (!Parent)
        {
            return false;
        }
        Cursor = *Parent;
        ReverseKeys.Add(Cursor);
    }

    Algo::Reverse(ReverseKeys);
    OutRoute.LaneIds.Reserve(ReverseKeys.Num());
    FString SignaturePayload;
    for (const FString& Key : ReverseKeys)
    {
        OutRoute.LaneIds.Add(FPinkCabLaneId(Key));
        SignaturePayload += Key;
        SignaturePayload += TEXT(";");
    }
    OutRoute.TotalLengthCm = BestCost.FindRef(GoalKey);
    SignaturePayload += FString::Printf(TEXT("|%.3f"), OutRoute.TotalLengthCm);
    OutRoute.RouteSignature = PinkCabWorldId::StableToken(
        TEXT("route:"),
        SignaturePayload);
    return true;
}

int32 FPinkCabRouteService::FindBestFrontierIndex(
    const TArray<FFrontierEntry>& Frontier)
{
    int32 BestIndex = 0;
    for (int32 Index = 1; Index < Frontier.Num(); ++Index)
    {
        const FFrontierEntry& Candidate = Frontier[Index];
        const FFrontierEntry& Best = Frontier[BestIndex];
        const bool bLowerCost = Candidate.Cost < Best.Cost;
        const bool bEqualCostBetterPath =
            FMath::IsNearlyEqual(Candidate.Cost, Best.Cost)
            && Candidate.PathKey < Best.PathKey;
        if (bLowerCost || bEqualCostBetterPath)
        {
            BestIndex = Index;
        }
    }
    return BestIndex;
}
