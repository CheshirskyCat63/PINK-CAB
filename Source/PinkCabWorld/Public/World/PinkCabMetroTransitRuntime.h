#pragma once

#include "CoreMinimal.h"
#include "World/PinkCabMetroNetworkDefinition.h"

class FPinkCabMetroTransitRuntime
{
public:
    FPinkCabMetroTransitRuntime(int32 InMaxStations = 64, int32 InMaxSegments = 128)
        : Definition(InMaxStations, InMaxSegments)
    {
    }

    static FString MakeStationId(const FPinkCabCityIdentity& City, const FString& SemanticKey)
    { return FPinkCabMetroNetworkDefinition::MakeStationId(City, SemanticKey); }

    static FString MakeSegmentId(const FPinkCabCityIdentity& City, const FString& FromStationId, const FString& ToStationId, const FString& SemanticKey)
    { return FPinkCabMetroNetworkDefinition::MakeSegmentId(City, FromStationId, ToStationId, SemanticKey); }

    bool TryAddStation(const FPinkCabCityIdentity& City, const FString& SemanticKey, FString& OutStationId)
    { return Definition.TryAddStation(City, SemanticKey, OutStationId); }

    bool TryAddSegment(const FPinkCabCityIdentity& City, const FString& FromStationId, const FString& ToStationId, const FString& SemanticKey, double TravelSeconds, double WindowStartSeconds, double WindowEndSeconds, FString& OutSegmentId)
    { return Definition.TryAddSegment(City, FromStationId, ToStationId, SemanticKey, TravelSeconds, WindowStartSeconds, WindowEndSeconds, OutSegmentId); }

    bool GetOutgoingSegments(const FString& StationId, TArray<FString>& OutSegmentIds) const
    { return Definition.GetOutgoingSegments(StationId, OutSegmentIds); }

    bool Start(const FString& RouteId, const FString& SegmentId)
    {
        const FString CleanRouteId = RouteId.TrimStartAndEnd();
        const FPinkCabMetroSegment* Segment = Definition.FindSegment(SegmentId);
        if (CleanRouteId.IsEmpty() || !Segment)
        {
            return false;
        }
        ActiveRouteId = CleanRouteId;
        CurrentSegmentId = SegmentId;
        CurrentStationId = Segment->FromStationId;
        SegmentElapsedSeconds = 0.0;
        ScheduleTimeSeconds = 0.0;
        TatraState = EPinkCabMetroTatraState::Road;
        return true;
    }
    bool Advance(double DeltaSeconds)
    {
        if (CurrentSegmentId.IsEmpty() || TatraState == EPinkCabMetroTatraState::Aborted
            || !FMath::IsFinite(DeltaSeconds) || DeltaSeconds < 0.0)
        {
            return false;
        }
        const FPinkCabMetroSegment* Segment = Definition.FindSegment(CurrentSegmentId);
        if (!Segment)
        {
            return false;
        }
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

    bool RefreshTatraTransitionState()
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
        const bool bInside = SegmentElapsedSeconds >= Segment->TransitionWindowStartSeconds
            && SegmentElapsedSeconds <= Segment->TransitionWindowEndSeconds;
        TatraState = bInside
            ? EPinkCabMetroTatraState::TransitionEligible
            : EPinkCabMetroTatraState::Road;
        return bInside;
    }

    bool EnterTransitContact()
    {
        if (TatraState != EPinkCabMetroTatraState::TransitionEligible
            || CurrentSegmentId.IsEmpty())
        {
            return false;
        }
        TatraState = EPinkCabMetroTatraState::TransitContact;
        return true;
    }

    bool RequestExit()
    {
        if (TatraState != EPinkCabMetroTatraState::TransitContact)
        {
            return false;
        }
        TatraState = EPinkCabMetroTatraState::ExitPending;
        return true;
    }

    bool CompleteExit()
    {
        if (TatraState != EPinkCabMetroTatraState::ExitPending)
        {
            return false;
        }
        TatraState = EPinkCabMetroTatraState::Road;
        return true;
    }

    void Abort()
    {
        ActiveRouteId.Reset();
        CurrentStationId.Reset();
        CurrentSegmentId.Reset();
        SegmentElapsedSeconds = 0.0;
        ScheduleTimeSeconds = 0.0;
        TatraState = EPinkCabMetroTatraState::Aborted;
    }
    void Reset()
    {
        ActiveRouteId.Reset();
        CurrentStationId.Reset();
        CurrentSegmentId.Reset();
        SegmentElapsedSeconds = 0.0;
        ScheduleTimeSeconds = 0.0;
        TatraState = EPinkCabMetroTatraState::Road;
    }

    bool IsMetroDriveableForTatra() const
    {
        return TatraState == EPinkCabMetroTatraState::TransitContact;
    }

    const FString& GetActiveRouteId() const { return ActiveRouteId; }
    const FString& GetCurrentStationId() const { return CurrentStationId; }
    const FString& GetCurrentSegmentId() const { return CurrentSegmentId; }
    double GetSegmentElapsedSeconds() const { return SegmentElapsedSeconds; }
    double GetScheduleTimeSeconds() const { return ScheduleTimeSeconds; }
    EPinkCabMetroTatraState GetTatraState() const { return TatraState; }

    FString GetDefinitionSignature() const
    { return Definition.GetDefinitionSignature(); }

    FString GetReconstructionSignature() const
    {
        const FString Payload = FString::Printf(
            TEXT("%s|%s|%s|%.3f|%.3f|%d|%s"),
            *ActiveRouteId,
            *CurrentStationId,
            *CurrentSegmentId,
            SegmentElapsedSeconds,
            ScheduleTimeSeconds,
            static_cast<int32>(TatraState),
            *Definition.GetDefinitionSignature());
        return PinkCabWorldId::StableToken(TEXT("metro-state:"), Payload);
    }

private:
    FPinkCabMetroNetworkDefinition Definition;
    FString ActiveRouteId;
    FString CurrentStationId;
    FString CurrentSegmentId;
    double SegmentElapsedSeconds = 0.0;
    double ScheduleTimeSeconds = 0.0;
    EPinkCabMetroTatraState TatraState = EPinkCabMetroTatraState::Road;
};
