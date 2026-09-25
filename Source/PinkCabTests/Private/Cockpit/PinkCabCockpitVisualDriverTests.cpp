#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Cockpit/PinkCabCockpitVisualDriverComponent.h"
#include "Cockpit/PinkCabCockpitPresentationState.h"
#include "Runtime/PinkCabChaosTatraPawn.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabCockpitVisualMappingTest,
    "PinkCab.Cockpit.VisualDriver.Mapping",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabCockpitVisualMappingTest::RunTest(const FString& Parameters)
{
    TestEqual(TEXT("semantic full left maps through Tatra wheel adapter to +450 degrees"),
        UPinkCabCockpitVisualDriverComponent::SteeringAngleDegrees(-1.0f), 450.0f);
    TestEqual(TEXT("semantic full right maps through Tatra wheel adapter to -450 degrees"),
        UPinkCabCockpitVisualDriverComponent::SteeringAngleDegrees(1.0f), -450.0f);
    TestEqual(TEXT("full pedal travel is bounded"),
        UPinkCabCockpitVisualDriverComponent::PedalTravelDegrees(1.0f), 18.0f);
    TestEqual(TEXT("half handbrake gets proportional lever angle"),
        UPinkCabCockpitVisualDriverComponent::HandbrakeAngleDegrees(0.5f), -10.0f);
    TestEqual(TEXT("full handbrake gets bounded lever angle"),
        UPinkCabCockpitVisualDriverComponent::HandbrakeAngleDegrees(1.0f), -20.0f);
    TestEqual(TEXT("cold temperature needle starts at low stop"),
        UPinkCabCockpitVisualDriverComponent::TemperatureNeedleAngleDegrees(0.0f), -60.0f);
    TestEqual(TEXT("full fuel needle reaches high stop"),
        UPinkCabCockpitVisualDriverComponent::FuelNeedleAngleDegrees(1.0f), 60.0f);
    TestEqual(TEXT("speedometer midpoint is 110 kmh"),
        UPinkCabCockpitVisualDriverComponent::SpeedometerNeedleAngleDegrees(110.0f), 0.0f);
    TestEqual(TEXT("tachometer midpoint is 3500 rpm"),
        UPinkCabCockpitVisualDriverComponent::TachometerNeedleAngleDegrees(3500.0f), 0.0f);
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabCockpitVisualGearPoseTest,
    "PinkCab.Cockpit.VisualDriver.GearPose",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabCockpitVisualGearPoseTest::RunTest(const FString& Parameters)
{
    const FVector Neutral = UPinkCabCockpitVisualDriverComponent::GearLeverOffset(0);
    const FVector First = UPinkCabCockpitVisualDriverComponent::GearLeverOffset(1);
    const FVector Second = UPinkCabCockpitVisualDriverComponent::GearLeverOffset(2);
    const FVector Third = UPinkCabCockpitVisualDriverComponent::GearLeverOffset(3);
    const FVector Fourth = UPinkCabCockpitVisualDriverComponent::GearLeverOffset(4);
    const FVector Fifth = UPinkCabCockpitVisualDriverComponent::GearLeverOffset(5);
    const FVector Reverse = UPinkCabCockpitVisualDriverComponent::GearLeverOffset(-1);
    TestTrue(TEXT("neutral rests under relocated 3/4 rail"),
        FMath::IsNearlyEqual(Neutral.X, 5.25f, KINDA_SMALL_NUMBER) && FMath::IsNearlyZero(Neutral.Y));
    TestTrue(TEXT("first is physical forward-left"), First.X < 0.0f && First.Y < 0.0f);
    TestTrue(TEXT("second is physical rear-left"), Second.X < 0.0f && Second.Y > 0.0f);
    TestTrue(TEXT("third occupies the previous 5th visual X"),
        FMath::IsNearlyEqual(Third.X, 5.25f, KINDA_SMALL_NUMBER) && Third.Y < 0.0f);
    TestTrue(TEXT("fourth occupies the previous reverse visual X"),
        FMath::IsNearlyEqual(Fourth.X, 5.25f, KINDA_SMALL_NUMBER) && Fourth.Y > 0.0f);
    TestTrue(TEXT("fifth is one equal visual step farther right"),
        FMath::IsNearlyEqual(Fifth.X, 10.5f, KINDA_SMALL_NUMBER) && Fifth.Y < 0.0f);
    TestTrue(TEXT("reverse is one equal visual step farther right"),
        FMath::IsNearlyEqual(Reverse.X, 10.5f, KINDA_SMALL_NUMBER) && Reverse.Y > 0.0f);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabCockpitVisualPawnCompositionTest,
    "PinkCab.Cockpit.VisualDriver.PawnComposition",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPinkCabCockpitVisualPawnCompositionTest::RunTest(const FString& Parameters)
{
    const APinkCabChaosTatraPawn* Pawn = GetDefault<APinkCabChaosTatraPawn>();
    TestNotNull(TEXT("Tatra pawn owns focused cockpit visual driver"), Pawn->GetCockpitVisualDriver());
    return Pawn->GetCockpitVisualDriver() != nullptr;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabCockpitPresentationFlagsTest,
    "PinkCab.Cockpit.VisualDriver.PresentationFlags",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabCockpitPresentationFlagsTest::RunTest(const FString& Parameters)
{
    FPinkCabCockpitPresentationState State;
    State.bIgnitionRunning = true;
    State.bMeterAvailable = true;
    State.bMeterRunning = true;
    State.bPassengerDoorOpen = true;
    State.Handbrake = 0.42f;
    TestTrue(TEXT("ignition presentation can be active"), State.bIgnitionRunning);
    TestTrue(TEXT("meter presentation can be available"), State.bMeterAvailable);
    TestTrue(TEXT("meter running presentation is explicit"), State.bMeterRunning);
    TestTrue(TEXT("door-open presentation is explicit"), State.bPassengerDoorOpen);
    TestEqual(TEXT("analog handbrake presentation is preserved"), State.Handbrake, 0.42f);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabSteeringGeometricPivotTest,
    "PinkCab.Cockpit.VisualDriver.SteeringGeometricPivot",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPinkCabSteeringGeometricPivotTest::RunTest(const FString& Parameters)
{
    const FTransform Base(FRotator(0.0f, 180.0f, 0.0f), FVector(48.0f, -44.0f, 82.0f), FVector::OneVector);
    const FVector MeshCenter(8.0f, -3.0f, 2.0f);
    const FRotator Offset(0.0f, 0.0f, 90.0f);
    const FVector Before = Base.TransformPosition(MeshCenter);
    const FVector NewLocation = UPinkCabCockpitVisualDriverComponent::PivotCompensatedLocation(Base, MeshCenter, Offset);
    const FTransform After(Base.Rotator() + Offset, NewLocation, Base.GetScale3D());
    TestTrue(TEXT("steering mesh center remains fixed while wheel rotates"), Before.Equals(After.TransformPosition(MeshCenter), 0.01f));
    return true;
}
#endif
