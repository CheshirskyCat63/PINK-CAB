#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Persistence/PinkCabSaveHeader.h"
#include "Persistence/PinkCabPersistenceService.h"
#include "Persistence/PinkCabMigrationRegistry.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabPersistenceHeaderTest,
    "PinkCab.Persistence.HeaderIdentity",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabPersistenceHeaderTest::RunTest(const FString& Parameters)
{
    const FPinkCabSaveHeader Header = FPinkCabSaveHeader::Create(
        TEXT("PINK CAB"), TEXT("1.0.0"), TEXT("cfg-1"), TEXT("gen-1"), TEXT("content-1"));
    TestEqual(TEXT("product identity"), Header.ProductName, FString(TEXT("PINK CAB")));
    TestEqual(TEXT("schema identity"), Header.SchemaVersion, FString(TEXT("1.0.0")));
    TestEqual(TEXT("config identity"), Header.ConfigVersion, FString(TEXT("cfg-1")));
    TestEqual(TEXT("generator identity"), Header.GeneratorVersion, FString(TEXT("gen-1")));
    TestEqual(TEXT("content identity"), Header.ContentSetVersion, FString(TEXT("content-1")));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabPersistencePolicyTest,
    "PinkCab.Persistence.Policy",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabPersistencePolicyTest::RunTest(const FString& Parameters)
{
    TestEqual(TEXT("three campaign slots"), FPinkCabPersistencePolicy::CampaignSlotCount, 3);
    TestEqual(TEXT("three rolling checkpoints"), FPinkCabPersistencePolicy::RollingCheckpointCount, 3);
    TestEqual(TEXT("periodic checkpoint every five minutes"), FPinkCabPersistencePolicy::PeriodicCheckpointSeconds, 300);
    TestTrue(TEXT("manual quit eligible at full stop with no fare/passenger"), FPinkCabPersistencePolicy::CanManualSaveExit(false, false, true));
    TestFalse(TEXT("manual quit blocked by active fare"), FPinkCabPersistencePolicy::CanManualSaveExit(true, false, true));
    TestFalse(TEXT("manual quit blocked while moving"), FPinkCabPersistencePolicy::CanManualSaveExit(false, false, false));
    TestTrue(TEXT("sleep eligible in car/full stop/no fare"), FPinkCabPersistencePolicy::CanSleepEndDay(true, true, false));
    TestFalse(TEXT("sleep blocked outside car"), FPinkCabPersistencePolicy::CanSleepEndDay(false, true, false));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabPersistenceRoundTripTest,
    "PinkCab.Persistence.LogicalRoundTrip",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabPersistenceRoundTripTest::RunTest(const FString& Parameters)
{
    const FPinkCabSaveHeader Header = FPinkCabSaveHeader::Create(TEXT("PINK CAB"), TEXT("1.0.0"), TEXT("cfg-1"), TEXT("gen-1"), TEXT("content-1"));
    FPinkCabLogicalSaveState State;
    State.VehicleHealth.ChannelHealth = {1.0f, 0.75f, 0.5f};
    State.VehicleHealth.FunctionalDamageSerial = 7;
    State.Passenger.IdentityId = TEXT("passenger-42");
    State.Passenger.TemplateId = TEXT("shopper-risky");
    State.Passenger.IdentitySeed = 123456u;
    State.Passenger.AppearanceSeed = 654321u;
    State.Passenger.AppearanceProfileId = TEXT("profile.shopper-risky.01");
    State.Passenger.PaidFareCount = 2;
    State.Passenger.AuthoredEventCount = 1;
    State.Passenger.bRepeatEligible = true;
    State.Passenger.bNeuralPermissionGranted = true;
    State.Passenger.bNeuralBlocked = false;
    State.Passenger.Trust = 0.25f;
    State.Passenger.Satisfaction = -0.5f;
    State.Passenger.RiskTolerance = 0.8f;
    State.Passenger.ReviewStars = {1, 4};
    State.Passenger.ReviewTexts = {TEXT("late"), TEXT("returned")};
    State.Economy.BalanceMinor = 12345;
    State.Economy.DebtLimitMinor = 5000;
    State.Economy.CommittedTransactionIds = {TEXT("fare-1"), TEXT("tip-1")};

    TArray<uint8> Bytes;
    TestTrue(TEXT("serialize logical state"), FPinkCabPersistenceService::Serialize(Header, State, Bytes));
    TestTrue(TEXT("serialized payload non-empty"), Bytes.Num() > 0);

    FPinkCabLogicalSaveState Restored;
    FPinkCabMigrationRegistry Registry;
    TestEqual(TEXT("compatible load succeeds"), FPinkCabPersistenceService::Deserialize(Bytes, Header, Registry, Restored), EPinkCabLoadResult::Success);
    TestEqual(TEXT("vehicle health round-trip"), Restored.VehicleHealth.ChannelHealth[1], 0.75f);
    TestEqual(TEXT("damage serial round-trip"), Restored.VehicleHealth.FunctionalDamageSerial, uint32(7));
    TestEqual(TEXT("passenger id round-trip"), Restored.Passenger.IdentityId, FString(TEXT("passenger-42")));
    TestEqual(TEXT("appearance seed round-trip"), Restored.Passenger.AppearanceSeed, uint64(654321));
    TestEqual(TEXT("appearance profile round-trip"), Restored.Passenger.AppearanceProfileId,
        FString(TEXT("profile.shopper-risky.01")));
    TestEqual(TEXT("paid fare count round-trip"), Restored.Passenger.PaidFareCount, 2);
    TestEqual(TEXT("authored count round-trip"), Restored.Passenger.AuthoredEventCount, 1);
    TestTrue(TEXT("repeat eligibility round-trip"), Restored.Passenger.bRepeatEligible);
    TestTrue(TEXT("neural permission round-trip"), Restored.Passenger.bNeuralPermissionGranted);
    TestFalse(TEXT("neural block round-trip"), Restored.Passenger.bNeuralBlocked);
    TestEqual(TEXT("review round-trip"), Restored.Passenger.ReviewTexts[1], FString(TEXT("returned")));
    TestEqual(TEXT("economy balance round-trip"), Restored.Economy.BalanceMinor, int64(12345));
    TestEqual(TEXT("transaction ids round-trip"), Restored.Economy.CommittedTransactionIds.Num(), 2);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabPersistenceIncompatibleSchemaTest,
    "PinkCab.Persistence.IncompatibleSchema",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabPersistenceIncompatibleSchemaTest::RunTest(const FString& Parameters)
{
    const FPinkCabSaveHeader Saved = FPinkCabSaveHeader::Create(TEXT("PINK CAB"), TEXT("1.0.0"), TEXT("cfg-1"), TEXT("gen-1"), TEXT("content-1"));
    const FPinkCabSaveHeader Expected = FPinkCabSaveHeader::Create(TEXT("PINK CAB"), TEXT("2.0.0"), TEXT("cfg-1"), TEXT("gen-1"), TEXT("content-1"));
    TArray<uint8> Bytes;
    TestTrue(TEXT("serialize old schema"), FPinkCabPersistenceService::Serialize(Saved, FPinkCabLogicalSaveState(), Bytes));
    FPinkCabLogicalSaveState Restored;
    FPinkCabMigrationRegistry Registry;
    TestEqual(TEXT("unregistered schema mismatch fails explicitly"), FPinkCabPersistenceService::Deserialize(Bytes, Expected, Registry, Restored), EPinkCabLoadResult::IncompatibleSchema);
    Registry.RegisterBoundary(TEXT("1.0.0"), TEXT("2.0.0"));
    TestEqual(TEXT("registered boundary reports migration required"), FPinkCabPersistenceService::Deserialize(Bytes, Expected, Registry, Restored), EPinkCabLoadResult::MigrationRequired);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabPersistenceLastCommittedTest,
    "PinkCab.Persistence.LastCommittedRecovery",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabPersistenceLastCommittedTest::RunTest(const FString& Parameters)
{
    const FPinkCabSaveHeader Header = FPinkCabSaveHeader::Create(TEXT("PINK CAB"), TEXT("1.0.0"), TEXT("cfg-1"), TEXT("gen-1"), TEXT("content-1"));
    FPinkCabPersistenceService Service;
    FPinkCabLogicalSaveState First;
    First.Economy.BalanceMinor = 1000;
    TestTrue(TEXT("first committed snapshot accepted"), Service.CommitSnapshot(Header, First, EPinkCabPersistenceCommitReason::FareSettlement));

    FPinkCabLogicalSaveState Uncommitted = First;
    Uncommitted.Economy.BalanceMinor = 999999;
    FPinkCabLogicalSaveState Recovered;
    FPinkCabMigrationRegistry Registry;
    TestEqual(TEXT("recovery uses last committed snapshot"), Service.RecoverLastCommitted(Header, Registry, Recovered), EPinkCabLoadResult::Success);
    TestEqual(TEXT("uncommitted mutation never rolls into recovery"), Recovered.Economy.BalanceMinor, int64(1000));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabPersistenceCheckpointRingTest,
    "PinkCab.Persistence.CheckpointRing",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabPersistenceCheckpointRingTest::RunTest(const FString& Parameters)
{
    FPinkCabCheckpointRing Ring;
    Ring.Push({1});
    Ring.Push({2});
    Ring.Push({3});
    Ring.Push({4});
    TestEqual(TEXT("ring remains bounded"), Ring.Num(), FPinkCabPersistencePolicy::RollingCheckpointCount);
    TestEqual(TEXT("oldest checkpoint evicted"), Ring.GetOldest()[0], uint8(2));
    TestEqual(TEXT("newest checkpoint retained"), Ring.GetNewest()[0], uint8(4));
    return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabPersistenceWorkdayPolicyTest,
    "PinkCab.Persistence.WorkdayPolicy",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabPersistenceWorkdayPolicyTest::RunTest(const FString& Parameters)
{
    TestEqual(TEXT("workday game hours"), FPinkCabPersistencePolicy::WorkdayGameHours, 12);
    TestEqual(TEXT("workday real minutes"), FPinkCabPersistencePolicy::WorkdayRealMinutes, 120);
    TestEqual(TEXT("workday time scale"), FPinkCabPersistencePolicy::WorkdayTimeScale, 6);
    TestTrue(TEXT("system menu hard pauses"), FPinkCabPersistencePolicy::ShouldHardPause(true));
    TestFalse(TEXT("in-world dashboard does not pause"), FPinkCabPersistencePolicy::ShouldHardPause(false));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabPersistenceTerminalAndDayGateTest,
    "PinkCab.Persistence.TerminalAndDayGate",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabPersistenceTerminalAndDayGateTest::RunTest(const FString& Parameters)
{
    const FPinkCabTerminalRecoveryPolicy Policy = FPinkCabTerminalRecoveryPolicy::Canonical();
    TestTrue(TEXT("terminal recovery fails active fare"), Policy.bFailActiveFare);
    TestTrue(TEXT("terminal recovery ends workday"), Policy.bEndWorkday);
    TestTrue(TEXT("damaged vehicle is preserved"), Policy.bPreserveDamagedVehicle);
    TestFalse(TEXT("no free reset"), Policy.bFreeReset);
    TestTrue(TEXT("exactly one household transaction unlocks next cycle"), FPinkCabPersistencePolicy::CanAdvanceToNextWorkday(true, 1));
    TestFalse(TEXT("missing household transaction blocks next cycle"), FPinkCabPersistencePolicy::CanAdvanceToNextWorkday(true, 0));
    TestFalse(TEXT("duplicate household transaction is invalid"), FPinkCabPersistencePolicy::CanAdvanceToNextWorkday(true, 2));
    return true;
}

#endif
