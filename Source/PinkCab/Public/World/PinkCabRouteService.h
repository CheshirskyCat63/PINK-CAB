#pragma once

#include "CoreMinimal.h"
#include "World/PinkCabCityDeltaState.h"
#include "World/PinkCabRoadGraph.h"

struct FPinkCabRouteRequest
{
    FPinkCabLaneId StartLaneId;
    FPinkCabLaneId GoalLaneId;
    int32 MaxVisitedNodes = 0;

    bool IsValid() const
    {
        return StartLaneId.IsValid() && GoalLaneId.IsValid() && MaxVisitedNodes > 0;
    }
};

struct FPinkCabRoute
{
    TArray<FPinkCabLaneId> LaneIds;
    double TotalLengthCm = 0.0;
    FString RouteSignature;

    void Reset()
    {
        LaneIds.Reset();
        TotalLengthCm = 0.0;
        RouteSignature.Reset();
    }
};

class FPinkCabRouteService
{
public:
    static bool FindRoute(
        const FPinkCabRoadGraph& Graph,
        const FPinkCabRouteRequest& Request,
        const FPinkCabCityDeltaState& Deltas,
        FPinkCabRoute& OutRoute)
    {
        OutRoute.Reset();
        if (!Request.IsValid())
        {
            return false;
        }

        const FPinkCabLogicalLane* StartLane = Graph.FindLane(Request.StartLaneId);
        const FPinkCabLogicalLane* GoalLane = Graph.FindLane(Request.GoalLaneId);
        if (!StartLane || !GoalLane
            || Deltas.IsLaneClosed(Request.StartLaneId)
            || Deltas.IsLaneClosed(Request.GoalLaneId))
        {
            return false;
        }

        TArray<FFrontierEntry> Frontier;
        TMap<FString, double> BestCost;
        TMap<FString, FString> BestPathKey;
        TMap<FString, FString> Previous;
        const FString StartKey = Request.StartLaneId.Serialize();
        const FString GoalKey = Request.GoalLaneId.Serialize();
        const FString StartPathKey = StartKey;
        Frontier.Add({StartKey, StartLane->LengthCm, StartPathKey});
        BestCost.Add(StartKey, StartLane->LengthCm);
        BestPathKey.Add(StartKey, StartPathKey);

        int32 VisitedCount = 0;
        bool bFoundGoal = false;
        while (Frontier.Num() > 0)
        {
            const int32 BestIndex = FindBestFrontierIndex(Frontier);
            const FFrontierEntry Current = Frontier[BestIndex];
            Frontier.RemoveAtSwap(BestIndex, 1, EAllowShrinking::No);

            const double* KnownCost = BestCost.Find(Current.LaneKey);
            const FString* KnownPath = BestPathKey.Find(Current.LaneKey);
            if (!KnownCost || !KnownPath
                || !FMath::IsNearlyEqual(Current.Cost, *KnownCost)
                || Current.PathKey != *KnownPath)
            {
                continue;
            }

            if (++VisitedCount > Request.MaxVisitedNodes)
            {
                return false;
            }
            if (Current.LaneKey == GoalKey)
            {
                bFoundGoal = true;
                break;
            }

            const TArray<FPinkCabLaneId> NextLanes = Graph.GetNextLanes(FPinkCabLaneId(Current.LaneKey));
            for (const FPinkCabLaneId& NextId : NextLanes)
            {
                if (Deltas.IsLaneClosed(NextId))
                {
                    continue;
                }
                const FPinkCabLogicalLane* NextLane = Graph.FindLane(NextId);
                if (!NextLane)
                {
                    continue;
                }

                const FString NextKey = NextId.Serialize();
                const double CandidateCost = Current.Cost + NextLane->LengthCm;
                const FString CandidatePathKey = Current.PathKey + TEXT("|") + NextKey;
                const double* ExistingCost = BestCost.Find(NextKey);
                const FString* ExistingPath = BestPathKey.Find(NextKey);
                const bool bBetterCost = !ExistingCost || CandidateCost < *ExistingCost;
                const bool bEqualCostBetterPath = ExistingCost && ExistingPath
                    && FMath::IsNearlyEqual(CandidateCost, *ExistingCost)
                    && CandidatePathKey < *ExistingPath;
                if (!bBetterCost && !bEqualCostBetterPath)
                {
                    continue;
                }

                BestCost.Add(NextKey, CandidateCost);
                BestPathKey.Add(NextKey, CandidatePathKey);
                Previous.Add(NextKey, Current.LaneKey);
                Frontier.Add({NextKey, CandidateCost, CandidatePathKey});
            }
        }

        if (!bFoundGoal)
        {
            return false;
        }

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
        OutRoute.RouteSignature = PinkCabWorldId::StableToken(TEXT("route:"), SignaturePayload);
        return true;
    }

private:
    struct FFrontierEntry
    {
        FString LaneKey;
        double Cost = 0.0;
        FString PathKey;
    };

    static int32 FindBestFrontierIndex(const TArray<FFrontierEntry>& Frontier)
    {
        int32 BestIndex = 0;
        for (int32 Index = 1; Index < Frontier.Num(); ++Index)
        {
            const FFrontierEntry& Candidate = Frontier[Index];
            const FFrontierEntry& Best = Frontier[BestIndex];
            const bool bLowerCost = Candidate.Cost < Best.Cost;
            const bool bEqualCostBetterPath = FMath::IsNearlyEqual(Candidate.Cost, Best.Cost)
                && Candidate.PathKey < Best.PathKey;
            if (bLowerCost || bEqualCostBetterPath)
            {
                BestIndex = Index;
            }
        }
        return BestIndex;
    }
};
