#pragma once

#include "CoreMinimal.h"
#include "World/PinkCabMetroNetworkDefinition.h"

class PINKCABWORLD_API FPinkCabMetroTransitRuntime
{
public:
    FPinkCabMetroTransitRuntime(int32 InMaxStations = 64, int32 InMaxSegments = 128);

    static FString MakeStationId(const FPinkCabCityIdentity& City, const FString& SemanticKey);
    static FString MakeSegmentId(
        const FPinkCabCityIdentity& City,
        const FString& FromStationId,
        const FString& ToStationId,
        const FString& SemanticKey);
    bool TryAddStation(
        const FPinkCabCityIdentity& City,
        const FString& SemanticKey,
        FString& OutStationId);
    bool TryAddSegment(
        const FPinkCabCityIdentity& City,
        const FString& FromStationId,
        const FString& ToStationId,
        const FString& SemanticKey,
        double TravelSeconds,
        double WindowStartSeconds,
        double WindowEndSeconds,
        FString& OutSegmentId);
    bool GetOutgoingSegments(const FString& StationId, TArray<FString>& OutSegmentIds) const;
    bool Start(const FString& RouteId, const FString& SegmentId);
    bool Advance(double DeltaSeconds);
    bool RefreshTatraTransitionState();
    bool EnterTransitContact();
    bool RequestExit();
    bool CompleteExit();
    void Abort();
    void Reset();
    bool IsMetroDriveableForTatra() const;
    const FString& GetActiveRouteId() const;
    const FString& GetCurrentStationId() const;
    const FString& GetCurrentSegmentId() const;
    double GetSegmentElapsedSeconds() const;
    double GetScheduleTimeSeconds() const;
    EPinkCabMetroTatraState GetTatraState() const;
    FString GetDefinitionSignature() const;
    FString GetReconstructionSignature() const;

private:
    FPinkCabMetroNetworkDefinition Definition;
    FString ActiveRouteId;
    FString CurrentStationId;
    FString CurrentSegmentId;
    double SegmentElapsedSeconds = 0.0;
    double ScheduleTimeSeconds = 0.0;
    EPinkCabMetroTatraState TatraState = EPinkCabMetroTatraState::Road;
};
