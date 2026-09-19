#include "World/PinkCabMetroTransitRuntime.h"

FPinkCabMetroTransitRuntime::FPinkCabMetroTransitRuntime(
    int32 InMaxStations,
    int32 InMaxSegments)
    : Definition(InMaxStations, InMaxSegments)
{
}

FString FPinkCabMetroTransitRuntime::MakeStationId(
    const FPinkCabCityIdentity& City,
    const FString& SemanticKey)
{
    return FPinkCabMetroNetworkDefinition::MakeStationId(City, SemanticKey);
}

FString FPinkCabMetroTransitRuntime::MakeSegmentId(
    const FPinkCabCityIdentity& City,
    const FString& FromStationId,
    const FString& ToStationId,
    const FString& SemanticKey)
{
    return FPinkCabMetroNetworkDefinition::MakeSegmentId(
        City, FromStationId, ToStationId, SemanticKey);
}

bool FPinkCabMetroTransitRuntime::TryAddStation(
    const FPinkCabCityIdentity& City,
    const FString& SemanticKey,
    FString& OutStationId)
{
    return Definition.TryAddStation(City, SemanticKey, OutStationId);
}

bool FPinkCabMetroTransitRuntime::TryAddSegment(
    const FPinkCabCityIdentity& City,
    const FString& FromStationId,
    const FString& ToStationId,
    const FString& SemanticKey,
    double TravelSeconds,
    double WindowStartSeconds,
    double WindowEndSeconds,
    FString& OutSegmentId)
{
    return Definition.TryAddSegment(
        City, FromStationId, ToStationId, SemanticKey,
        TravelSeconds, WindowStartSeconds, WindowEndSeconds, OutSegmentId);
}

bool FPinkCabMetroTransitRuntime::GetOutgoingSegments(
    const FString& StationId,
    TArray<FString>& OutSegmentIds) const
{
    return Definition.GetOutgoingSegments(StationId, OutSegmentIds);
}

bool FPinkCabMetroTransitRuntime::Start(
    const FString& RouteId,
    const FString& SegmentId)
{
    const FString CleanRouteId = RouteId.TrimStartAndEnd();
    const FPinkCabMetroSegment* Segment = Definition.FindSegment(SegmentId);
    if (CleanRouteId.IsEmpty() || !Segment) return false;

    ActiveRouteId = CleanRouteId;
    CurrentSegmentId = SegmentId;
    CurrentStationId = Segment->FromStationId;
    SegmentElapsedSeconds = 0.0;
    ScheduleTimeSeconds = 0.0;
    TatraState = EPinkCabMetroTatraState::Road;
    return true;
}

bool FPinkCabMetroTransitRuntime::Advance(double DeltaSeconds)
{
    if (CurrentSegmentId.IsEmpty()
        || TatraState == EPinkCabMetroTatraState::Aborted
        || !FMath::IsFinite(DeltaSeconds)
        || DeltaSeconds < 0.0)
    {
        return false;
    }

    const FPinkCabMetroSegment* Segment = Definition.FindSegment(CurrentSegmentId);
    if (!Segment) return false;

    const double Remaining = Segment->TravelSeconds - SegmentElapsedSeconds;
    const double Applied = FMath::Min(DeltaSeconds, FMath::Max(0.0, Remaining));
    SegmentElapsedSeconds += Applied;
    ScheduleTimeSeconds += Applied;
    if (SegmentElapsedSeconds + KINDA_SMALL_NUMBER >= Segment->TravelSeconds)
    {
        CurrentStationId = Segment->ToStationId;
        CurrentSegmentId.Reset();
        SegmentElapsedSeconds = 0.0;
        TatraState = TatraState == EPinkCabMetroTatraState::TransitContact
            ? EPinkCabMetroTatraState::ExitPending
            : EPinkCabMetroTatraState::Road;
    }
    return true;
}

bool FPinkCabMetroTransitRuntime::RefreshTatraTransitionState()
{
    if (TatraState == EPinkCabMetroTatraState::Aborted
        || TatraState == EPinkCabMetroTatraState::TransitContact
        || TatraState == EPinkCabMetroTatraState::ExitPending)
    {
        return false;
    }

    const FPinkCabMetroSegment* Segment = Definition.FindSegment(CurrentSegmentId);
    if (!Segment)
    {
        TatraState = EPinkCabMetroTatraState::Road;
        return false;
    }

    const bool bInside =
        SegmentElapsedSeconds >= Segment->TransitionWindowStartSeconds
        && SegmentElapsedSeconds <= Segment->TransitionWindowEndSeconds;
    TatraState = bInside
        ? EPinkCabMetroTatraState::TransitionEligible
        : EPinkCabMetroTatraState::Road;
    return bInside;
}

bool FPinkCabMetroTransitRuntime::EnterTransitContact()
{
    if (TatraState != EPinkCabMetroTatraState::TransitionEligible
        || CurrentSegmentId.IsEmpty())
    {
        return false;
    }
    TatraState = EPinkCabMetroTatraState::TransitContact;
    return true;
}

bool FPinkCabMetroTransitRuntime::RequestExit()
{
    if (TatraState != EPinkCabMetroTatraState::TransitContact) return false;
    TatraState = EPinkCabMetroTatraState::ExitPending;
    return true;
}

bool FPinkCabMetroTransitRuntime::CompleteExit()
{
    if (TatraState != EPinkCabMetroTatraState::ExitPending) return false;
    TatraState = EPinkCabMetroTatraState::Road;
    return true;
}

void FPinkCabMetroTransitRuntime::Abort()
{
    ActiveRouteId.Reset();
    CurrentStationId.Reset();
    CurrentSegmentId.Reset();
    SegmentElapsedSeconds = 0.0;
    ScheduleTimeSeconds = 0.0;
    TatraState = EPinkCabMetroTatraState::Aborted;
}

void FPinkCabMetroTransitRuntime::Reset()
{
    ActiveRouteId.Reset();
    CurrentStationId.Reset();
    CurrentSegmentId.Reset();
    SegmentElapsedSeconds = 0.0;
    ScheduleTimeSeconds = 0.0;
    TatraState = EPinkCabMetroTatraState::Road;
}

bool FPinkCabMetroTransitRuntime::IsMetroDriveableForTatra() const
{
    return TatraState == EPinkCabMetroTatraState::TransitContact;
}

const FString& FPinkCabMetroTransitRuntime::GetActiveRouteId() const { return ActiveRouteId; }
const FString& FPinkCabMetroTransitRuntime::GetCurrentStationId() const { return CurrentStationId; }
const FString& FPinkCabMetroTransitRuntime::GetCurrentSegmentId() const { return CurrentSegmentId; }
double FPinkCabMetroTransitRuntime::GetSegmentElapsedSeconds() const { return SegmentElapsedSeconds; }
double FPinkCabMetroTransitRuntime::GetScheduleTimeSeconds() const { return ScheduleTimeSeconds; }
EPinkCabMetroTatraState FPinkCabMetroTransitRuntime::GetTatraState() const { return TatraState; }

FString FPinkCabMetroTransitRuntime::GetDefinitionSignature() const
{
    return Definition.GetDefinitionSignature();
}

FString FPinkCabMetroTransitRuntime::GetReconstructionSignature() const
{
    const FString Payload = FString::Printf(
        TEXT("%s|%s|%s|%.3f|%.3f|%d|%s"),
        *ActiveRouteId, *CurrentStationId, *CurrentSegmentId,
        SegmentElapsedSeconds, ScheduleTimeSeconds,
        static_cast<int32>(TatraState),
        *Definition.GetDefinitionSignature());
    return PinkCabWorldId::StableToken(TEXT("metro-state:"), Payload);
}
