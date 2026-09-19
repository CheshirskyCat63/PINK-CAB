#include "Taxi/PinkCabTaximeter.h"

FPinkCabFarePricingTerms FPinkCabFarePricingTerms::Metered(
    int64 InBaseMinor,
    int64 InPerKmMinor,
    int64 InPerMinuteMinor)
{
    FPinkCabFarePricingTerms Result;
    Result.Mode = EPinkCabFareMode::Metered;
    Result.BaseMinor = FMath::Max<int64>(0, InBaseMinor);
    Result.PerKmMinor = FMath::Max<int64>(0, InPerKmMinor);
    Result.PerMinuteMinor = FMath::Max<int64>(0, InPerMinuteMinor);
    return Result;
}

FPinkCabFarePricingTerms FPinkCabFarePricingTerms::OffMeter(int64 InAgreedMinor)
{
    FPinkCabFarePricingTerms Result;
    Result.Mode = EPinkCabFareMode::OffMeter;
    Result.AgreedMinor = FMath::Max<int64>(0, InAgreedMinor);
    return Result;
}

FPinkCabTaximeter::FPinkCabTaximeter(const FPinkCabFarePricingTerms& InTerms)
    : Terms(InTerms)
{
}

bool FPinkCabTaximeter::Start()
{
    if (bStarted) return false;
    bStarted = true;
    bRunning = true;
    return true;
}

bool FPinkCabTaximeter::Stop()
{
    if (!bRunning) return false;
    bRunning = false;
    return true;
}

void FPinkCabTaximeter::Tick(
    double DistanceDeltaKm,
    double DeltaSeconds,
    bool bHardPaused)
{
    if (!bRunning) return;
    DistanceKm += FMath::Max(0.0, DistanceDeltaKm);
    if (!bHardPaused)
    {
        FareSeconds += FMath::Max(0.0, DeltaSeconds);
    }
}

int64 FPinkCabTaximeter::GetFareMinor() const
{
    if (Terms.Mode == EPinkCabFareMode::OffMeter)
    {
        return Terms.AgreedMinor;
    }

    const double VariableMinor =
        DistanceKm * static_cast<double>(Terms.PerKmMinor)
        + (FareSeconds / 60.0) * static_cast<double>(Terms.PerMinuteMinor);
    return Terms.BaseMinor
        + static_cast<int64>(FMath::FloorToDouble(VariableMinor + 1.e-9));
}

bool FPinkCabTaximeter::IsRunning() const { return bRunning; }
double FPinkCabTaximeter::GetDistanceKm() const { return DistanceKm; }
double FPinkCabTaximeter::GetFareSeconds() const { return FareSeconds; }
