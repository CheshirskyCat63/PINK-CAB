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
    TestEqual(TEXT("Tatra 613 desktop scene profile id"), Profile.ProfileId,
        FName(TEXT("PinkCab.Visual.Tatra613.ScenePreserved")));
    TestTrue(TEXT("scene profile does not use a merged exterior mesh"), Profile.ExteriorStaticMesh.IsNull());
    TestFalse(TEXT("scene profile does not duplicate exterior into a cabin mesh"), Profile.bUseExteriorAsCabinWhenCabinMissing);
    TestEqual(TEXT("scene profile contains 132 static source meshes plus four donor wheels; steering is live"),
        Profile.PresentationParts.Num(), 136);

    int32 WheelPartCount = 0;
    int32 SourceScenePartCount = 0;
    TSet<FName> PartIds;
    for (const FPinkCabVehiclePresentationPart& Part : Profile.PresentationParts)
    {
        TestTrue(TEXT("presentation part is valid"), Part.IsValid());
        TestTrue(TEXT("presentation part ids are unique"), !PartIds.Contains(Part.PartId));
        PartIds.Add(Part.PartId);
        if (Part.PartId.ToString().StartsWith(TEXT("Wheel")))
        {
            ++WheelPartCount;
            TestTrue(TEXT("V12 wheels use archive wheel mesh"),
                Part.Mesh.ToSoftObjectPath().ToString().Contains(TEXT("Tatra613ArchiveV12Clean/Tatra613_V12_Wheel")));
        }
        else
        {
            ++SourceScenePartCount;
            TestTrue(TEXT("source scene part uses untouched desktop scene mesh"),
                Part.Mesh.ToSoftObjectPath().ToString().Contains(TEXT("Tatra613DesktopScene/Tatra613_ScenePreserved/StaticMeshes")));
        }
    }
    TestEqual(TEXT("four visual wheel instances are authored"), WheelPartCount, 4);
    TestEqual(TEXT("all non-steering Blender mesh objects stay authored as untouched scene parts"), SourceScenePartCount, 132);

    const auto FindPart = [&Profile](const TCHAR* Id)
    {
        return Profile.PresentationParts.FindByPredicate([Id](const FPinkCabVehiclePresentationPart& Part)
        {
            return Part.PartId == FName(Id);
        });
    };
    const auto* WheelFL = FindPart(TEXT("WheelFL"));
    const auto* WheelFR = FindPart(TEXT("WheelFR"));
    const auto* WheelRL = FindPart(TEXT("WheelRL"));
    const auto* WheelRR = FindPart(TEXT("WheelRR"));
    TestNotNull(TEXT("front-left wheel contract exists"), WheelFL);
    TestNotNull(TEXT("front-right wheel contract exists"), WheelFR);
    TestNotNull(TEXT("rear-left wheel contract exists"), WheelRL);
    TestNotNull(TEXT("rear-right wheel contract exists"), WheelRR);
    if (WheelFL && WheelFR && WheelRL && WheelRR)
    {
        const float WheelbaseCm = FMath::Abs(WheelFL->LocalTransform.GetLocation().X - WheelRL->LocalTransform.GetLocation().X);
        const float FrontTrackCm = FMath::Abs(WheelFR->LocalTransform.GetLocation().Y - WheelFL->LocalTransform.GetLocation().Y);
        const float RearTrackCm = FMath::Abs(WheelRR->LocalTransform.GetLocation().Y - WheelRL->LocalTransform.GetLocation().Y);
        TestEqual(TEXT("visual wheelbase matches Tatra 613 source"), WheelbaseCm, 298.0f);
        TestEqual(TEXT("visual front track matches Tatra 613 source"), FrontTrackCm, 152.0f);
        TestEqual(TEXT("visual rear track matches Tatra 613 source"), RearTrackCm, 152.0f);
    }

    TestEqual(TEXT("desktop Tatra uses exactly one live cockpit visual override"),
        Profile.CockpitBindings.Num(), 1);
    if (Profile.CockpitBindings.Num() == 1)
    {
        TestEqual(TEXT("only steering wheel is allowed to be a live cockpit override"),
            Profile.CockpitBindings[0].Slot, EPinkCabCockpitSlot::SteeringWheel);
        TestTrue(TEXT("steering override uses the dedicated V12 donor mesh"),
            Profile.CockpitBindings[0].MeshOverride.ToSoftObjectPath().ToString().Contains(TEXT("Tatra613_V12_Steering")));
    }
    return true;
}

#endif
