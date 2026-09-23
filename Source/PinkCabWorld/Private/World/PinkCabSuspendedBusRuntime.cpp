#include "World/PinkCabSuspendedBusRuntime.h"

FPinkCabSuspendedBusRuntime::FPinkCabSuspendedBusRuntime(
    int32 InMaxSegments,
    int32 InMaxObstacles)
    : MaxSegments(FMath::Max(1, InMaxSegments))
    , MaxObstacles(FMath::Max(1, InMaxObstacles))
{
}

bool FPinkCabSuspendedBusRuntime::TryAddSegment(
    const FString& SegmentId,
    double LengthCm,
    bool bContactEligible)
{
    const FString CleanId = SegmentId.TrimStartAndEnd();
    if (CleanId.IsEmpty() || !FMath::IsFinite(LengthCm) || LengthCm <= 0.0
        || Segments.Num() >= MaxSegments || SegmentIndexById.Contains(CleanId))
    {
        return false;
    }

    FPinkCabSuspendedBusSegment Segment;
    Segment.SegmentId = CleanId;
    Segment.LengthCm = LengthCm;
    Segment.bTatraContactEligible = bContactEligible;
    SegmentIndexById.Add(CleanId, Segments.Num());
    Segments.Add(MoveTemp(Segment));
    return true;
}

bool FPinkCabSuspendedBusRuntime::TryAddObstacle(
    const FString& ObstacleId,
    const FString& SegmentId,
    double StartCm,
    double EndCm)
{
    const FString CleanObstacleId = ObstacleId.TrimStartAndEnd();
    const FString CleanSegmentId = SegmentId.TrimStartAndEnd();
    const int32* SegmentIndex = SegmentIndexById.Find(CleanSegmentId);
    if (CleanObstacleId.IsEmpty() || !SegmentIndex
        || Obstacles.Num() >= MaxObstacles || Obstacles.Contains(CleanObstacleId))
    {
        return false;
    }

    const FPinkCabSuspendedBusSegment& Segment = Segments[*SegmentIndex];
    if (!FMath::IsFinite(StartCm) || !FMath::IsFinite(EndCm)
        || StartCm < 0.0 || EndCm <= StartCm || EndCm > Segment.LengthCm)
    {
        return false;
    }

    FPinkCabSuspendedBusObstacle Obstacle;
    Obstacle.ObstacleId = CleanObstacleId;
    Obstacle.SegmentId = CleanSegmentId;
    Obstacle.StartCm = StartCm;
    Obstacle.EndCm = EndCm;
    Obstacles.Add(CleanObstacleId, MoveTemp(Obstacle));
    return true;
}

bool FPinkCabSuspendedBusRuntime::Start(
    const FString& RouteId,
    double SpeedCmPerSec)
{
    const FString CleanRouteId = RouteId.TrimStartAndEnd();
    if (CleanRouteId.IsEmpty() || Segments.IsEmpty()
        || !FMath::IsFinite(SpeedCmPerSec) || SpeedCmPerSec <= 0.0)
    {
        return false;
    }
    ActiveRouteId = CleanRouteId;
    Speed = SpeedCmPerSec;
    CurrentSegmentIndex = 0;
    DistanceOnSegmentCm = 0.0;
    Phase = EPinkCabSuspendedBusPhase::Moving;
    RefreshPhase();
    return true;
}

bool FPinkCabSuspendedBusRuntime::Advance(double DeltaSeconds)
{
    if ((Phase != EPinkCabSuspendedBusPhase::Moving
            && Phase != EPinkCabSuspendedBusPhase::ObstacleWindow)
        || !FMath::IsFinite(DeltaSeconds) || DeltaSeconds < 0.0)
    {
        return false;
    }

    double RemainingCm = Speed * DeltaSeconds;
    while (RemainingCm > 0.0 && CurrentSegmentIndex < Segments.Num())
    {
        const double SegmentRemaining =
            Segments[CurrentSegmentIndex].LengthCm - DistanceOnSegmentCm;
        if (RemainingCm < SegmentRemaining)
        {
            DistanceOnSegmentCm += RemainingCm;
            RemainingCm = 0.0;
            break;
        }
        RemainingCm -= SegmentRemaining;
        ++CurrentSegmentIndex;
        DistanceOnSegmentCm = 0.0;
        if (CurrentSegmentIndex >= Segments.Num())
        {
            Phase = EPinkCabSuspendedBusPhase::Completed;
            return true;
        }
    }
    RefreshPhase();
    return true;
}

void FPinkCabSuspendedBusRuntime::Abort()
{
    ActiveRouteId.Reset();
    Speed = 0.0;
    CurrentSegmentIndex = INDEX_NONE;
    DistanceOnSegmentCm = 0.0;
    Phase = EPinkCabSuspendedBusPhase::Aborted;
}

void FPinkCabSuspendedBusRuntime::Reset()
{
    ActiveRouteId.Reset();
    Speed = 0.0;
    CurrentSegmentIndex = INDEX_NONE;
    DistanceOnSegmentCm = 0.0;
    Phase = EPinkCabSuspendedBusPhase::Idle;
}

FString FPinkCabSuspendedBusRuntime::GetCurrentSegmentId() const
{
    return Segments.IsValidIndex(CurrentSegmentIndex)
        ? Segments[CurrentSegmentIndex].SegmentId : FString();
}

double FPinkCabSuspendedBusRuntime::GetDistanceOnSegmentCm() const { return DistanceOnSegmentCm; }
double FPinkCabSuspendedBusRuntime::GetSpeedCmPerSec() const { return Speed; }
EPinkCabSuspendedBusPhase FPinkCabSuspendedBusRuntime::GetPhase() const { return Phase; }
bool FPinkCabSuspendedBusRuntime::HasSteeringAuthority() const { return false; }
bool FPinkCabSuspendedBusRuntime::HasBrakingAuthority() const { return false; }

bool FPinkCabSuspendedBusRuntime::IsTatraContactEligible() const
{
    return Segments.IsValidIndex(CurrentSegmentIndex)
        && Segments[CurrentSegmentIndex].bTatraContactEligible
        && (Phase == EPinkCabSuspendedBusPhase::Moving
            || Phase == EPinkCabSuspendedBusPhase::ObstacleWindow);
}

bool FPinkCabSuspendedBusRuntime::GetObstaclesForSegment(
    const FString& SegmentId,
    TArray<FPinkCabSuspendedBusObstacle>& OutObstacles) const
{
    OutObstacles.Reset();
    if (!SegmentIndexById.Contains(SegmentId)) return false;
    for (const TPair<FString, FPinkCabSuspendedBusObstacle>& Pair : Obstacles)
    {
        if (Pair.Value.SegmentId == SegmentId) OutObstacles.Add(Pair.Value);
    }
    OutObstacles.Sort([](
        const FPinkCabSuspendedBusObstacle& A,
        const FPinkCabSuspendedBusObstacle& B)
    {
        if (!FMath::IsNearlyEqual(A.StartCm, B.StartCm)) return A.StartCm < B.StartCm;
        return A.ObstacleId < B.ObstacleId;
    });
    return true;
}

FString FPinkCabSuspendedBusRuntime::GetReconstructionSignature() const
{
    FString Payload = ActiveRouteId + TEXT("|")
        + FString::Printf(
            TEXT("%.3f|%d|%.3f|%d;"),
            Speed, CurrentSegmentIndex, DistanceOnSegmentCm, static_cast<int32>(Phase));
    for (const FPinkCabSuspendedBusSegment& Segment : Segments)
    {
        Payload += FString::Printf(
            TEXT("S:%s|%.3f|%d;"),
            *Segment.SegmentId, Segment.LengthCm,
            Segment.bTatraContactEligible ? 1 : 0);
    }
    TArray<FString> ObstacleKeys;
    Obstacles.GetKeys(ObstacleKeys);
    ObstacleKeys.Sort();
    for (const FString& Key : ObstacleKeys)
    {
        const FPinkCabSuspendedBusObstacle& Obstacle = Obstacles[Key];
        Payload += FString::Printf(
            TEXT("O:%s|%s|%.3f|%.3f;"),
            *Obstacle.ObstacleId, *Obstacle.SegmentId,
            Obstacle.StartCm, Obstacle.EndCm);
    }
    return PinkCabWorldId::StableToken(TEXT("suspended-bus:"), Payload);
}

void FPinkCabSuspendedBusRuntime::RefreshPhase()
{
    if (!Segments.IsValidIndex(CurrentSegmentIndex)) return;
    Phase = EPinkCabSuspendedBusPhase::Moving;
    const FString& SegmentId = Segments[CurrentSegmentIndex].SegmentId;
    for (const TPair<FString, FPinkCabSuspendedBusObstacle>& Pair : Obstacles)
    {
        const FPinkCabSuspendedBusObstacle& Obstacle = Pair.Value;
        if (Obstacle.SegmentId == SegmentId
            && DistanceOnSegmentCm >= Obstacle.StartCm
            && DistanceOnSegmentCm <= Obstacle.EndCm)
        {
            Phase = EPinkCabSuspendedBusPhase::ObstacleWindow;
            return;
        }
    }
}
