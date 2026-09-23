#include "Traffic/PinkCabTrafficFlow.h"

bool FPinkCabTrafficFlowConstraints::IsValid() const
{
    return MaxLogicalEntities > 0
        && MinPlayableGapCm > 0.0
        && FMath::IsFinite(MinPlayableGapCm);
}

FPinkCabTrafficFlow::FPinkCabTrafficFlow(
    const FPinkCabTrafficFlowConstraints& InConstraints)
    : Constraints(InConstraints)
{
}

bool FPinkCabTrafficFlow::TryAddOrdinary(
    const FPinkCabTrafficEntity& Entity)
{
    if (!Constraints.IsValid()
        || !Entity.IsValidLogical()
        || Entity.GetSpeedCmPerSec() <= 0.0
        || Entities.Num() >= Constraints.MaxLogicalEntities)
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

void FPinkCabTrafficFlow::AdvanceAll(double DeltaSeconds)
{
    for (FPinkCabTrafficEntity& Entity : Entities)
    {
        Entity.Advance(DeltaSeconds);
    }
}

bool FPinkCabTrafficFlow::AdvanceAllAlongRoutes(
    const FPinkCabRoadGraph& Graph,
    double DeltaSeconds)
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

bool FPinkCabTrafficFlow::TryGetEntity(
    const FString& TrafficId,
    FPinkCabTrafficEntity& OutEntity) const
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

int32 FPinkCabTrafficFlow::Num() const { return Entities.Num(); }
