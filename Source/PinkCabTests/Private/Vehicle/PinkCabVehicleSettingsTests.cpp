#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Vehicle/PinkCabVehicleSettings.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabVehicleSettingsDefaultPathTest,
    "PinkCab.Vehicle.Settings.UsesDefaultDefinition",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabVehicleSettingsDefaultPathTest::RunTest(const FString& Parameters)
{
    const FSoftObjectPath DefaultPath(TEXT("/Game/Dev/Vehicles/Fixture/DA_Default.DA_Default"));
    TestEqual(
        TEXT("default definition is used without override"),
        UPinkCabVehicleSettings::ResolveDefinitionPath(TEXT("-game"), DefaultPath),
        DefaultPath);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabVehicleSettingsCommandLineOverrideTest,
    "PinkCab.Vehicle.Settings.CommandLineOverride",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabVehicleSettingsCommandLineOverrideTest::RunTest(const FString& Parameters)
{
    const FSoftObjectPath DefaultPath(TEXT("/Game/Dev/Vehicles/Fixture/DA_Default.DA_Default"));
    const FSoftObjectPath OverridePath(TEXT("/Game/Dev/Vehicles/Other/DA_Other.DA_Other"));
    const FString CommandLine =
        TEXT("-game -PinkCabVehicleDefinition=/Game/Dev/Vehicles/Other/DA_Other.DA_Other");
    TestEqual(
        TEXT("command line definition overrides config default"),
        UPinkCabVehicleSettings::ResolveDefinitionPath(CommandLine, DefaultPath),
        OverridePath);
    return true;
}

#endif
