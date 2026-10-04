#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Cockpit/PinkCabPrototypeVisualProfile.h"
#include "Runtime/PinkCabVehicleVisualProfile.h"
#include "Vehicle/PinkCabVehicleDefinitionTestUtils.h"

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

    const UPinkCabVehicleDefinition* TatraDefinition =
        PinkCabVehicleDefinitionTestUtils::LoadTatra();
    TestNotNull(TEXT("Tatra vehicle definition resolves"), TatraDefinition);
    if (!TatraDefinition) return false;
    const FPinkCabVehicleVisualProfile Tatra = TatraDefinition->BuildVisualProfile();
    TestTrue(TEXT("Tatra presentation profile is structurally valid"), Tatra.IsValid());
    TestEqual(TEXT("Tatra owns six presentation articulations"), Tatra.Articulations.Num(), 6);
    for (const FPinkCabVehicleArticulationDefinition& Hinge : Tatra.Articulations)
    {
        TestTrue(*FString::Printf(TEXT("%s articulation valid"), *Hinge.ArticulationId.ToString()), Hinge.IsValid());
    }
    const FPinkCabVehicleArticulationDefinition* FrontLid =
        Tatra.Articulations.FindByPredicate([](const FPinkCabVehicleArticulationDefinition& Hinge)
        {
            return Hinge.ArticulationId == FName(TEXT("FrontLid"));
        });
    TestNotNull(TEXT("reverse front lid is declared"), FrontLid);
    if (FrontLid)
    {
        TestTrue(TEXT("reverse front lid opens around lateral hinge"),
            FMath::Abs(FrontLid->AxisLocal.Y) > 0.9f);
        TestTrue(TEXT("reverse front lid pivot matches authored front edge"),
            FrontLid->PivotLocal.Equals(FVector(235.2f, 0.0f, 82.0f), 0.25f));
        TestTrue(TEXT("reverse front lid has donor body panel"),
            FrontLid->PartIds.Num() > 0);
    }

    const FPinkCabVehicleArticulationDefinition* DoorFL =
        Tatra.Articulations.FindByPredicate([](const FPinkCabVehicleArticulationDefinition& Hinge)
        {
            return Hinge.ArticulationId == FName(TEXT("DoorFL"));
        });
    TestNotNull(TEXT("front-left door articulation is declared"), DoorFL);
    if (DoorFL)
    {
        TestTrue(TEXT("front-left hinge matches authored door edge"),
            DoorFL->PivotLocal.Equals(FVector(73.5f, -91.0f, 85.0f), 0.25f));
        TestTrue(TEXT("front-left door rotates around vertical axis"),
            FMath::Abs(DoorFL->AxisLocal.Z) > 0.9f);
    }

    const FPinkCabVehicleArticulationDefinition* RearLid =
        Tatra.Articulations.FindByPredicate([](const FPinkCabVehicleArticulationDefinition& Hinge)
        {
            return Hinge.ArticulationId == FName(TEXT("RearLid"));
        });
    TestNotNull(TEXT("rear lid articulation is declared"), RearLid);
    if (RearLid)
    {
        TestTrue(TEXT("rear lid pivot matches authored forward edge"),
            RearLid->PivotLocal.Equals(FVector(-187.4f, 0.0f, 85.0f), 0.25f));
        TestTrue(TEXT("rear lid opens around lateral hinge"),
            FMath::Abs(RearLid->AxisLocal.Y) > 0.9f);
    }

    const FPinkCabVehicleVisualProfile Visual = FPinkCabVehicleVisualProfile::Fallback();
    TestTrue(TEXT("visual profile is valid without donor art"), Visual.IsValid());
    TestFalse(TEXT("fallback visual profile has no exterior donor"), Visual.HasExteriorAsset());
    TestFalse(TEXT("fallback visual profile has no cabin donor"), Visual.HasCabinAsset());
    TestEqual(TEXT("visual profile identity is independent from physics chassis"),
        Visual.ProfileId, FName(TEXT("PinkCab.Visual.Fallback")));
    return true;
}

#endif
