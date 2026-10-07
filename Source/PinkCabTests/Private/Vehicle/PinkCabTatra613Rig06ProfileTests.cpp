#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "ReferenceSkeleton.h"
#include "Engine/SkeletalMesh.h"
#include "Runtime/PinkCabVehicleVisualProfile.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabTatra613Rig06ProfileTest,
    "PinkCab.Vehicle.Visual.Tatra613Rig06Profile",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabTatra613Rig06ProfileTest::RunTest(const FString& Parameters)
{
    const FPinkCabVehicleVisualProfile Profile =
        FPinkCabVehicleVisualProfile::Tatra613Rig06();

    TestTrue(TEXT("RIG06 profile validates"), Profile.IsValid());
    TestEqual(TEXT("RIG06 profile id"), Profile.ProfileId,
        FName(TEXT("PinkCab.Visual.Tatra613.Rig06.TexturedOpenables")));
    TestTrue(TEXT("RIG06 uses skeletal exterior"), !Profile.ExteriorSkeletalMesh.IsNull());
    TestTrue(TEXT("RIG06 uses skeletal cabin"), !Profile.CabinSkeletalMesh.IsNull());
    TestTrue(TEXT("RIG06 uses poseable presentation"), Profile.bUsePoseableSkeletalPresentation);
    TestEqual(TEXT("RIG06 has no legacy static presentation parts"),
        Profile.PresentationParts.Num(), 0);
    TestTrue(TEXT("RIG06 V22 presentation scale aligns visual wheelbase to P4"),
        Profile.ExteriorTransform.GetScale3D().Equals(
            FVector(95.9121f, 100.0f, 100.0f), 0.001f));
    TestTrue(TEXT("RIG06 accepted axle-center X alignment"),
        FMath::IsNearlyEqual(Profile.ExteriorTransform.GetLocation().X, 5.5167f, 0.05f));
    TestTrue(TEXT("RIG06 requires no legacy scene yaw"),
        Profile.ExteriorTransform.GetRotation().Equals(FQuat::Identity, 0.001f));

    USkeletalMesh* Mesh = Profile.ExteriorSkeletalMesh.LoadSynchronous();
    TestNotNull(TEXT("RIG06 skeletal mesh loads"), Mesh);
    if (!Mesh) return false;

    const FReferenceSkeleton& Ref = Mesh->GetRefSkeleton();
    const TArray<FTransform>& Pose = Ref.GetRefBonePose();
    static const FName RequiredBones[] = {
        TEXT("root"),
        TEXT("Phys_Wheel_FL"), TEXT("Phys_Wheel_FR"),
        TEXT("Phys_Wheel_BL"), TEXT("Phys_Wheel_BR"),
        TEXT("Steering_Wheel"), TEXT("Cabin_GearLever"),
        TEXT("Cabin_ClutchPedal"), TEXT("Cabin_BrakePedal"),
        TEXT("Cabin_ThrottlePedal"), TEXT("Cabin_Handbrake"),
        TEXT("Cabin_Horn"), TEXT("Cabin_Stalk_L"), TEXT("Cabin_Stalk_R"),
        TEXT("Cabin_Radio"), TEXT("Cabin_Climate"),
        TEXT("Door_FL"), TEXT("Door_FR"), TEXT("Door_RL"), TEXT("Door_RR"),
        TEXT("Window_FL"), TEXT("Window_FR"), TEXT("Window_RL"), TEXT("Window_RR"),
        TEXT("Trunk_Front"), TEXT("Hood_Rear"),
        TEXT("Mirror_L"), TEXT("Mirror_R"),
        TEXT("Cabin_SpeedometerNeedle"), TEXT("Cabin_TachometerNeedle"),
        TEXT("Cabin_FuelNeedle"), TEXT("Cabin_TemperatureNeedle")
    };
    for (const FName BoneName : RequiredBones)
    {
        TestTrue(
            *FString::Printf(TEXT("RIG06 required bone exists: %s"), *BoneName.ToString()),
            Ref.FindBoneIndex(BoneName) != INDEX_NONE);
    }

    const auto WorldRest = [&Ref, &Pose, &Profile](const FName BoneName)
    {
        const int32 Index = Ref.FindBoneIndex(BoneName);
        if (Index == INDEX_NONE) return FVector::ZeroVector;

        // Wheel/openable bones are direct root children in RIG06. Still build
        // the component-space chain so this test remains valid if the armature
        // hierarchy later gains an authored alignment parent.
        FTransform BoneToRoot = Pose[Index];
        int32 Parent = Ref.GetParentIndex(Index);
        while (Parent != INDEX_NONE)
        {
            BoneToRoot = BoneToRoot * Pose[Parent];
            Parent = Ref.GetParentIndex(Parent);
        }
        return Profile.ExteriorTransform.TransformPosition(
            BoneToRoot.GetLocation());
    };

    const FVector FL = WorldRest(TEXT("Phys_Wheel_FL"));
    const FVector FR = WorldRest(TEXT("Phys_Wheel_FR"));
    const FVector RL = WorldRest(TEXT("Phys_Wheel_BL"));
    const FVector RR = WorldRest(TEXT("Phys_Wheel_BR"));
    TestTrue(TEXT("RIG06 wheelbase is 298 cm"),
        FMath::IsNearlyEqual(FMath::Abs(FL.X - RL.X), 298.0f, 0.2f));
    TestTrue(TEXT("RIG06 front track is 152 cm"),
        FMath::IsNearlyEqual(FMath::Abs(FL.Y - FR.Y), 152.0f, 0.2f));
    TestTrue(TEXT("RIG06 rear track is 152 cm"),
        FMath::IsNearlyEqual(FMath::Abs(RL.Y - RR.Y), 152.0f, 0.2f));
    TestTrue(TEXT("RIG06 front axle aligns accepted P4 visual center"),
        FMath::IsNearlyEqual((FL.X + FR.X) * 0.5f, 135.0f, 0.2f));
    TestTrue(TEXT("RIG06 rear axle aligns accepted P4 visual center"),
        FMath::IsNearlyEqual((RL.X + RR.X) * 0.5f, -163.0f, 0.2f));

    const FVector ImportedFullSize =
        Mesh->GetBounds().BoxExtent * Profile.ExteriorTransform.GetScale3D() * 2.0f;
    TestTrue(TEXT("RIG06 full length is plausible Tatra 613 size"),
        ImportedFullSize.X > 500.0f && ImportedFullSize.X < 550.0f);
    TestTrue(TEXT("RIG06 full width is plausible Tatra 613 size"),
        ImportedFullSize.Y > 195.0f && ImportedFullSize.Y < 220.0f);
    TestTrue(TEXT("RIG06 full height is plausible Tatra 613 size"),
        ImportedFullSize.Z > 140.0f && ImportedFullSize.Z < 165.0f);

    const TArray<FSkeletalMaterial>& Materials = Mesh->GetMaterials();
    TestEqual(TEXT("RIG06 retains 18 material slots"), Materials.Num(), 18);
    TSet<FName> MaterialNames;
    for (const FSkeletalMaterial& Material : Materials)
    {
        MaterialNames.Add(Material.MaterialSlotName);
        TestNotNull(
            *FString::Printf(TEXT("RIG06 slot has material: %s"), *Material.MaterialSlotName.ToString()),
            Material.MaterialInterface.Get());
    }
    for (const FName RequiredMaterial : {
        FName(TEXT("M_PC_CarPaint_PinkCab")),
        FName(TEXT("M_PC_Chrome_Aged")),
        FName(TEXT("M_PC_Rubber_Tyre")),
        FName(TEXT("M_PC_Pedal_OEM_BlackSteel")),
        FName(TEXT("M_PC_Glass_Clear")),
        FName(TEXT("M_PC_Velour_Charcoal")) })
    {
        TestTrue(
            *FString::Printf(TEXT("RIG06 material slot retained: %s"), *RequiredMaterial.ToString()),
            MaterialNames.Contains(RequiredMaterial));
    }
    return true;
}

#endif
