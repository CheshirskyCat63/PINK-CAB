#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Persistence/PinkCabVehicleSnapshot.h"
#include "Vehicle/PinkCabTatraProfile.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabVehicleSnapshotRoundTripTest,
    "PinkCab.Persistence.VehicleSnapshot.RoundTripMassCgDamage",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabVehicleSnapshotRoundTripTest::RunTest(const FString& Parameters)
{
    FPinkCabVehicleHealthState Health;
    TestTrue(TEXT("wheel damage applied"),
        Health.ApplyFunctionalDamage(EPinkCabVehicleHealthChannel::Wheel, 0.25f));
    TestTrue(TEXT("brake damage applied"),
        Health.ApplyFunctionalDamage(EPinkCabVehicleHealthChannel::Brake, 0.5f));
    TestTrue(TEXT("clutch damage applied"),
        Health.ApplyFunctionalDamage(EPinkCabVehicleHealthChannel::Clutch, 0.1f));
    TestTrue(TEXT("gearbox damage applied"),
        Health.ApplyFunctionalDamage(EPinkCabVehicleHealthChannel::Gearbox, 0.2f));
    Health.SetClutchTemperature01(0.42f);
    Health.SetBrakeTemperature01(0.73f);

    FPinkCabVehicleLoadState Load;
    Load.SetFuelMassKg(40.0f, -10.0f);
    Load.SetCrew(58.0f, 49.0f);
    Load.AddPassenger(FPinkCabVehicleLoadItem(70.0f, -20.0f));
    const FPinkCabStableId FareId(TEXT("fare:vehicle:snapshot"));
    const TArray<FPinkCabVehicleLoadItem> FareItems = {
        FPinkCabVehicleLoadItem(65.0f, -35.0f),
        FPinkCabVehicleLoadItem(80.0f, 45.0f)};
    TestTrue(TEXT("fare group set"), Load.TrySetFarePassengerGroup(FareId, FareItems));

    const FPinkCabTatraProfile Profile = FPinkCabTatraProfile::Canonical();
    const float ExpectedMass = Load.GetTotalMassKg(Profile);
    const float ExpectedCg = Load.GetLongitudinalCgInputCm(Profile);
    FPinkCabVehicleSnapshot Snapshot;
    TestTrue(TEXT("capture succeeds"),
        FPinkCabVehicleSnapshotCodec::Capture(Health, Load, Snapshot));
    TestEqual(TEXT("health channel count"), Snapshot.Health.ChannelHealth.Num(),
        static_cast<int32>(EPinkCabVehicleHealthChannel::Count));
    TestEqual(TEXT("damage serial captured"), Snapshot.Health.FunctionalDamageSerial, uint32(4));
    TestEqual(TEXT("clutch temperature captured"), Snapshot.Health.ClutchTemperature01, 0.42f);
    TestEqual(TEXT("brake temperature captured"), Snapshot.Health.BrakeTemperature01, 0.73f);
    TestEqual(TEXT("fare group captured"), Snapshot.Load.FarePassengerGroupId, FareId.Serialize());
    TestEqual(TEXT("fare passenger count captured"), Snapshot.Load.FarePassengers.Num(), 2);

    FPinkCabVehicleHealthState RestoredHealth;
    FPinkCabVehicleLoadState RestoredLoad;
    TestTrue(TEXT("restore succeeds"),
        FPinkCabVehicleSnapshotCodec::Restore(Snapshot, RestoredHealth, RestoredLoad));
    TestEqual(TEXT("wheel health restored"),
        RestoredHealth.GetHealth(EPinkCabVehicleHealthChannel::Wheel), 0.75f);
    TestEqual(TEXT("brake health restored"),
        RestoredHealth.GetHealth(EPinkCabVehicleHealthChannel::Brake), 0.5f);
    TestEqual(TEXT("clutch health restored"),
        RestoredHealth.GetHealth(EPinkCabVehicleHealthChannel::Clutch), 0.9f);
    TestEqual(TEXT("gearbox health restored"),
        RestoredHealth.GetHealth(EPinkCabVehicleHealthChannel::Gearbox), 0.8f);
    TestEqual(TEXT("clutch temperature restored"), RestoredHealth.GetClutchTemperature01(), 0.42f);
    TestEqual(TEXT("brake temperature restored"), RestoredHealth.GetBrakeTemperature01(), 0.73f);
    TestEqual(TEXT("damage serial restored"), RestoredHealth.GetFunctionalDamageSerial(), uint32(4));
    TestTrue(TEXT("fare group restored"), RestoredLoad.HasFarePassengerGroup(FareId));
    TestTrue(TEXT("total mass restored"), FMath::IsNearlyEqual(
        RestoredLoad.GetTotalMassKg(Profile), ExpectedMass, 0.001f));
    TestTrue(TEXT("cg restored"), FMath::IsNearlyEqual(
        RestoredLoad.GetLongitudinalCgInputCm(Profile), ExpectedCg, 0.001f));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabVehicleSnapshotAtomicRejectTest,
    "PinkCab.Persistence.VehicleSnapshot.AtomicInvalidLoad",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPinkCabVehicleSnapshotAtomicRejectTest::RunTest(const FString& Parameters)
{
    FPinkCabVehicleHealthState SourceHealth;
    FPinkCabVehicleLoadState SourceLoad;
    SourceLoad.SetFuelMassKg(25.0f, 5.0f);
    SourceLoad.SetCrew(58.0f, 49.0f);
    const FPinkCabStableId FareId(TEXT("fare:vehicle:invalid"));
    const TArray<FPinkCabVehicleLoadItem> FareItems = {FPinkCabVehicleLoadItem(70.0f, -30.0f)};
    TestTrue(TEXT("source fare group"), SourceLoad.TrySetFarePassengerGroup(FareId, FareItems));

    FPinkCabVehicleSnapshot Snapshot;
    TestTrue(TEXT("capture valid source"),
        FPinkCabVehicleSnapshotCodec::Capture(SourceHealth, SourceLoad, Snapshot));
    Snapshot.Load.FarePassengers[0].MassKg = -1.0f;

    FPinkCabVehicleHealthState TargetHealth;
    TargetHealth.ApplyFunctionalDamage(EPinkCabVehicleHealthChannel::Wheel, 0.1f);
    FPinkCabVehicleLoadState TargetLoad;
    TargetLoad.SetFuelMassKg(10.0f, 0.0f);
    TargetLoad.SetCrew(50.0f, 40.0f);
    const FPinkCabTatraProfile Profile = FPinkCabTatraProfile::Canonical();
    const float BeforeMass = TargetLoad.GetTotalMassKg(Profile);
    const float BeforeWheel = TargetHealth.GetHealth(EPinkCabVehicleHealthChannel::Wheel);
    const uint32 BeforeSerial = TargetHealth.GetFunctionalDamageSerial();

    TestFalse(TEXT("invalid negative mass rejected"),
        FPinkCabVehicleSnapshotCodec::Restore(Snapshot, TargetHealth, TargetLoad));
    TestEqual(TEXT("target wheel remains atomic"),
        TargetHealth.GetHealth(EPinkCabVehicleHealthChannel::Wheel), BeforeWheel);
    TestEqual(TEXT("target serial remains atomic"),
        TargetHealth.GetFunctionalDamageSerial(), BeforeSerial);
    TestTrue(TEXT("target load remains atomic"), FMath::IsNearlyEqual(
        TargetLoad.GetTotalMassKg(Profile), BeforeMass, 0.001f));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabVehicleSnapshotLegacyV1MigrationTest,
    "PinkCab.Persistence.VehicleSnapshot.LegacyV1Migration",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabVehicleSnapshotLegacyV1MigrationTest::RunTest(const FString& Parameters)
{
    FPinkCabVehicleSnapshot Legacy;
    Legacy.SchemaVersion = FPinkCabVehicleSnapshot::LegacySchemaVersion;
    Legacy.Health.ChannelHealth.Init(1.0f, FPinkCabVehicleSnapshot::LegacySchema1HealthChannelCount);
    Legacy.Health.ChannelHealth[0] = 0.75f; // Wheel
    Legacy.Health.ChannelHealth[4] = 0.55f; // Brake
    Legacy.Health.ChannelHealth[5] = 0.60f; // Door in v1
    Legacy.Health.ChannelHealth[7] = 0.70f; // EngineOil in v1
    Legacy.Health.FunctionalDamageSerial = 9;

    FPinkCabVehicleHealthState Health;
    FPinkCabVehicleLoadState Load;
    TestTrue(TEXT("schema v1 restore remains supported"),
        FPinkCabVehicleSnapshotCodec::Restore(Legacy, Health, Load));
    TestEqual(TEXT("legacy wheel index preserved"),
        Health.GetHealth(EPinkCabVehicleHealthChannel::Wheel), 0.75f);
    TestEqual(TEXT("legacy brake index preserved"),
        Health.GetHealth(EPinkCabVehicleHealthChannel::Brake), 0.55f);
    TestEqual(TEXT("new clutch defaults healthy"),
        Health.GetHealth(EPinkCabVehicleHealthChannel::Clutch), 1.0f);
    TestEqual(TEXT("new gearbox defaults healthy"),
        Health.GetHealth(EPinkCabVehicleHealthChannel::Gearbox), 1.0f);
    TestEqual(TEXT("legacy door remaps after inserted drivetrain channels"),
        Health.GetHealth(EPinkCabVehicleHealthChannel::Door), 0.60f);
    TestEqual(TEXT("legacy engine oil remaps after inserted drivetrain channels"),
        Health.GetHealth(EPinkCabVehicleHealthChannel::EngineOil), 0.70f);
    TestEqual(TEXT("legacy clutch temp starts cold"), Health.GetClutchTemperature01(), 0.0f);
    TestEqual(TEXT("legacy brake temp starts cold"), Health.GetBrakeTemperature01(), 0.0f);
    TestEqual(TEXT("legacy functional serial preserved"), Health.GetFunctionalDamageSerial(), uint32(9));
    return true;
}

#endif
