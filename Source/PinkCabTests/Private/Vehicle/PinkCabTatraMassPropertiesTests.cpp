#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Vehicle/PinkCabVehicleLoadState.h"
#include "Vehicle/PinkCabVehicleMassProperties.h"
#include "Vehicle/PinkCabVehicleStateSnapshot.h"
#include "Persistence/PinkCabVehicleSnapshot.h"
#include "Persistence/PinkCabVehicleSnapshotArchive.h"
#include "Serialization/MemoryReader.h"
#include "Serialization/MemoryWriter.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPinkCabTatraMassInertiaTest,
    "PinkCab.Vehicle.PhysicalFoundation.MassInertiaCombination",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPinkCabTatraMassInertiaTest::RunTest(const FString& Parameters)
{
    const FPinkCabTatraProfile Profile = FPinkCabTatraProfile::Canonical();
    FPinkCabVehicleLoadState Near, Wide;
    Near.AddPassenger(FPinkCabVehicleLoadItem(50.0f, FVector(-30,-20,60)));
    Near.AddPassenger(FPinkCabVehicleLoadItem(50.0f, FVector(-30,20,60)));
    Wide.AddPassenger(FPinkCabVehicleLoadItem(50.0f, FVector(-30,-70,60)));
    Wide.AddPassenger(FPinkCabVehicleLoadItem(50.0f, FVector(-30,70,60)));
    FPinkCabVehicleMassProperties A, B;
    if (!TestTrue(TEXT("near mass properties resolve"), Near.TryGetMassProperties(Profile, A))
        || !TestTrue(TEXT("wide mass properties resolve"), Wide.TryGetMassProperties(Profile, B))) return false;
    TestTrue(TEXT("equal balanced loads have equal mass and centre"),
        FMath::IsNearlyEqual(A.MassKg, B.MassKg) && A.CenterCm.Equals(B.CenterCm, 0.001));
    const double TraceA = A.PrincipalInertiaKgCm2.X + A.PrincipalInertiaKgCm2.Y + A.PrincipalInertiaKgCm2.Z;
    const double TraceB = B.PrincipalInertiaKgCm2.X + B.PrincipalInertiaKgCm2.Y + B.PrincipalInertiaKgCm2.Z;
    TestTrue(TEXT("native parallel-axis combination includes lateral load inertia"),
        FMath::IsNearlyEqual(TraceB - TraceA, 900000.0, 2.0));
    TestTrue(TEXT("principal inertias remain positive and orientations normalized"), A.IsValid() && B.IsValid());
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPinkCabTatraMassCoordinatePersistenceTest,
    "PinkCab.Vehicle.PhysicalFoundation.MassCoordinatePersistence",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPinkCabTatraMassCoordinatePersistenceTest::RunTest(const FString& Parameters)
{
    FPinkCabVehicleHealthState Health;
    Health.SetClutchTemperature01(0.6f);
    Health.SetBrakeTemperature01(0.4f);
    FPinkCabVehicleLoadState Load;
    Load.SetFuelMassKg(42.0f, FVector(-20,12,36));
    Load.SetCrew(58.0f, 49.0f);
    Load.AddPassenger(FPinkCabVehicleLoadItem(90.0f, FVector(-120,-52,77)));
    FPinkCabVehicleSnapshot Snapshot;
    if (!TestTrue(TEXT("3D load snapshot captures"), FPinkCabVehicleSnapshotCodec::Capture(Health, Load, Snapshot))) return false;
    TArray<uint8> Bytes;
    FMemoryWriter Writer(Bytes);
    FPinkCabVehicleSnapshotArchive::Serialize(Writer, Snapshot);
    TestFalse(TEXT("3D archive serializes without error"), Writer.IsError());
    FPinkCabVehicleSnapshot Read;
    FMemoryReader Reader(Bytes);
    FPinkCabVehicleSnapshotArchive::Serialize(Reader, Read);
    TestFalse(TEXT("3D archive reads without error"), Reader.IsError());
    TestEqual(TEXT("all bytes consumed"), Reader.Tell(), int64(Bytes.Num()));
    FPinkCabVehicleHealthState RestoredHealth;
    FPinkCabVehicleLoadState Restored;
    if (!TestTrue(TEXT("3D load snapshot restores"), FPinkCabVehicleSnapshotCodec::Restore(Read, RestoredHealth, Restored))) return false;
    FPinkCabVehicleMassProperties Before, After;
    const FPinkCabTatraProfile Profile = FPinkCabTatraProfile::Canonical();
    Load.TryGetMassProperties(Profile, Before);
    Restored.TryGetMassProperties(Profile, After);
    TestTrue(TEXT("restore preserves complete centre and inertia, not only kilograms"),
        Before.CenterCm.Equals(After.CenterCm, 0.001)
        && Before.PrincipalInertiaKgCm2.Equals(After.PrincipalInertiaKgCm2, 0.01));
    TestEqual(TEXT("no thermal repair during coordinate migration"), RestoredHealth.GetClutchTemperature01(), 0.6f);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPinkCabTatraLegacyCoordinateMigrationTest,
    "PinkCab.Vehicle.PhysicalFoundation.LegacyCoordinateMigration",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPinkCabTatraLegacyCoordinateMigrationTest::RunTest(const FString& Parameters)
{
    FPinkCabVehicleHealthState Health;
    Health.SetClutchTemperature01(0.6f);
    Health.SetBrakeTemperature01(0.4f);
    FPinkCabVehicleLoadState Load;
    Load.SetFuelMassKg(100.0f);
    Load.SetCrew(58.0f, 49.0f);
    Load.AddPassenger(FPinkCabVehicleLoadItem(90.0f, -120.0f));
    FPinkCabVehicleSnapshot Snapshot;
    if (!FPinkCabVehicleSnapshotCodec::Capture(Health, Load, Snapshot)) return false;
    Snapshot.SchemaVersion = FPinkCabVehicleSnapshot::LongitudinalOnlySchemaVersion;
    TArray<uint8> Bytes;
    FMemoryWriter Writer(Bytes);
    FPinkCabVehicleSnapshotArchive::Serialize(Writer, Snapshot);
    FPinkCabVehicleSnapshot Read;
    FMemoryReader Reader(Bytes);
    FPinkCabVehicleSnapshotArchive::Serialize(Reader, Read);
    TestFalse(TEXT("v3 payload read has no overread"), Reader.IsError());
    TestEqual(TEXT("old byte layout consumed exactly"), Reader.Tell(), int64(Bytes.Num()));
    FPinkCabVehicleHealthState RestoredHealth;
    FPinkCabVehicleLoadState Restored;
    if (!TestTrue(TEXT("old v3 coordinates migrate"), FPinkCabVehicleSnapshotCodec::Restore(Read, RestoredHealth, Restored))) return false;
    TestEqual(TEXT("v3 thermals remain serialized after introducing v4"), RestoredHealth.GetBrakeTemperature01(), 0.4f);
    const FPinkCabTatraProfile Profile = FPinkCabTatraProfile::Canonical();
    TestEqual(TEXT("v3 restore does not duplicate passengers"), Restored.GetTotalMassKg(Profile), 1747.0f);
    TestTrue(TEXT("legacy longitudinal moment retained"), FMath::IsNearlyEqual(
        Load.GetLongitudinalCgInputCm(Profile), Restored.GetLongitudinalCgInputCm(Profile), 0.001f));
    return true;
}
#endif
