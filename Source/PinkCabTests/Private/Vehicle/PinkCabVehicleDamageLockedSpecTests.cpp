#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Persistence/PinkCabVehicleSnapshot.h"
#include "Vehicle/PinkCabVehicleControlState.h"
#include "Vehicle/PinkCabVehicleHealthService.h"
#include <type_traits>
#include <utility>

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabLockedDamageChannelsTest,
    "PinkCab.Vehicle.Health.LockedDamageChannels",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabLockedDamageChannelsTest::RunTest(const FString& Parameters)
{
    TestEqual(TEXT("CD-600 functional+cosmetic channel count"),
        static_cast<int32>(EPinkCabVehicleHealthChannel::Count), 16);

    FPinkCabVehicleHealthService Service;
    const FName RequiredZones[] = {
        TEXT("Glass"), TEXT("BrakeHeat"), TEXT("BrakeHydraulic"), TEXT("EngineHead")};
    TSet<uint8> ResolvedChannels;
    for (const FName Zone : RequiredZones)
    {
        FPinkCabVehicleHitEvent Event;
        TestTrue(TEXT("locked authored zone resolves"), Service.ResolveSyntheticHitZone(Zone, 0.5f, Event));
        ResolvedChannels.Add(static_cast<uint8>(Event.Channel));
    }
    TestEqual(TEXT("locked authored zones own distinct health channels"), ResolvedChannels.Num(), 4);
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabDamageSymptomCeilingTest,
    "PinkCab.Vehicle.Health.DamageSymptomCeilings",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabDamageSymptomCeilingTest::RunTest(const FString& Parameters)
{
    FPinkCabVehicleHealthService Service;
    FPinkCabVehicleHealthState Health;
    FPinkCabVehicleHitEvent BrakeHeat;
    FPinkCabVehicleHitEvent EngineHead;
    TestTrue(TEXT("brake heat zone resolves"), Service.ResolveSyntheticHitZone(TEXT("BrakeHeat"), 0.5f, BrakeHeat));
    TestTrue(TEXT("engine head zone resolves"), Service.ResolveSyntheticHitZone(TEXT("EngineHead"), 0.5f, EngineHead));
    TestTrue(TEXT("brake heat damage applies"), Service.ApplyHit(Health, BrakeHeat));
    TestTrue(TEXT("engine head damage applies"), Service.ApplyHit(Health, EngineHead));

    FPinkCabVehicleControlState Controls;
    Controls.SetBrake(1.0f);
    Controls.SetThrottle(1.0f);
    Service.ApplyCapabilitiesToControls(Health, Controls);
    TestEqual(TEXT("brake health creates measurable bounded brake ceiling"), Controls.Brake, 0.5f);
    TestEqual(TEXT("engine thermal health creates measurable bounded power ceiling"), Controls.Throttle, 0.5f);
    Service.ApplyCapabilitiesToControls(Health, Controls);
    TestEqual(TEXT("brake ceiling application is idempotent"), Controls.Brake, 0.5f);
    TestEqual(TEXT("power ceiling application is idempotent"), Controls.Throttle, 0.5f);
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabVehicleSnapshotV2Test,
    "PinkCab.Persistence.VehicleSnapshot.DamageSchemaV2",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabVehicleSnapshotV2Test::RunTest(const FString& Parameters)
{
    TestEqual(TEXT("expanded damage schema bumps vehicle snapshot version"),
        FPinkCabVehicleSnapshot::CurrentSchemaVersion, 2);

    FPinkCabVehicleHealthState Health;
    FPinkCabVehicleLoadState Load;
    FPinkCabVehicleSnapshot Snapshot;
    TestTrue(TEXT("expanded health captures"),
        FPinkCabVehicleSnapshotCodec::Capture(Health, Load, Snapshot));
    TestEqual(TEXT("expanded channel array persists completely"),
        Snapshot.Health.ChannelHealth.Num(), 16);

    Snapshot.SchemaVersion = 1;
    FPinkCabVehicleHealthState RestoredHealth;
    FPinkCabVehicleLoadState RestoredLoad;
    TestFalse(TEXT("old v1 layout is rejected rather than misread"),
        FPinkCabVehicleSnapshotCodec::Restore(Snapshot, RestoredHealth, RestoredLoad));
    return true;
}

template <typename T, typename = void>
struct THasPinkCabAuthoredHitResolver : std::false_type {};

template <typename T>
struct THasPinkCabAuthoredHitResolver<T, std::void_t<decltype(
    std::declval<const T&>().ResolveAuthoredHitZone(
        FName(), 0.5f, 0.25f, std::declval<FPinkCabVehicleHitEvent&>()))>> : std::true_type {};

template <typename T>
void VerifyPinkCabAuthoredHitResolver(FAutomationTestBase& Test, const T& Service)
{
    Test.TestTrue(TEXT("E04 exposes authored hit-zone severity/threshold resolver"),
        THasPinkCabAuthoredHitResolver<T>::value);
    if constexpr (THasPinkCabAuthoredHitResolver<T>::value)
    {
        FPinkCabVehicleHitEvent Event;
        Test.TestFalse(TEXT("below authored threshold has no functional consequence"),
            Service.ResolveAuthoredHitZone(TEXT("Brake"), 0.30f, 0.40f, Event));
        Test.TestFalse(TEXT("threshold boundary is non-triggering"),
            Service.ResolveAuthoredHitZone(TEXT("Brake"), 0.40f, 0.40f, Event));
        Test.TestTrue(TEXT("severity above authored threshold resolves"),
            Service.ResolveAuthoredHitZone(TEXT("Brake"), 0.70f, 0.40f, Event));
        Test.TestEqual(TEXT("resolved severity stays authored collision severity"), Event.Severity, 0.70f);
    }
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabAuthoredDamageThresholdTest,
    "PinkCab.Vehicle.Health.AuthoredThresholdMapping",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabAuthoredDamageThresholdTest::RunTest(const FString& Parameters)
{
    const FPinkCabVehicleHealthService Service;
    VerifyPinkCabAuthoredHitResolver(*this, Service);
    return true;
}
#endif
