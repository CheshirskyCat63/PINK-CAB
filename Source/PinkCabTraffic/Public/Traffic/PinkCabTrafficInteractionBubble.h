#pragma once

#include "CoreMinimal.h"

struct PINKCABTRAFFIC_API FPinkCabTrafficProxyCandidate
{
    FString TrafficId;
    double DistanceSquared = 0.0;

    bool IsValid() const;
};

struct FPinkCabTrafficProxyAssignment
{
    FString TrafficId;
    int32 SlotIndex = INDEX_NONE;
};

struct PINKCABTRAFFIC_API FPinkCabTrafficInteractionBubbleResult
{
    TArray<FPinkCabTrafficProxyAssignment> Assignments;
    TArray<FString> DemotedTrafficIds;

    void Reset();
};

class PINKCABTRAFFIC_API FPinkCabTrafficInteractionBubble
{
public:
    explicit FPinkCabTrafficInteractionBubble(int32 InCapacity);

    bool Update(
        const TArray<FPinkCabTrafficProxyCandidate>& Candidates,
        FPinkCabTrafficInteractionBubbleResult& OutResult);
    int32 NumAssigned() const;

private:
    int32 Capacity = 1;
    TMap<FString, int32> AssignedSlots;
};
