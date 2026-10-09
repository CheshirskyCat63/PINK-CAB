#include "Vehicle/PinkCabTatraProfile.h"

FPinkCabTatraProfile FPinkCabTatraProfile::Canonical()
{
    FPinkCabTatraProfile Result;
    Result.BaseVehicleMassKg = 1450.0f;
    Result.FullFuelMassKg = 100.0f;
    Result.HeroineMassKg = 58.0f;
    Result.DaughterMassKg = 49.0f;
    Result.DeclaredMaxFixtureKg = 2107.0f;
    // Mass-distribution calibration v2. Keep the accepted physical root frame:
    // front axle +135 cm, wheelbase 298 cm, project reference 45/55 F/R.
    // Seat geometry is a calibration seed for occupied mass, not a factory COM.
    Result.HeroineLongitudinalCm = 5.5167f + 95.9121f * 0.04833435f;
    Result.DaughterLongitudinalCm = Result.HeroineLongitudinalCm;
    const float ReferenceCgCm = 135.0f - 298.0f * 0.55f;
    Result.BaseLongitudinalCm =
        (ReferenceCgCm * Result.GetReferenceCrewMassKg()
         - Result.HeroineMassKg * Result.HeroineLongitudinalCm
         - Result.DaughterMassKg * Result.DaughterLongitudinalCm)
        / Result.BaseVehicleMassKg;
    // The rear engine is INCLUDED in this aggregate base; never add it again.
    // The existing reference fuel longitudinal datum remains 0 cm.
    // Explicit mass/inertia calibration seeds, not measured factory values.
    Result.RearAssemblyMassKg = 275.0f; // contained within the 1450 kg base
    Result.RearAssemblyCenterCm = FVector(-210.0, 0.0, 50.0);
    Result.RearAssemblySizeCm = FVector(110.0, 95.0, 65.0);
    Result.ChassisCenterHeightCm = 45.0f;
    Result.ChassisSizeCm = FVector(485.137f, 195.149f, 70.0f);
    Result.CrewHalfTrackCm = 47.74276f;
    Result.CrewCenterHeightCm = 73.07945f;
    Result.OccupantSizeCm = FVector(45.0, 40.0, 80.0);
    Result.FuelSizeCm = FVector(80.0, 100.0, 20.0);
    return Result;
}
