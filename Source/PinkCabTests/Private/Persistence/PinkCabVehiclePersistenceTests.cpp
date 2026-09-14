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
    TestEqual(TEXT("damage serial captured"), Snapshot.Health.FunctionalDamageSerial, uint32(2));
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
    TestEqual(TEXT("damage serial restored"), RestoredHealth.GetFunctionalDamageSerial(), uint32(2));
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
#endif
