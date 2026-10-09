#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Engine/SkeletalMesh.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "PhysicsEngine/SkeletalBodySetup.h"
#include "Cockpit/PinkCabPrototypeVisualProfile.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPinkCabTatraPhysicalRigContractTest,
    "PinkCab.Vehicle.PhysicalFoundation.AuthoredRigContract",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPinkCabTatraPhysicalRigContractTest::RunTest(const FString& Parameters)
{
    const FPinkCabPrototypeVisualProfile Profile = FPinkCabPrototypeVisualProfile::Tatra613Physical();
    USkeletalMesh* Mesh = Cast<USkeletalMesh>(Profile.VehicleMeshPath.TryLoad());
    UPhysicsAsset* Asset = Cast<UPhysicsAsset>(Profile.PhysicsAssetPath.TryLoad());
    if (!TestNotNull(TEXT("authored physical mesh"), Mesh)
        || !TestNotNull(TEXT("authored Tatra collision"), Asset)) return false;
    TestEqual(TEXT("five bone rig, no donor suspension constraints"), Mesh->GetRefSkeleton().GetNum(), 5);
    TestEqual(TEXT("one native physical chassis body"), Asset->SkeletalBodySetups.Num(), 1);
    TestEqual(TEXT("no donor wheel constraints"), Asset->ConstraintSetup.Num(), 0);
    TestTrue(TEXT("physics root is identity in centimetres"),
        Mesh->GetRefSkeleton().GetRefBonePose()[0].Equals(FTransform::Identity, 0.001));
    if (Asset->SkeletalBodySetups.Num() != 1) return false;
    const USkeletalBodySetup* Body = Asset->SkeletalBodySetups[0];
    TestEqual(TEXT("chassis owns root"), Body->BoneName, FName(TEXT("Root")));
    TestEqual(TEXT("lower body and upper cabin physical envelopes"), Body->AggGeom.ConvexElems.Num(), 2);
    const FBox Box = Body->AggGeom.CalcAABB(FTransform::Identity);
    TestTrue(TEXT("physical hull represents Tatra length not SportsCar bounds"),
        Box.GetSize().X > 480.0 && Box.GetSize().X < 490.0);
    TestTrue(TEXT("physical hull represents Tatra body width"),
        Box.GetSize().Y > 190.0 && Box.GetSize().Y < 200.0);
    TestTrue(TEXT("chassis clears road while including its roof"), Box.Min.Z > 15.0 && Box.Max.Z > 140.0);
    const FVector Expected[] = {{135,-76,25},{135,76,25},{-163,-76,26.5},{-163,76,26.5}};
    for (int32 Index = 0; Index < 4; ++Index)
    {
        TestTrue(TEXT("authored physical wheel has accepted centre and correct side"),
            Mesh->GetComposedRefPoseMatrix(Profile.WheelBones[Index]).GetOrigin().Equals(Expected[Index], 0.1));
    }
    return true;
}
#endif
