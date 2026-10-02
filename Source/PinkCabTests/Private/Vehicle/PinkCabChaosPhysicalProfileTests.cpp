#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Vehicle/PinkCabChaosPhysicalProfile.h"
#include "Vehicle/PinkCabThrottleResponse.h"

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
    TestEqual(TEXT("Tatra 613 wheelbase source"), Profile.WheelbaseMm.Value, 2980.0f);
    TestEqual(TEXT("Tatra 613 front track source"), Profile.FrontTrackMm.Value, 1520.0f);
    TestEqual(TEXT("Tatra 613 rear track source"), Profile.RearTrackMm.Value, 1520.0f);
    TestEqual(TEXT("wheelbase is source authority"), Profile.WheelbaseMm.Authority,
        EPinkCabPhysicalParameterAuthority::Source);
    TestEqual(TEXT("FIRST EURO boosted power target"), Profile.MaxPowerHp.Value, 250.0f);
    TestEqual(TEXT("FIRST EURO boosted torque target"), Profile.MaxTorqueNm.Value, 260.0f);
    TestEqual(TEXT("high-rev redline target"), Profile.EngineMaxRpm.Value, 8500.0f);
    TestEqual(TEXT("warm idle authority"), Profile.EngineIdleRpm.Value, 925.0f);
    TestEqual(TEXT("red-zone start preserves accepted limiter onset"),
        Profile.EngineRedZoneStartRpm.Value, 8202.5f);
    TestEqual(TEXT("hard limiter preserves accepted cut point"),
        Profile.EngineLimiterHardCutRpm.Value, 8457.5f);
    TestEqual(TEXT("damage overspeed follows 8500 operating profile"),
        Profile.EngineDamageOverspeedRpm.Value, 8500.0f);
    TestTrue(TEXT("profile RPM envelope is ordered and valid"),
        Profile.GetEngineRpmEnvelope().IsValid());
    TestEqual(TEXT("high-rev engine spins up quickly"), Profile.EngineRevUpMOI.Value, 0.17f);
    TestEqual(TEXT("human correction gives clutch-open free revs more persistence"),
        Profile.EngineRevDownRate.Value, 1500.0f);
    TestEqual(TEXT("P02 healthy clutch capacity candidate"),
        Profile.ClutchMaxTorqueNm.Value, 390.0f);
    TestEqual(TEXT("P02 clutch capacity is explicit calibration"),
        Profile.ClutchMaxTorqueNm.Authority,
        EPinkCabPhysicalParameterAuthority::Calibration);
    TestEqual(TEXT("P02 effective inertia starts from accepted rev-up MOI seed"),
        Profile.ClutchEffectiveEngineInertia.Value,
        Profile.EngineRevUpMOI.Value);
    TestEqual(TEXT("P02 clutch synchronization horizon is explicit"),
        Profile.ClutchSynchronizationTimeSeconds.Value, 0.20f);
    TestEqual(TEXT("P02 locked slip threshold is explicit"),
        Profile.ClutchLockedSlipRpm.Value, 25.0f);
    const float RedlineTorqueFactor = Profile.NormalizedTorqueCurve.Value.Last().Y;
    const float RedlinePowerHp = Profile.MaxTorqueNm.Value * RedlineTorqueFactor * Profile.EngineMaxRpm.Value / 7127.0f;
    TestTrue(TEXT("redline stays around 250 honest horsepower"), FMath::IsNearlyEqual(RedlinePowerHp, 250.0f, 5.0f));
    TestEqual(TEXT("torque is design target"), Profile.MaxTorqueNm.Authority,
        EPinkCabPhysicalParameterAuthority::DesignTarget);
    TestEqual(TEXT("terminal speed target"), Profile.TerminalTargetKmh.Value, 195.0f);
    TestEqual(TEXT("first ratio is short enough to turn excess launch torque into tire slip"),
        Profile.ForwardGearRatios.Value[0], 4.6f);
    TestEqual(TEXT("reverse ratio mirrors first for strong controllable reverse launch"),
        Profile.ReverseGearRatios.Value[0], 4.6f);
    TestTrue(TEXT("rear drive axle has lower grip than front for power oversteer"),
        Profile.RearWheel.FrictionForceMultiplier.Value
            < Profile.FrontWheel.FrictionForceMultiplier.Value);
    TestEqual(TEXT("nominal dry front grip stays near the surface reference"),
        Profile.FrontWheel.FrictionForceMultiplier.Value, 1.05f);
    TestEqual(TEXT("nominal dry rear grip stays close to front"),
        Profile.RearWheel.FrictionForceMultiplier.Value, 0.95f);
    TestEqual(TEXT("front load sensitivity uses physical reference"),
        Profile.FrontWheel.WheelLoadRatio.Value, 1.0f);
    TestEqual(TEXT("rear load sensitivity uses physical reference"),
        Profile.RearWheel.WheelLoadRatio.Value, 1.0f);
    // Use a conservative rear-heavy 60% static load budget. The actual Tatra
    // is rear-engined, so a 50/50 estimate would understate rear grip and make
    // the 50% wheelspin threshold look easier than it is in runtime.
    const float RearGripBudgetN =
        Profile.ReferenceMassKg.Value * 9.81f * 0.60f
        * Profile.RearWheel.FrictionForceMultiplier.Value;
    const float RadiusM = Profile.RearWheel.WheelRadiusCm.Value / 100.0f;
    const float AxleDriveForceAt2000N =
        Profile.MaxTorqueNm.Value * 0.95f
        * Profile.ForwardGearRatios.Value[0]
        * Profile.FinalDriveRatio.Value
        / RadiusM;
    const float QuarterPedalEngineThrottle =
        FPinkCabThrottleResponse::ToEngineThrottle(0.25f);
    const float HalfPedalEngineThrottle =
        FPinkCabThrottleResponse::ToEngineThrottle(0.50f);
    // The player must be able to meter ordinary acceleration without the rear
    // axle being permanently under-gripped. Full throttle may still exceed the
    // static budget and create power oversteer naturally.
    TestTrue(TEXT("25 percent pedal stays comfortably below rear static grip budget"),
        AxleDriveForceAt2000N * QuarterPedalEngineThrottle < RearGripBudgetN * 0.75f);
    TestTrue(TEXT("50 percent pedal is dosable instead of guaranteed by profile math to spin"),
        AxleDriveForceAt2000N * HalfPedalEngineThrottle < RearGripBudgetN * 0.95f);
    TestTrue(TEXT("full throttle can still exceed rear static grip budget"),
        AxleDriveForceAt2000N > RearGripBudgetN * 1.15f);
    TestEqual(TEXT("front steering lock target"), Profile.FrontWheel.MaxSteerAngleDeg.Value, 41.0f);
    TestFalse(TEXT("front ABS disabled"), Profile.FrontWheel.bABSEnabled.Value);
    TestFalse(TEXT("rear ABS disabled"), Profile.RearWheel.bABSEnabled.Value);
    TestFalse(TEXT("front traction control disabled"), Profile.FrontWheel.bTractionControlEnabled.Value);
    TestFalse(TEXT("rear traction control disabled"), Profile.RearWheel.bTractionControlEnabled.Value);
    TestEqual(TEXT("rear handbrake seed"), Profile.RearWheel.MaxHandBrakeTorqueNm.Value, 1700.0f);
    TestEqual(TEXT("nominal suspension is softened for visible travel"), Profile.FrontWheel.SpringRate.Value, 170.0f);
    TestEqual(TEXT("front suspension bump travel keeps pre-integration seed"), Profile.FrontWheel.SuspensionMaxRaiseCm.Value, 8.0f);
    TestEqual(TEXT("front suspension droop travel keeps pre-integration seed"), Profile.FrontWheel.SuspensionMaxDropCm.Value, 8.0f);
    TestEqual(TEXT("rear suspension bump travel keeps pre-integration seed"), Profile.RearWheel.SuspensionMaxRaiseCm.Value, 9.0f);
    TestEqual(TEXT("rear suspension droop travel keeps pre-integration seed"), Profile.RearWheel.SuspensionMaxDropCm.Value, 9.0f);
    TestEqual(TEXT("suspension damping is compliant"), Profile.FrontWheel.SuspensionDampingRatio.Value, 0.38f);
    TestEqual(TEXT("handbrake seed is calibration"), Profile.RearWheel.MaxHandBrakeTorqueNm.Authority,
        EPinkCabPhysicalParameterAuthority::Calibration);
    TestEqual(TEXT("nominal spring is softened from the wooden prototype"),
        Profile.FrontWheel.SpringRate.Value, 170.0f);
    TestEqual(TEXT("nominal suspension damping is compliant"),
        Profile.FrontWheel.SuspensionDampingRatio.Value, 0.38f);
    TestEqual(TEXT("front suspension travel matches 160 mm seed"),
        Profile.FrontWheel.SuspensionMaxRaiseCm.Value + Profile.FrontWheel.SuspensionMaxDropCm.Value, 16.0f);
    TestEqual(TEXT("rear suspension travel matches 180 mm seed"),
        Profile.RearWheel.SuspensionMaxRaiseCm.Value + Profile.RearWheel.SuspensionMaxDropCm.Value, 18.0f);
    TestFalse(TEXT("FIRST EURO gearbox is manual"), Profile.bUseAutomaticGears.Value);
    TestFalse(TEXT("FIRST EURO reverse is manually selected"), Profile.bUseAutoReverse.Value);
    return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabP03HumanFeelCorrectionContractTest,
    "PinkCab.Vehicle.Physics.P03.HumanFeelCorrectionContract",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabP03HumanFeelCorrectionContractTest::RunTest(const FString&)
{
    const FPinkCabChaosPhysicalProfile Profile =
        FPinkCabChaosPhysicalProfile::ForVariant(
            EPinkCabCalibrationVariant::Nominal);

    // Human-gate rejection 2026-10-02: the permanent 2.00/0.50 axle split
    // and WheelLoadRatio=0.38 are no longer an admissible way to manufacture
    // oversteer. Reuse the already-proven R6 physical-reference candidate.
    TestEqual(TEXT("human correction advances calibration identity"),
        Profile.CalibrationVersion, 6);
    TestEqual(TEXT("front load sensitivity uses physical reference"),
        Profile.FrontWheel.WheelLoadRatio.Value, 1.0f);
    TestEqual(TEXT("rear load sensitivity uses physical reference"),
        Profile.RearWheel.WheelLoadRatio.Value, 1.0f);
    TestEqual(TEXT("front dry grip returns near surface reference"),
        Profile.FrontWheel.FrictionForceMultiplier.Value, 1.05f);
    TestEqual(TEXT("rear dry grip stays close to front instead of wheelspin shortcut"),
        Profile.RearWheel.FrictionForceMultiplier.Value, 0.95f);
    TestTrue(TEXT("axle grip balance cannot be cliff-like"),
        Profile.RearWheel.FrictionForceMultiplier.Value
            / Profile.FrontWheel.FrictionForceMultiplier.Value >= 0.85f);

    // Q/clutch-open must feel like a free-spinning carbureted engine shedding
    // rotational energy, not like a tachometer snapping to the 925 rpm floor.
    TestEqual(TEXT("free-rev decay is deliberately slower after human rejection"),
        Profile.EngineRevDownRate.Value, 1500.0f);
    TestEqual(TEXT("warm idle remains canonical"),
        Profile.EngineIdleRpm.Value, 925.0f);

    // Correctness must come from the physical profile, never hidden assists.
    TestEqual(TEXT("steering lock remains fully available"),
        Profile.FrontWheel.MaxSteerAngleDeg.Value, 41.0f);
    TestFalse(TEXT("ABS remains disabled"),
        Profile.FrontWheel.bABSEnabled.Value
            || Profile.RearWheel.bABSEnabled.Value);
    TestFalse(TEXT("traction control remains disabled"),
        Profile.FrontWheel.bTractionControlEnabled.Value
            || Profile.RearWheel.bTractionControlEnabled.Value);
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
    TestTrue(TEXT("rear friction calibration rises low to nominal"),
        Low.RearWheel.FrictionForceMultiplier.Value < Nominal.RearWheel.FrictionForceMultiplier.Value);
    TestTrue(TEXT("rear friction calibration rises nominal to high"),
        Nominal.RearWheel.FrictionForceMultiplier.Value < High.RearWheel.FrictionForceMultiplier.Value);
    TestTrue(TEXT("spring calibration rises low to nominal"),
        Low.FrontWheel.SpringRate.Value < Nominal.FrontWheel.SpringRate.Value);
    TestTrue(TEXT("spring calibration rises nominal to high"),
        Nominal.FrontWheel.SpringRate.Value < High.FrontWheel.SpringRate.Value);
    return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabPhysicsProfileEnvelopeIdentityTest,
    "PinkCab.Vehicle.Physics.Profile.Envelope.Identity",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPinkCabPhysicsProfileEnvelopeIdentityTest::RunTest(const FString& Parameters)
{
    const FPinkCabChaosPhysicalProfile Profile =
        FPinkCabChaosPhysicalProfile::ForVariant(EPinkCabCalibrationVariant::Nominal);

    TestTrue(TEXT("profile envelope validates"), Profile.HasValidEnvelope());
    TestEqual(TEXT("model id is the current 613 donor profile only"),
        Profile.ModelId, FName(TEXT("TATRA_613")));
    TestEqual(TEXT("profile id is stable"),
        Profile.ProfileId, FName(TEXT("PINKCAB_TATRA613_CHAOS")));
    TestEqual(TEXT("schema starts at v1"), Profile.SchemaVersion, 1);
    TestEqual(TEXT("human handling correction advances profile to v6"), Profile.CalibrationVersion, 6);
    TestEqual(TEXT("unit contract id is explicit"),
        Profile.UnitSystemId, FName(TEXT("PINKCAB_PHYSICS_UNITS_V1")));
    TestEqual(TEXT("provenance set id is explicit"),
        Profile.ProvenanceSetId, FName(TEXT("PINKCAB_TATRA613_BASELINE_2026_09_26")));
    TestEqual(TEXT("compatibility id is explicit"),
        Profile.CompatibilityId, FName(TEXT("PINKCAB_CHAOS_PROFILE_V1")));
    TestEqual(TEXT("migration id is explicit"),
        Profile.MigrationId, FName(TEXT("PINKCAB_TATRA613_PROFILE_V1")));
    TestEqual(TEXT("variant identity is retained"),
        Profile.CalibrationVariant, EPinkCabCalibrationVariant::Nominal);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabPhysicsProfileEnvelopeHashTest,
    "PinkCab.Vehicle.Physics.Profile.Envelope.DeterministicHash",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPinkCabPhysicsProfileEnvelopeHashTest::RunTest(const FString& Parameters)
{
    const FPinkCabChaosPhysicalProfile NominalA =
        FPinkCabChaosPhysicalProfile::ForVariant(EPinkCabCalibrationVariant::Nominal);
    const FPinkCabChaosPhysicalProfile NominalB =
        FPinkCabChaosPhysicalProfile::ForVariant(EPinkCabCalibrationVariant::Nominal);
    const FPinkCabChaosPhysicalProfile Low =
        FPinkCabChaosPhysicalProfile::ForVariant(EPinkCabCalibrationVariant::Low);

    const uint64 HashA = NominalA.GetDeterministicProfileHash();
    const uint64 HashB = NominalB.GetDeterministicProfileHash();
    const uint64 LowHash = Low.GetDeterministicProfileHash();

    TestTrue(TEXT("profile hash is non-zero"), HashA != 0);
    TestEqual(TEXT("same profile produces same deterministic hash"), HashA, HashB);
    TestTrue(TEXT("calibration variant changes deterministic hash"), HashA != LowHash);

    FPinkCabChaosPhysicalProfile Mutated = NominalA;
    Mutated.EngineIdleRpm.Value += 1.0f;
    TestTrue(TEXT("physical parameter mutation changes deterministic hash"),
        HashA != Mutated.GetDeterministicProfileHash());

    Mutated = NominalA;
    Mutated.EngineDamageOverspeedRpm.Value += 1.0f;
    TestTrue(TEXT("RPM-envelope mutation changes deterministic hash"),
        HashA != Mutated.GetDeterministicProfileHash());

    Mutated = NominalA;
    Mutated.ClutchMaxTorqueNm.Value += 1.0f;
    TestTrue(TEXT("clutch calibration mutation changes deterministic hash"),
        HashA != Mutated.GetDeterministicProfileHash());

    Mutated = NominalA;
    Mutated.EngineIdleRpm.Authority = EPinkCabPhysicalParameterAuthority::Source;
    TestTrue(TEXT("provenance mutation changes deterministic hash"),
        HashA != Mutated.GetDeterministicProfileHash());
    return true;
}

#endif

#if WITH_DEV_AUTOMATION_TESTS
#include "ChaosWheeledVehicleMovementComponent.h"
#include "Runtime/PinkCabChaosTatraPawn.h"
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
    TestEqual(TEXT("engine redline comes from profile"), Movement->EngineSetup.MaxRPM, Profile.EngineMaxRpm.Value);
    TestEqual(TEXT("engine rev-up inertia comes from profile"), Movement->EngineSetup.EngineRevUpMOI, Profile.EngineRevUpMOI.Value);
    TestEqual(TEXT("engine rev-down rate comes from profile"), Movement->EngineSetup.EngineRevDownRate, Profile.EngineRevDownRate.Value);
    TestEqual(TEXT("front radius comes from profile"), Front->WheelRadius, Profile.FrontWheel.WheelRadiusCm.Value);
    TestEqual(TEXT("front spring comes from profile"), Front->SpringRate, Profile.FrontWheel.SpringRate.Value);
    TestEqual(TEXT("rear steer comes from profile"), Rear->MaxSteerAngle, Profile.RearWheel.MaxSteerAngleDeg.Value);
    TestEqual(TEXT("front handbrake torque comes from profile"), Front->MaxHandBrakeTorque, Profile.FrontWheel.MaxHandBrakeTorqueNm.Value);
    TestEqual(TEXT("rear handbrake torque comes from profile"), Rear->MaxHandBrakeTorque, Profile.RearWheel.MaxHandBrakeTorqueNm.Value);
    return true;
}
#endif
