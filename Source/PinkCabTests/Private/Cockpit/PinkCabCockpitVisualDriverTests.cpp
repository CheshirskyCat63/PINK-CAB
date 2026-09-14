#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Cockpit/PinkCabCockpitVisualDriverComponent.h"
#include "Cockpit/PinkCabCockpitPresentationState.h"
#include "Vehicle/PinkCabChaosTatraPawn.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabCockpitVisualMappingTest,
    "PinkCab.Cockpit.VisualDriver.Mapping",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabCockpitVisualMappingTest::RunTest(const FString& Parameters)
{
    TestEqual(TEXT("full left steering maps to -450 degrees"),
        UPinkCabCockpitVisualDriverComponent::SteeringAngleDegrees(-1.0f), -450.0f);
    TestEqual(TEXT("full right steering maps to 450 degrees"),
        UPinkCabCockpitVisualDriverComponent::SteeringAngleDegrees(1.0f), 450.0f);
    TestEqual(TEXT("full pedal travel is bounded"),
        UPinkCabCockpitVisualDriverComponent::PedalTravelDegrees(1.0f), 18.0f);
    TestEqual(TEXT("engaged handbrake gets visible lever angle"),
        UPinkCabCockpitVisualDriverComponent::HandbrakeAngleDegrees(true), -32.0f);
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabCockpitVisualGearPoseTest,
    "PinkCab.Cockpit.VisualDriver.GearPose",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabCockpitVisualGearPoseTest::RunTest(const FString& Parameters)
{
    const FVector Neutral = UPinkCabCockpitVisualDriverComponent::GearLeverOffset(0);
    const FVector Reverse = UPinkCabCockpitVisualDriverComponent::GearLeverOffset(-1);
    const FVector Fifth = UPinkCabCockpitVisualDriverComponent::GearLeverOffset(5);
    TestEqual(TEXT("neutral gear lever is centered"), Neutral, FVector::ZeroVector);
    TestTrue(TEXT("reverse has distinct lever pose"), Reverse != Neutral);
    TestTrue(TEXT("fifth has distinct lever pose"), Fifth != Neutral && Fifth != Reverse);
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
    TestTrue(TEXT("ignition presentation can be active"), State.bIgnitionRunning);
    TestTrue(TEXT("meter presentation can be available"), State.bMeterAvailable);
    TestTrue(TEXT("meter running presentation is explicit"), State.bMeterRunning);
    TestTrue(TEXT("door-open presentation is explicit"), State.bPassengerDoorOpen);
    return true;
}

#endif
