#include "Vehicle/PinkCabVehicleMassProperties.h"
#include "Vehicle/PinkCabVehicleLoadState.h"
#include "Chaos/MassProperties.h"

namespace
{
bool AddMassElement(TArray<Chaos::FMassProperties>& Parts,
    const double MassKg, const FVector& Center, const FVector& Size)
{
    if (!FMath::IsFinite(MassKg) || MassKg < 0.0 || Center.ContainsNaN()
        || Size.ContainsNaN() || Size.GetMin() <= 0.0) return false;
    if (MassKg == 0.0) return true;
    Chaos::FMassProperties Part;
    Part.Mass = MassKg;
    Part.CenterOfMass = Chaos::FVec3(Center);
    Part.Volume = Size.X * Size.Y * Size.Z;
    Chaos::CalculateInertiaAndRotationOfMass(FBox(-Size / 2.0, Size / 2.0),
        MassKg / Part.Volume, Part.InertiaTensor, Part.RotationOfMass);
    Parts.Add(Part);
    return true;
}
}

bool FPinkCabVehicleLoadState::TryGetMassProperties(
    const FPinkCabTatraProfile& Profile, FPinkCabVehicleMassProperties& Out) const
{
    Out = {};
    const double ChassisMass = Profile.BaseVehicleMassKg - Profile.RearAssemblyMassKg;
    if (!FMath::IsFinite(ChassisMass) || ChassisMass <= 0.0) return false;
    // Rear assembly is one part of the base, not extra mass on top of it.
    const double ChassisX =
        (Profile.BaseLongitudinalCm * Profile.BaseVehicleMassKg
         - Profile.RearAssemblyCenterCm.X * Profile.RearAssemblyMassKg) / ChassisMass;
    TArray<Chaos::FMassProperties> Parts;
    Parts.Reserve(5 + Passengers.Num() + FarePassengers.Num());
    bool bValid = AddMassElement(Parts, ChassisMass,
        FVector(ChassisX, 0.0, Profile.ChassisCenterHeightCm), Profile.ChassisSizeCm);
    bValid &= AddMassElement(Parts, Profile.RearAssemblyMassKg,
        Profile.RearAssemblyCenterCm, Profile.RearAssemblySizeCm);
    bValid &= AddMassElement(Parts, HeroineMassKg,
        FVector(Profile.HeroineLongitudinalCm, -Profile.CrewHalfTrackCm, Profile.CrewCenterHeightCm),
        Profile.OccupantSizeCm);
    bValid &= AddMassElement(Parts, DaughterMassKg,
        FVector(Profile.DaughterLongitudinalCm, Profile.CrewHalfTrackCm, Profile.CrewCenterHeightCm),
        Profile.OccupantSizeCm);
    bValid &= AddMassElement(Parts, FuelMassKg,
        FVector(FuelLongitudinalCm, FuelLateralCm, FuelVerticalCm), Profile.FuelSizeCm);
    for (const FPinkCabVehicleLoadItem& Item : Passengers)
    {
        bValid &= AddMassElement(Parts, Item.MassKg,
            FVector(Item.LongitudinalCm, Item.LateralCm, Item.VerticalCm), Profile.OccupantSizeCm);
    }
    for (const FPinkCabVehicleLoadItem& Item : FarePassengers)
    {
        bValid &= AddMassElement(Parts, Item.MassKg,
            FVector(Item.LongitudinalCm, Item.LateralCm, Item.VerticalCm), Profile.OccupantSizeCm);
    }
    if (!bValid || Parts.IsEmpty()) return false;
    // Native parallel-axis combination and principal-axis diagonalization.
    const Chaos::FMassProperties Combined = Chaos::Combine(Parts);
    FPinkCabVehicleMassProperties Result;
    Result.MassKg = Combined.Mass;
    Result.CenterCm = FVector(Combined.CenterOfMass);
    Result.PrincipalInertiaKgCm2 = FVector(Combined.InertiaTensor.GetDiagonal());
    Result.PrincipalRotation = FQuat(Combined.RotationOfMass);
    if (!Result.IsValid()
        || !FMath::IsNearlyEqual(Result.MassKg, double(GetTotalMassKg(Profile)), 0.01)) return false;
    Out = Result;
    return true;
}

bool FPinkCabVehicleMassProperties::IsValid() const
{
    return FMath::IsFinite(MassKg) && MassKg > 0.0
        && !CenterCm.ContainsNaN() && !PrincipalInertiaKgCm2.ContainsNaN()
        && PrincipalInertiaKgCm2.GetMin() > 0.0
        && !PrincipalRotation.ContainsNaN() && PrincipalRotation.IsNormalized();
}
