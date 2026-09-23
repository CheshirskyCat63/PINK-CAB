#pragma once

#include "CoreMinimal.h"
#include "World/PinkCabCityIdentity.h"

enum class EPinkCabSuspendedBusPhase : uint8
{
    Idle,
    Moving,
    ObstacleWindow,
    Completed,
    Aborted
};

struct FPinkCabSuspendedBusSegment
{
    FString SegmentId;
    double LengthCm = 0.0;
    bool bTatraContactEligible = false;
};

struct FPinkCabSuspendedBusObstacle
{
    FString ObstacleId;
    FString SegmentId;
    double StartCm = 0.0;
    double EndCm = 0.0;
};

class PINKCABWORLD_API FPinkCabSuspendedBusRuntime
{
public:
    FPinkCabSuspendedBusRuntime(int32 InMaxSegments = 64, int32 InMaxObstacles = 64);
    bool TryAddSegment(const FString& SegmentId, double LengthCm, bool bContactEligible);
    bool TryAddObstacle(
        const FString& ObstacleId,
        const FString& SegmentId,
        double StartCm,
        double EndCm);
    bool Start(const FString& RouteId, double SpeedCmPerSec);
    bool Advance(double DeltaSeconds);
    void Abort();
    void Reset();
    FString GetCurrentSegmentId() const;
    double GetDistanceOnSegmentCm() const;
    double GetSpeedCmPerSec() const;
    EPinkCabSuspendedBusPhase GetPhase() const;
    bool HasSteeringAuthority() const;
    bool HasBrakingAuthority() const;
    bool IsTatraContactEligible() const;
    bool GetObstaclesForSegment(
        const FString& SegmentId,
        TArray<FPinkCabSuspendedBusObstacle>& OutObstacles) const;
    FString GetReconstructionSignature() const;

private:
    void RefreshPhase();

    int32 MaxSegments = 64;
    int32 MaxObstacles = 64;
    TArray<FPinkCabSuspendedBusSegment> Segments;
    TMap<FString, int32> SegmentIndexById;
    TMap<FString, FPinkCabSuspendedBusObstacle> Obstacles;
    FString ActiveRouteId;
    double Speed = 0.0;
    int32 CurrentSegmentIndex = INDEX_NONE;
    double DistanceOnSegmentCm = 0.0;
    EPinkCabSuspendedBusPhase Phase = EPinkCabSuspendedBusPhase::Idle;
};
