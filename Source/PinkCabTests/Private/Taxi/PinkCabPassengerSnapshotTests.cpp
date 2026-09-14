#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Taxi/PinkCabPassengerSnapshot.h"
#include "Taxi/PinkCabConductorServiceHooks.h"

namespace PinkCabPassengerSnapshotTests
{
FPinkCabPassengerRecord* AddPassenger(
    FPinkCabPassengerRegistry& Registry,
    const TCHAR* Id,
    const TCHAR* Context)
{
    FPinkCabPassengerRecord* Record = nullptr;
    Registry.TryCreate(
        FPinkCabStableId(Id),
        FPinkCabPassengerTemplate(FName(TEXT("Commuter")), 72.0f),
        Context,
        {FName(TEXT("quiet")), FName(TEXT("airport"))},
        Record);
    return Record;
}

FPinkCabPassengerRelationship Delta(float Trust, float Satisfaction, float Risk)
{
    FPinkCabPassengerRelationship Result;
    Result.Trust = Trust;
    Result.Satisfaction = Satisfaction;
    Result.RiskTolerance = Risk;
    return Result;
}
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabPassengerSnapshotRoundTripTest,
    "PinkCab.Taxi.PassengerSnapshot.RoundTripFullState",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabPassengerSnapshotRoundTripTest::RunTest(const FString& Parameters)
{
    using namespace PinkCabPassengerSnapshotTests;
    FPinkCabPassengerRegistry Source(4, 4, 3, 16, 3, 16);
    FPinkCabPassengerRecord* Record = AddPassenger(
        Source, TEXT("passenger:snapshot:1"), TEXT("city:first-euro:snapshot"));
    TestNotNull(TEXT("record created"), Record);
    Record->RegisterPaidFareOnce(
        FPinkCabStableId(TEXT("social:snapshot:1")), Delta(0.2f, 0.3f, -0.1f));
    Record->AddRideMemoryOnce(
        FPinkCabStableId(TEXT("memory:snapshot:1")),
        FPinkCabStableId(TEXT("fare:snapshot:1")), FName(TEXT("paid")));
    Record->SetNeuralPermission(true);
    Record->AddMessageOnce(
        FPinkCabStableId(TEXT("message:snapshot:1")), TEXT("same pickup tomorrow"));
    const uint64 ExpectedSignature = Source.GetReconstructionSignature();

    FPinkCabPassengerSnapshot Snapshot;
    TestTrue(TEXT("capture succeeds"),
        FPinkCabPassengerSnapshotCodec::Capture(Source, Snapshot));
    FPinkCabPassengerRegistry Restored(1, 1, 1, 1, 1, 1);
    TestTrue(TEXT("restore succeeds"),
        FPinkCabPassengerSnapshotCodec::Restore(Snapshot, Restored));
    TestEqual(TEXT("record count restored"), Restored.Num(), 1);
    TestEqual(TEXT("full reconstruction signature restored"),
        Restored.GetReconstructionSignature(), ExpectedSignature);

    const FPinkCabPassengerRecord* RestoredRecord = Restored.Find(
        FPinkCabStableId(TEXT("passenger:snapshot:1")));
    TestNotNull(TEXT("restored record exists"), RestoredRecord);
    TestEqual(TEXT("appearance profile survives"),
        RestoredRecord->AppearanceProfileId, Record->AppearanceProfileId);
    TestEqual(TEXT("ride memory survives"), RestoredRecord->GetRideMemories().Num(), 1);
    TestEqual(TEXT("neural message survives"), RestoredRecord->GetNeuralMessages().Num(), 1);
    TestTrue(TEXT("neural permission survives"), RestoredRecord->HasNeuralPermission());
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabPassengerSnapshotAtomicRejectTest,
    "PinkCab.Taxi.PassengerSnapshot.AtomicInvalidSchema",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabPassengerSnapshotAtomicRejectTest::RunTest(const FString& Parameters)
{
    using namespace PinkCabPassengerSnapshotTests;    FPinkCabPassengerRegistry Source(4, 4, 3, 16, 3, 16);
    AddPassenger(Source, TEXT("passenger:snapshot:source"), TEXT("snapshot:source"));
    FPinkCabPassengerSnapshot Snapshot;
    TestTrue(TEXT("capture valid source"), FPinkCabPassengerSnapshotCodec::Capture(Source, Snapshot));
    Snapshot.SchemaVersion = FPinkCabPassengerSnapshot::CurrentSchemaVersion + 1;

    FPinkCabPassengerRegistry Target(4, 4, 3, 16, 3, 16);
    AddPassenger(Target, TEXT("passenger:sentinel"), TEXT("snapshot:sentinel"));
    const uint64 Before = Target.GetReconstructionSignature();
    TestFalse(TEXT("invalid schema rejected"),
        FPinkCabPassengerSnapshotCodec::Restore(Snapshot, Target));
    TestEqual(TEXT("target remains atomic"), Target.GetReconstructionSignature(), Before);
    TestNotNull(TEXT("sentinel survives rejection"),
        Target.Find(FPinkCabStableId(TEXT("passenger:sentinel"))));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabPassengerSnapshotBoundsTest,
    "PinkCab.Taxi.PassengerSnapshot.BoundedRestore",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabPassengerSnapshotBoundsTest::RunTest(const FString& Parameters)
{
    using namespace PinkCabPassengerSnapshotTests;
    FPinkCabPassengerRegistry Source(4, 4, 3, 16, 3, 16);    AddPassenger(Source, TEXT("passenger:bounds:1"), TEXT("snapshot:bounds:1"));
    AddPassenger(Source, TEXT("passenger:bounds:2"), TEXT("snapshot:bounds:2"));
    FPinkCabPassengerSnapshot Snapshot;
    TestTrue(TEXT("capture valid two records"),
        FPinkCabPassengerSnapshotCodec::Capture(Source, Snapshot));
    Snapshot.MaxRecords = 1;

    FPinkCabPassengerRegistry Target(2, 4, 3, 16, 3, 16);
    TestFalse(TEXT("record overflow rejected"),
        FPinkCabPassengerSnapshotCodec::Restore(Snapshot, Target));
    TestEqual(TEXT("empty target remains empty"), Target.Num(), 0);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabPassengerSnapshotConductorPersistenceTest,
    "PinkCab.Taxi.PassengerSnapshot.ConductorPersistenceBoundary",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabPassengerSnapshotConductorPersistenceTest::RunTest(const FString& Parameters)
{
    using namespace PinkCabPassengerSnapshotTests;
    FPinkCabPassengerRegistry Registry(4, 4, 3, 16, 3, 16);
    FPinkCabPassengerRecord* Record = AddPassenger(
        Registry, TEXT("passenger:boundary:1"), TEXT("snapshot:boundary"));
    Record->SetNeuralPermission(true);
    Record->RegisterAuthoredEventOnce(
        FPinkCabStableId(TEXT("authored:boundary:1")), Delta(0.4f, 0.1f, 0.0f));
    FPinkCabConductorServiceHooks Hooks(Registry);
    TestEqual(TEXT("conductor sees bounded passenger count"), Hooks.GetPassengerCount(), 1);
    TestFalse(TEXT("conductor never drives"), Hooks.HasDrivingAuthority());
    TestFalse(TEXT("conductor never steers"), Hooks.HasSteeringAuthority());
    TestTrue(TEXT("conductor may read contact availability"),
        Hooks.CanContact(FPinkCabStableId(TEXT("passenger:boundary:1"))));

    FPinkCabPersistedPassengerIdentity Persisted;
    TestTrue(TEXT("canonical record maps to persistence payload"),
        FPinkCabPassengerSnapshotCodec::ToPersistedIdentity(*Record, Persisted));
    TestEqual(TEXT("persisted stable id"), Persisted.IdentityId, Record->IdentityId.Serialize());
    TestEqual(TEXT("persisted appearance seed"), Persisted.AppearanceSeed, Record->AppearanceSeed);
    TestEqual(TEXT("persisted authored count"), Persisted.AuthoredEventCount, 1);
    TestTrue(TEXT("persisted repeat flag"), Persisted.bRepeatEligible);
    TestTrue(TEXT("persisted neural permission"), Persisted.bNeuralPermissionGranted);
    return true;
}
#endif