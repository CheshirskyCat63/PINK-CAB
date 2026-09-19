#pragma once

#include "CoreMinimal.h"
#include "Traffic/PinkCabTrafficEntity.h"

struct PINKCABTRAFFIC_API FPinkCabTrafficFlowConstraints
{
    int32 MaxLogicalEntities = 0;
    double MinPlayableGapCm = 0.0;

    bool IsValid() const;
};

class PINKCABTRAFFIC_API FPinkCabTrafficFlow
{
public:
    explicit FPinkCabTrafficFlow(const FPinkCabTrafficFlowConstraints& InConstraints);

    bool TryAddOrdinary(const FPinkCabTrafficEntity& Entity);
    void AdvanceAll(double DeltaSeconds);
    bool AdvanceAllAlongRoutes(const FPinkCabRoadGraph& Graph, double DeltaSeconds);
    bool TryGetEntity(const FString& TrafficId, FPinkCabTrafficEntity& OutEntity) const;
    int32 Num() const;

private:
    FPinkCabTrafficFlowConstraints Constraints;
    TArray<FPinkCabTrafficEntity> Entities;
};
