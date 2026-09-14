#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Cockpit/PinkCabPrototypeVisualProfile.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabPrototypeVisualProfileDefaultsTest,
    "PinkCab.Cockpit.PrototypeVisual.ProfileDefaults",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabPrototypeVisualProfileDefaultsTest::RunTest(const FString& Parameters)
{
    const FPinkCabPrototypeVisualProfile Profile =
        FPinkCabPrototypeVisualProfile::EpicSportsCarManny();

    TestTrue(TEXT("prototype profile is structurally valid"), Profile.IsValid());
    TestEqual(TEXT("vehicle mesh stays replaceable"), Profile.VehicleMeshPath.ToString(),
        FString(TEXT("/Game/Vehicles/SportsCar/SKM_SportsCar.SKM_SportsCar")));
    TestEqual(TEXT("driver mesh stays replaceable"), Profile.DriverMeshPath.ToString(),
        FString(TEXT("/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple.SKM_Manny_Simple")));
    TestEqual(TEXT("front-left wheel bone"), Profile.WheelBones[0], FName(TEXT("Phys_Wheel_FL")));
    TestEqual(TEXT("front-right wheel bone"), Profile.WheelBones[1], FName(TEXT("Phys_Wheel_FR")));
    TestEqual(TEXT("rear-left wheel bone"), Profile.WheelBones[2], FName(TEXT("Phys_Wheel_BL")));
    TestEqual(TEXT("rear-right wheel bone"), Profile.WheelBones[3], FName(TEXT("Phys_Wheel_BR")));
    TestFalse(TEXT("driver transform is explicit, not implicit identity"),
        Profile.DriverTransform.Equals(FTransform::Identity));
    return true;
}

#endif
