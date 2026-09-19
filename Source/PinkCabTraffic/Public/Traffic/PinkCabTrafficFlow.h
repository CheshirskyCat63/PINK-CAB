#pragma once

#include "CoreMinimal.h"
#include "Traffic/PinkCabTrafficEntity.h"

struct FPinkCabTrafficFlowConstraints
{
    int32 MaxLogicalEntities = 0;
    double MinPlayableGapCm = 0.0;

    bool IsValid() const
    {
        return MaxLogicalEntities > 0 && MinPlayableGapCm > 0.0
            && FMath::IsFinite(MinPlayableGapCm);
    }
};

class FPinkCabTrafficFlow
{
public:
    explicit FPinkCabTrafficFlow(const FPinkCabTrafficFlowConstraints& InConstraints)
        : Constraints(InConstraints)
    {
    }

    bool TryAddOrdinary(const FPinkCabTrafficEntity& Entity)
    {
        if (!Constraints.IsValid() || !Entity.IsValidLogical()
            || Entity.GetSpeedCmPerSec() <= 0.0)
        {
            return false;
        }
        if (Entities.Num() >= Constraints.MaxLogicalEntities)
        {
            return false;
        }
        for (const FPinkCabTrafficEntity& Existing : Entities)
        {
            if (Existing.GetTrafficId() == Entity.GetTrafficId())
            {
                return false;
            }
            if (Existing.GetLaneId() == Entity.GetLaneId())
            {
                const double Gap = FMath::Abs(
                    Existing.GetLongitudinalCm() - Entity.GetLongitudinalCm());
                if (Gap < Constraints.MinPlayableGapCm)
                {
                    return false;
                }
            }
        }
        Entities.Add(Entity);
        return true;
    }

    void AdvanceAll(double DeltaSeconds)
    {
        for (FPinkCabTrafficEntity& Entity : Entities)
        {
            Entity.Advance(DeltaSeconds);
        }
    }

    bool AdvanceAllAlongRoutes(const FPinkCabRoadGraph& Graph, double DeltaSeconds)
    {
        if (DeltaSeconds <= 0.0 || !FMath::IsFinite(DeltaSeconds))
        {
            return false;
        }
        for (FPinkCabTrafficEntity& Entity : Entities)
        {
            if (!Entity.AdvanceAlongRoute(Graph, DeltaSeconds))
            {
                return false;
            }
        }
        return true;
    }

    bool TryGetEntity(const FString& TrafficId, FPinkCabTrafficEntity& OutEntity) const
    {
        for (const FPinkCabTrafficEntity& Entity : Entities)
        {
            if (Entity.GetTrafficId() == TrafficId)
            {
                OutEntity = Entity;
                return true;
            }
        }
        return false;
    }

    int32 Num() const { return Entities.Num(); }

private:
    FPinkCabTrafficFlowConstraints Constraints;
    TArray<FPinkCabTrafficEntity> Entities;
};
