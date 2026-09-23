#pragma once

#include "CoreMinimal.h"
#include "World/PinkCabCityIdentity.h"

enum class EPinkCabMetroTatraState : uint8
{
    Road,
    TransitionEligible,
    TransitContact,
    ExitPending,
    Aborted
};

struct PINKCABWORLD_API FPinkCabMetroStation
{
    FString StationId;
    FString SemanticKey;
    bool IsValid() const;
};

struct PINKCABWORLD_API FPinkCabMetroSegment
{
    FString SegmentId;
    FString FromStationId;
    FString ToStationId;
    FString SemanticKey;
    double TravelSeconds = 0.0;
    double TransitionWindowStartSeconds = 0.0;
    double TransitionWindowEndSeconds = 0.0;
    bool IsValid() const;
};

class PINKCABWORLD_API FPinkCabMetroNetworkDefinition
{
public:
    FPinkCabMetroNetworkDefinition(int32 InMaxStations = 64, int32 InMaxSegments = 128);
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
    const FPinkCabMetroSegment* FindSegment(const FString& SegmentId) const;
    FString GetDefinitionSignature() const;

private:
    int32 MaxStations = 64;
    int32 MaxSegments = 128;
    TMap<FString, FPinkCabMetroStation> Stations;
    TMap<FString, FPinkCabMetroSegment> Segments;
    TMap<FString, TArray<FString>> OutgoingByStation;
};
