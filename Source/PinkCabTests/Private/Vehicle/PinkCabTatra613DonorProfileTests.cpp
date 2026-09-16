#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Engine/StaticMesh.h"
#include "Cockpit/PinkCabCockpitVisualBinding.h"
#include "Vehicle/PinkCabVehicleVisualProfile.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabTatra613DonorProfileTest,
    "PinkCab.Vehicle.Visual.Tatra613DonorProfile",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabTatra613DonorProfileTest::RunTest(const FString& Parameters)
{
    const FPinkCabVehicleVisualProfile Profile = FPinkCabVehicleVisualProfile::Tatra613Donor();
    TestTrue(TEXT("Tatra 613 donor profile validates"), Profile.IsValid());
    TestEqual(TEXT("Tatra 613 donor profile id"), Profile.ProfileId,
        FName(TEXT("PinkCab.Visual.Tatra613.Donor")));
    TestTrue(TEXT("donor main mesh resolves"), Profile.ExteriorStaticMesh.LoadSynchronous() != nullptr);
    TestTrue(TEXT("same donor can supply owner cabin"), Profile.bUseExteriorAsCabinWhenCabinMissing);
    TestEqual(TEXT("donor has bounded logical presentation parts"), Profile.PresentationParts.Num(), 10);

    int32 WheelPartCount = 0;
    TSet<FName> PartIds;
    for (const FPinkCabVehiclePresentationPart& Part : Profile.PresentationParts)
    {
        TestTrue(TEXT("presentation part is valid"), Part.IsValid());
        TestTrue(TEXT("presentation part ids are unique"), !PartIds.Contains(Part.PartId));
        PartIds.Add(Part.PartId);
        if (Part.PartId.ToString().StartsWith(TEXT("Wheel"))) ++WheelPartCount;
    }
    TestEqual(TEXT("four visual wheel instances are authored"), WheelPartCount, 4);
    const FPinkCabCockpitVisualBinding* Steering = Profile.CockpitBindings.FindByPredicate(
        [](const FPinkCabCockpitVisualBinding& Binding)
        {
            return Binding.Slot == EPinkCabCockpitSlot::SteeringWheel;
        });
    TestNotNull(TEXT("donor steering binds to canonical steering slot"), Steering);
    if (Steering)
    {
        TestTrue(TEXT("donor steering mesh resolves"), Steering->MeshOverride.LoadSynchronous() != nullptr);
        TestFalse(TEXT("donor steering uses visible mesh"), !Steering->bShowAnchorMesh);
    }
    return true;
}

#endif
