#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Vehicle/PinkCabChaosPhysicalProfile.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabChaosPhysicalProfileAuthorityTest,
    "PinkCab.Vehicle.ChaosCalibration.Profile.Authority",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPinkCabChaosPhysicalProfileAuthorityTest::RunTest(const FString& Parameters)
{
    const FPinkCabChaosPhysicalProfile Profile =
        FPinkCabChaosPhysicalProfile::ForVariant(EPinkCabCalibrationVariant::Nominal);

    TestTrue(TEXT("all exposed physical fields have provenance"), Profile.HasCompleteProvenance());
    TestEqual(TEXT("reference service mass"), Profile.ReferenceMassKg.Value, 1657.0f);
    TestEqual(TEXT("mass is product design authority"), Profile.ReferenceMassKg.Authority,
        EPinkCabPhysicalParameterAuthority::DesignTarget);
    TestEqual(TEXT("historical wheelbase source"), Profile.WheelbaseMm.Value, 2750.0f);
    TestEqual(TEXT("wheelbase is source authority"), Profile.WheelbaseMm.Authority,
        EPinkCabPhysicalParameterAuthority::Source);
    TestEqual(TEXT("Tatra torque target"), Profile.MaxTorqueNm.Value, 240.0f);
    TestEqual(TEXT("torque is design target"), Profile.MaxTorqueNm.Authority,
        EPinkCabPhysicalParameterAuthority::DesignTarget);
    TestEqual(TEXT("terminal speed target"), Profile.TerminalTargetKmh.Value, 195.0f);
    TestEqual(TEXT("front steering lock target"), Profile.FrontWheel.MaxSteerAngleDeg.Value, 41.0f);
    TestFalse(TEXT("front ABS disabled"), Profile.FrontWheel.bABSEnabled.Value);
    TestFalse(TEXT("rear ABS disabled"), Profile.RearWheel.bABSEnabled.Value);
    TestFalse(TEXT("front traction control disabled"), Profile.FrontWheel.bTractionControlEnabled.Value);
    TestFalse(TEXT("rear traction control disabled"), Profile.RearWheel.bTractionControlEnabled.Value);
    TestEqual(TEXT("rear handbrake seed"), Profile.RearWheel.MaxHandBrakeTorqueNm.Value, 1700.0f);
    TestEqual(TEXT("handbrake seed is calibration"), Profile.RearWheel.MaxHandBrakeTorqueNm.Authority,
        EPinkCabPhysicalParameterAuthority::Calibration);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabChaosPhysicalProfileVariantTest,
    "PinkCab.Vehicle.ChaosCalibration.Profile.Variants",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPinkCabChaosPhysicalProfileVariantTest::RunTest(const FString& Parameters)
{
    const auto Low = FPinkCabChaosPhysicalProfile::ForVariant(EPinkCabCalibrationVariant::Low);
    const auto Nominal = FPinkCabChaosPhysicalProfile::ForVariant(EPinkCabCalibrationVariant::Nominal);
    const auto High = FPinkCabChaosPhysicalProfile::ForVariant(EPinkCabCalibrationVariant::High);

    TestEqual(TEXT("source wheelbase invariant low/nominal"), Low.WheelbaseMm.Value, Nominal.WheelbaseMm.Value);
    TestEqual(TEXT("source wheelbase invariant nominal/high"), Nominal.WheelbaseMm.Value, High.WheelbaseMm.Value);
    TestEqual(TEXT("design torque invariant low/nominal"), Low.MaxTorqueNm.Value, Nominal.MaxTorqueNm.Value);
    TestEqual(TEXT("design torque invariant nominal/high"), Nominal.MaxTorqueNm.Value, High.MaxTorqueNm.Value);
    TestEqual(TEXT("design steering invariant"), Low.FrontWheel.MaxSteerAngleDeg.Value, High.FrontWheel.MaxSteerAngleDeg.Value);

    TestTrue(TEXT("friction calibration rises low to nominal"),
        Low.FrontWheel.FrictionForceMultiplier.Value < Nominal.FrontWheel.FrictionForceMultiplier.Value);
    TestTrue(TEXT("friction calibration rises nominal to high"),
        Nominal.FrontWheel.FrictionForceMultiplier.Value < High.FrontWheel.FrictionForceMultiplier.Value);
    TestTrue(TEXT("spring calibration rises low to nominal"),
        Low.FrontWheel.SpringRate.Value < Nominal.FrontWheel.SpringRate.Value);
    TestTrue(TEXT("spring calibration rises nominal to high"),
        Nominal.FrontWheel.SpringRate.Value < High.FrontWheel.SpringRate.Value);
    return true;
}

#endif

#if WITH_DEV_AUTOMATION_TESTS
#include "ChaosWheeledVehicleMovementComponent.h"
#include "Vehicle/PinkCabChaosTatraPawn.h"
#include "Vehicle/PinkCabChaosWheelFront.h"
#include "Vehicle/PinkCabChaosWheelRear.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabChaosPhysicalProfileAppliedDefaultsTest,
    "PinkCab.Vehicle.ChaosCalibration.Profile.AppliedDefaults",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPinkCabChaosPhysicalProfileAppliedDefaultsTest::RunTest(const FString& Parameters)
{
    const auto Profile = FPinkCabChaosPhysicalProfile::ForVariant(EPinkCabCalibrationVariant::Nominal);
    const auto* Pawn = GetDefault<APinkCabChaosTatraPawn>();
    const auto* Movement = Pawn->GetChaosMovement();
    const auto* Front = GetDefault<UPinkCabChaosWheelFront>();
    const auto* Rear = GetDefault<UPinkCabChaosWheelRear>();
    TestNotNull(TEXT("movement exists"), Movement);
    TestEqual(TEXT("pawn mass comes from profile"), Movement->Mass, Profile.ReferenceMassKg.Value);
    TestEqual(TEXT("engine torque comes from profile"), Movement->EngineSetup.MaxTorque, Profile.MaxTorqueNm.Value);
    TestEqual(TEXT("front radius comes from profile"), Front->WheelRadius, Profile.FrontWheel.WheelRadiusCm.Value);
    TestEqual(TEXT("front spring comes from profile"), Front->SpringRate, Profile.FrontWheel.SpringRate.Value);
    TestEqual(TEXT("rear steer comes from profile"), Rear->MaxSteerAngle, Profile.RearWheel.MaxSteerAngleDeg.Value);
    TestEqual(TEXT("front handbrake torque comes from profile"), Front->MaxHandBrakeTorque, Profile.FrontWheel.MaxHandBrakeTorqueNm.Value);
    TestEqual(TEXT("rear handbrake torque comes from profile"), Rear->MaxHandBrakeTorque, Profile.RearWheel.MaxHandBrakeTorqueNm.Value);
    return true;
}
#endif
