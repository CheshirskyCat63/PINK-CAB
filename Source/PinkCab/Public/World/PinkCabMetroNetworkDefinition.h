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

struct FPinkCabMetroStation
{
    FString StationId;
    FString SemanticKey;

    bool IsValid() const
    {
        return !StationId.IsEmpty() && !SemanticKey.IsEmpty();
    }
};

struct FPinkCabMetroSegment
{
    FString SegmentId;
    FString FromStationId;
    FString ToStationId;
    FString SemanticKey;
    double TravelSeconds = 0.0;
    double TransitionWindowStartSeconds = 0.0;
    double TransitionWindowEndSeconds = 0.0;

    bool IsValid() const
    {
        return !SegmentId.IsEmpty() && !FromStationId.IsEmpty()
            && !ToStationId.IsEmpty() && !SemanticKey.IsEmpty()
            && FMath::IsFinite(TravelSeconds) && TravelSeconds > 0.0
            && FMath::IsFinite(TransitionWindowStartSeconds)
            && FMath::IsFinite(TransitionWindowEndSeconds)
            && TransitionWindowStartSeconds >= 0.0
            && TransitionWindowEndSeconds >= TransitionWindowStartSeconds
            && TransitionWindowEndSeconds <= TravelSeconds;
    }
};

class FPinkCabMetroNetworkDefinition
{
public:
    FPinkCabMetroNetworkDefinition(int32 InMaxStations = 64, int32 InMaxSegments = 128)
        : MaxStations(FMath::Max(1, InMaxStations))
        , MaxSegments(FMath::Max(1, InMaxSegments))
    {
    }

    static FString MakeStationId(
        const FPinkCabCityIdentity& City,
        const FString& SemanticKey)
    {
        const FString CleanKey = SemanticKey.TrimStartAndEnd();
        if (!City.IsValid() || CleanKey.IsEmpty())
        {
            return FString();
        }
        return PinkCabWorldId::StableToken(
            TEXT("metro-station:"), City.GetStableKey() + TEXT("|") + CleanKey);
    }

    static FString MakeSegmentId(
        const FPinkCabCityIdentity& City,
        const FString& FromStationId,
        const FString& ToStationId,
        const FString& SemanticKey)
    {
        const FString CleanKey = SemanticKey.TrimStartAndEnd();
        if (!City.IsValid() || FromStationId.IsEmpty()
            || ToStationId.IsEmpty() || CleanKey.IsEmpty())
        {
            return FString();
        }
        const FString Payload = FString::Printf(
            TEXT("%s|%s|%s|%s"),
            *City.GetStableKey(), *FromStationId, *ToStationId, *CleanKey);
        return PinkCabWorldId::StableToken(TEXT("metro-segment:"), Payload);
    }

    bool TryAddStation(
        const FPinkCabCityIdentity& City,
        const FString& SemanticKey,
        FString& OutStationId)
    {
        const FString CleanKey = SemanticKey.TrimStartAndEnd();
        const FString StationId = MakeStationId(City, CleanKey);
        if (StationId.IsEmpty() || Stations.Num() >= MaxStations
            || Stations.Contains(StationId))
        {
            return false;
        }
        FPinkCabMetroStation Station;
        Station.StationId = StationId;
        Station.SemanticKey = CleanKey;
        Stations.Add(StationId, MoveTemp(Station));
        OutStationId = StationId;
        return true;
    }

    bool TryAddSegment(
        const FPinkCabCityIdentity& City,
        const FString& FromStationId,
        const FString& ToStationId,
        const FString& SemanticKey,
        double TravelSeconds,
        double WindowStartSeconds,
        double WindowEndSeconds,
        FString& OutSegmentId)
    {
        const FString CleanKey = SemanticKey.TrimStartAndEnd();
        const FString SegmentId = MakeSegmentId(
            City, FromStationId, ToStationId, CleanKey);
        if (SegmentId.IsEmpty() || !Stations.Contains(FromStationId)
            || !Stations.Contains(ToStationId) || Segments.Num() >= MaxSegments
            || Segments.Contains(SegmentId))
        {
            return false;
        }
        FPinkCabMetroSegment Segment;
        Segment.SegmentId = SegmentId;
        Segment.FromStationId = FromStationId;
        Segment.ToStationId = ToStationId;
        Segment.SemanticKey = CleanKey;
        Segment.TravelSeconds = TravelSeconds;
        Segment.TransitionWindowStartSeconds = WindowStartSeconds;
        Segment.TransitionWindowEndSeconds = WindowEndSeconds;
        if (!Segment.IsValid())
        {
            return false;
        }
        Segments.Add(SegmentId, Segment);
        OutgoingByStation.FindOrAdd(FromStationId).Add(SegmentId);
        OutSegmentId = SegmentId;
        return true;
    }

    bool GetOutgoingSegments(
        const FString& StationId,
        TArray<FString>& OutSegmentIds) const
    {
        OutSegmentIds.Reset();
        if (!Stations.Contains(StationId))
        {
            return false;
        }
        if (const TArray<FString>* Found = OutgoingByStation.Find(StationId))
        {
            OutSegmentIds = *Found;
            OutSegmentIds.Sort();
        }
        return true;
    }

    const FPinkCabMetroSegment* FindSegment(const FString& SegmentId) const
    {
        return Segments.Find(SegmentId);
    }

    FString GetDefinitionSignature() const
    {
        TArray<FString> StationKeys;
        Stations.GetKeys(StationKeys);
        StationKeys.Sort();
        FString Payload;
        for (const FString& Key : StationKeys)
        {
            const FPinkCabMetroStation& Station = Stations[Key];
            Payload += FString::Printf(
                TEXT("S:%s|%s;"), *Station.StationId, *Station.SemanticKey);
        }
        TArray<FString> SegmentKeys;
        Segments.GetKeys(SegmentKeys);
        SegmentKeys.Sort();
        for (const FString& Key : SegmentKeys)
        {
            const FPinkCabMetroSegment& Segment = Segments[Key];
            Payload += FString::Printf(
                TEXT("G:%s|%s|%s|%s|%.3f|%.3f|%.3f;"),
                *Segment.SegmentId,
                *Segment.FromStationId,
                *Segment.ToStationId,
                *Segment.SemanticKey,
                Segment.TravelSeconds,
                Segment.TransitionWindowStartSeconds,
                Segment.TransitionWindowEndSeconds);
        }
        return PinkCabWorldId::StableToken(TEXT("metro-definition:"), Payload);
    }
private:
    int32 MaxStations = 64;
    int32 MaxSegments = 128;
    TMap<FString, FPinkCabMetroStation> Stations;
    TMap<FString, FPinkCabMetroSegment> Segments;
    TMap<FString, TArray<FString>> OutgoingByStation;
};
