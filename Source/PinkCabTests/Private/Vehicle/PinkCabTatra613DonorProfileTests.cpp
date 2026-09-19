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
    TestEqual(TEXT("scene profile contains all 133 source meshes plus four donor wheels"),
        Profile.PresentationParts.Num(), 137);

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
    TestEqual(TEXT("all 133 Blender scene meshes stay authored as untouched scene parts"), SourceScenePartCount, 133);

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

    TestEqual(TEXT("desktop Tatra uses no replacement cockpit mesh overrides"),
        Profile.CockpitBindings.Num(), 0);
    TestFalse(TEXT("source steering part is tagged for runtime pivoting"),
        Profile.SteeringPresentationPartId.IsNone());
    const FPinkCabVehiclePresentationPart* SourceSteering =
        Profile.PresentationParts.FindByPredicate([&Profile](const FPinkCabVehiclePresentationPart& Part)
        {
            return Part.PartId == Profile.SteeringPresentationPartId;
        });
    TestNotNull(TEXT("tagged steering part exists"), SourceSteering);
    if (SourceSteering)
    {
        TestTrue(TEXT("tagged steering part is the preserved t613_steer object"),
            SourceSteering->Mesh.ToSoftObjectPath().ToString().Contains(TEXT("t613_Black_material_021")));
    }
    return true;
}

#endif
