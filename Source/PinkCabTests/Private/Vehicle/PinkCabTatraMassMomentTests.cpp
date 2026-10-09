#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Vehicle/PinkCabTatraProfile.h"
#include "Vehicle/PinkCabVehicleLoadState.h"
#include "Vehicle/PinkCabChaosLoadBridge.h"
#include "ChaosWheeledVehicleMovementComponent.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPinkCabTatraReferenceMassMomentTest,
    "PinkCab.Vehicle.PhysicalFoundation.ReferenceMassMoment",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPinkCabTatraReferenceMassMomentTest::RunTest(const FString& Parameters)
{
    const FPinkCabTatraProfile Profile = FPinkCabTatraProfile::Canonical();
    FPinkCabVehicleLoadState Empty;
    const float EmptyCg = Empty.GetLongitudinalCgInputCm(Profile);
    TestTrue(TEXT("empty base includes the rear-engine mass moment"), EmptyCg < -1.0f);

    FPinkCabVehicleLoadState Reference;
    Reference.SetFuelMassKg(Profile.FullFuelMassKg);
    Reference.SetCrew(Profile.HeroineMassKg, Profile.DaughterMassKg);
    const float ReferenceCg = Reference.GetLongitudinalCgInputCm(Profile);
    // Declared project reference, not a historical factory measurement:
    // front axle +135 cm, wheelbase 298 cm, rear support fraction 0.55.
    TestTrue(TEXT("reference mass moment agrees with the declared axle geometry"),
        FMath::IsNearlyEqual(ReferenceCg, 135.0f - 298.0f * 0.55f, 0.05f));
    TestTrue(TEXT("front crew and fuel move the rear-heavy empty COM forward"),
        ReferenceCg > EmptyCg);
    TestEqual(TEXT("engine is included in base, not added a second time"),
        Reference.GetTotalMassKg(Profile), 1657.0f);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPinkCabTatraCrewMassMomentTest,
    "PinkCab.Vehicle.PhysicalFoundation.CrewMassMoment",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPinkCabTatraCrewMassMomentTest::RunTest(const FString& Parameters)
{
    const FPinkCabTatraProfile Profile = FPinkCabTatraProfile::Canonical();
    FPinkCabVehicleLoadState Empty;
    FPinkCabVehicleLoadState Crewed;
    Crewed.SetCrew(58.0f, 49.0f);
    const double AddedMoment =
        Crewed.GetLongitudinalCgInputCm(Profile) * Crewed.GetTotalMassKg(Profile)
        - Empty.GetLongitudinalCgInputCm(Profile) * Empty.GetTotalMassKg(Profile);
    // Accepted RIG24 seat geometry transformed by the existing visual frame.
    const double SeatX = 5.5167 + 95.9121 * 0.04833435;
    TestTrue(TEXT("crew adds its own nonzero moment instead of only denominator mass"),
        FMath::IsNearlyEqual(AddedMoment, (58.0 + 49.0) * SeatX, 0.1));

    FPinkCabVehicleLoadState Fuel;
    Fuel.SetFuelMassKg(40.0f, 75.0f);
    const double FuelMoment = Fuel.GetLongitudinalCgInputCm(Profile) * Fuel.GetTotalMassKg(Profile)
        - Empty.GetLongitudinalCgInputCm(Profile) * Empty.GetTotalMassKg(Profile);
    TestTrue(TEXT("explicit fuel position contributes exactly once"),
        FMath::IsNearlyEqual(FuelMoment, 40.0 * 75.0, 0.1));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPinkCabTatraThreeAxisMassMomentTest,
    "PinkCab.Vehicle.PhysicalFoundation.ThreeAxisMassMoment",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPinkCabTatraThreeAxisMassMomentTest::RunTest(const FString& Parameters)
{
    const FPinkCabTatraProfile Profile = FPinkCabTatraProfile::Canonical();
    UChaosWheeledVehicleMovementComponent* Movement = NewObject<UChaosWheeledVehicleMovementComponent>();
    FPinkCabVehicleLoadState Left, Right;
    Left.SetCrew(58.0f, 0.0f);
    Right.SetCrew(0.0f, 58.0f);
    TestTrue(TEXT("left occupant fixture applies"), FPinkCabChaosLoadBridge::Apply(Left, Profile, *Movement));
    const FVector LeftCom = Movement->CenterOfMassOverride;
    TestTrue(TEXT("driver on left has a left lateral mass moment"), LeftCom.Y < -0.1);
    TestTrue(TEXT("mass centre is above underbody, not at ground origin"), LeftCom.Z > 20.0 && LeftCom.Z < 120.0);
    TestTrue(TEXT("right occupant fixture applies"), FPinkCabChaosLoadBridge::Apply(Right, Profile, *Movement));
    const FVector RightCom = Movement->CenterOfMassOverride;
    TestTrue(TEXT("equal occupant on right reverses lateral mass moment"), RightCom.Y > 0.1);
    TestTrue(TEXT("equal left/right loads preserve longitudinal and vertical centres"),
        FMath::IsNearlyEqual(LeftCom.X, RightCom.X, 0.001)
        && FMath::IsNearlyEqual(LeftCom.Z, RightCom.Z, 0.001));
    return true;
}

#endif
