#pragma once

#include "CoreMinimal.h"

struct FPinkCabTrafficProxyCandidate
{
    FString TrafficId;
    double DistanceSquared = 0.0;

    bool IsValid() const
    {
        return !TrafficId.IsEmpty() && FMath::IsFinite(DistanceSquared) && DistanceSquared >= 0.0;
    }
};

struct FPinkCabTrafficProxyAssignment
{
    FString TrafficId;
    int32 SlotIndex = INDEX_NONE;
};

struct FPinkCabTrafficInteractionBubbleResult
{
    TArray<FPinkCabTrafficProxyAssignment> Assignments;
    TArray<FString> DemotedTrafficIds;

    void Reset()
    {
        Assignments.Reset();
        DemotedTrafficIds.Reset();
    }
};

class FPinkCabTrafficInteractionBubble
{
public:
    explicit FPinkCabTrafficInteractionBubble(int32 InCapacity)
        : Capacity(FMath::Max(1, InCapacity))
    {
    }    bool Update(
        const TArray<FPinkCabTrafficProxyCandidate>& Candidates,
        FPinkCabTrafficInteractionBubbleResult& OutResult)
    {
        OutResult.Reset();
        TArray<FPinkCabTrafficProxyCandidate> Sorted;
        Sorted.Reserve(Candidates.Num());
        TSet<FString> SeenIds;
        for (const FPinkCabTrafficProxyCandidate& Candidate : Candidates)
        {
            if (!Candidate.IsValid() || SeenIds.Contains(Candidate.TrafficId))
            {
                return false;
            }
            SeenIds.Add(Candidate.TrafficId);
            Sorted.Add(Candidate);
        }

        Sorted.Sort([](const FPinkCabTrafficProxyCandidate& A, const FPinkCabTrafficProxyCandidate& B)
        {
            if (!FMath::IsNearlyEqual(A.DistanceSquared, B.DistanceSquared))
            {
                return A.DistanceSquared < B.DistanceSquared;
            }
            return A.TrafficId < B.TrafficId;
        });
        const int32 SelectedCount = FMath::Min(Capacity, Sorted.Num());
        TSet<FString> SelectedIds;
        for (int32 Index = 0; Index < SelectedCount; ++Index)
        {
            SelectedIds.Add(Sorted[Index].TrafficId);
        }

        TMap<FString, int32> NextAssignments;
        TSet<int32> UsedSlots;
        for (const TPair<FString, int32>& Existing : AssignedSlots)
        {
            if (SelectedIds.Contains(Existing.Key))
            {
                NextAssignments.Add(Existing.Key, Existing.Value);
                UsedSlots.Add(Existing.Value);
            }
            else
            {
                OutResult.DemotedTrafficIds.Add(Existing.Key);
            }
        }
        OutResult.DemotedTrafficIds.Sort();

        for (int32 Index = 0; Index < SelectedCount; ++Index)
        {
            const FString& TrafficId = Sorted[Index].TrafficId;
            if (NextAssignments.Contains(TrafficId))
            {
                continue;
            }
            int32 FreeSlot = INDEX_NONE;
            for (int32 Slot = 0; Slot < Capacity; ++Slot)
            {
                if (!UsedSlots.Contains(Slot))
                {
                    FreeSlot = Slot;
                    break;
                }
            }
            if (FreeSlot == INDEX_NONE)
            {
                return false;
            }
            NextAssignments.Add(TrafficId, FreeSlot);
            UsedSlots.Add(FreeSlot);
        }

        AssignedSlots = MoveTemp(NextAssignments);
        OutResult.Assignments.Reserve(AssignedSlots.Num());
        for (int32 Index = 0; Index < SelectedCount; ++Index)
        {
            const FString& TrafficId = Sorted[Index].TrafficId;
            OutResult.Assignments.Add({TrafficId, AssignedSlots.FindRef(TrafficId)});
        }
        return true;
    }

    int32 NumAssigned() const { return AssignedSlots.Num(); }

private:
    int32 Capacity = 1;
    TMap<FString, int32> AssignedSlots;
};
