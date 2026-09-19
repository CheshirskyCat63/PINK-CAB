#pragma once

#include "CoreMinimal.h"
#include "Taxi/PinkCabOrder.h"

struct PINKCABTAXI_API FPinkCabFarePricingTerms
{
    static FPinkCabFarePricingTerms Metered(
        int64 InBaseMinor,
        int64 InPerKmMinor,
        int64 InPerMinuteMinor);
    static FPinkCabFarePricingTerms OffMeter(int64 InAgreedMinor);

    EPinkCabFareMode Mode = EPinkCabFareMode::Metered;
    int64 BaseMinor = 0;
    int64 PerKmMinor = 0;
    int64 PerMinuteMinor = 0;
    int64 AgreedMinor = 0;
};

class PINKCABTAXI_API FPinkCabTaximeter
{
public:
    explicit FPinkCabTaximeter(const FPinkCabFarePricingTerms& InTerms);

    bool Start();
    bool Stop();
    void Tick(
        double DistanceDeltaKm,
        double DeltaSeconds,
        bool bHardPaused);
    int64 GetFareMinor() const;
    bool IsRunning() const;
    double GetDistanceKm() const;
    double GetFareSeconds() const;

private:
    friend class FPinkCabFareRuntimeSnapshotCodec;

    FPinkCabFarePricingTerms Terms;
    double DistanceKm = 0.0;
    double FareSeconds = 0.0;
    bool bStarted = false;
    bool bRunning = false;
};
