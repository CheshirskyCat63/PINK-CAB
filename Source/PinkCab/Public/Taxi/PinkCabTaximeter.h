#pragma once

#include "CoreMinimal.h"
#include "Taxi/PinkCabOrder.h"

struct FPinkCabFarePricingTerms
{
    static FPinkCabFarePricingTerms Metered(
        const int64 InBaseMinor,
        const int64 InPerKmMinor,
        const int64 InPerMinuteMinor)
    {
        FPinkCabFarePricingTerms Result;
        Result.Mode = EPinkCabFareMode::Metered;
        Result.BaseMinor = FMath::Max<int64>(0, InBaseMinor);
        Result.PerKmMinor = FMath::Max<int64>(0, InPerKmMinor);
        Result.PerMinuteMinor = FMath::Max<int64>(0, InPerMinuteMinor);
        return Result;
    }

    static FPinkCabFarePricingTerms OffMeter(const int64 InAgreedMinor)
    {
        FPinkCabFarePricingTerms Result;
        Result.Mode = EPinkCabFareMode::OffMeter;
        Result.AgreedMinor = FMath::Max<int64>(0, InAgreedMinor);
        return Result;
    }

    EPinkCabFareMode Mode = EPinkCabFareMode::Metered;
    int64 BaseMinor = 0;
    int64 PerKmMinor = 0;
    int64 PerMinuteMinor = 0;
    int64 AgreedMinor = 0;
};

class FPinkCabTaximeter
{
public:
    explicit FPinkCabTaximeter(const FPinkCabFarePricingTerms& InTerms)
        : Terms(InTerms) {}

    bool Start()
    {
        if (bStarted) return false;
        bStarted = true;
        bRunning = true;
        return true;
    }

    bool Stop()
    {
        if (!bRunning) return false;
        bRunning = false;
        return true;
    }

    void Tick(const double DistanceDeltaKm, const double DeltaSeconds, const bool bHardPaused)
    {
        if (!bRunning) return;
        DistanceKm += FMath::Max(0.0, DistanceDeltaKm);
        if (!bHardPaused)
        {
            FareSeconds += FMath::Max(0.0, DeltaSeconds);
        }
    }

    int64 GetFareMinor() const
    {
        if (Terms.Mode == EPinkCabFareMode::OffMeter)
        {
            return Terms.AgreedMinor;
        }

        const double VariableMinor =
            DistanceKm * static_cast<double>(Terms.PerKmMinor)
            + (FareSeconds / 60.0) * static_cast<double>(Terms.PerMinuteMinor);
        return Terms.BaseMinor + static_cast<int64>(FMath::FloorToDouble(VariableMinor + 1.e-9));
    }

    bool IsRunning() const { return bRunning; }
    double GetDistanceKm() const { return DistanceKm; }
    double GetFareSeconds() const { return FareSeconds; }

private:
    FPinkCabFarePricingTerms Terms;
    double DistanceKm = 0.0;
    double FareSeconds = 0.0;
    bool bStarted = false;
    bool bRunning = false;
};
