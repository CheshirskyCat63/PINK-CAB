#pragma once

#include "CoreMinimal.h"
#include "World/PinkCabRoadGraph.h"

struct FPinkCabTrafficEntity
{
    FPinkCabTrafficEntity() = default;
    FPinkCabTrafficEntity(
        const FString& InTrafficId,
        const FPinkCabLaneId& InLaneId,
        double InLongitudinalCm,
        double InSpeedCmPerSec)
        : TrafficId(InTrafficId)
        , LaneId(InLaneId)
        , LongitudinalCm(InLongitudinalCm)
        , SpeedCmPerSec(InSpeedCmPerSec)
    {
    }

    bool IsValidLogical() const
    {
        return !TrafficId.IsEmpty() && LaneId.IsValid() && FMath::IsFinite(LongitudinalCm) && FMath::IsFinite(SpeedCmPerSec);
    }

    void Advance(double DeltaSeconds)
    {
        if (DeltaSeconds > 0.0 && FMath::IsFinite(DeltaSeconds))
        {
            LongitudinalCm += SpeedCmPerSec * DeltaSeconds;
        }
    }

    double GetLongitudinalCm() const { return LongitudinalCm; }
    double GetSpeedCmPerSec() const { return SpeedCmPerSec; }
    const FString& GetTrafficId() const { return TrafficId; }
    const FPinkCabLaneId& GetLaneId() const { return LaneId; }

private:
    FString TrafficId;
    FPinkCabLaneId LaneId;
    double LongitudinalCm = 0.0;
    double SpeedCmPerSec = 0.0;
};
