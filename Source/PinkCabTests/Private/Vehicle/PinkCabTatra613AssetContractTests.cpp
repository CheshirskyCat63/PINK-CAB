#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"
#include "Materials/Material.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "Vehicle/PinkCabChaosPhysicalProfile.h"
#include "Runtime/PinkCabChaosTatraPawn.h"
#include "Vehicle/PinkCabVehicleVisualProfile.h"

namespace
{
const FPinkCabVehiclePresentationPart* FindPart(
    const FPinkCabVehicleVisualProfile& Profile,
    const FName PartId)
{
    return Profile.PresentationParts.FindByPredicate([PartId](const FPinkCabVehiclePresentationPart& Part)
    {
        return Part.PartId == PartId;
    });
}

bool IsPositiveFiniteScale(const FTransform& Transform)
{
    const FVector Scale = Transform.GetScale3D();
    return !Transform.ContainsNaN()
        && FMath::IsFinite(Scale.X) && FMath::IsFinite(Scale.Y) && FMath::IsFinite(Scale.Z)
        && Scale.X > 0.0f && Scale.Y > 0.0f && Scale.Z > 0.0f;
}

bool HasFiniteNonDegenerateBounds(const UStaticMesh& Mesh)
{
    const FBoxSphereBounds Bounds = Mesh.GetBounds();
    const FVector Extent = Bounds.BoxExtent;
    return FMath::IsFinite(Extent.X) && FMath::IsFinite(Extent.Y) && FMath::IsFinite(Extent.Z)
        && Extent.X > 0.1f && Extent.Y > 0.1f && Extent.Z > 0.1f;
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabTatra613V12AssetContractTest,
    "PinkCab.Vehicle.AssetContract.Tatra613V12",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabTatra613V12AssetContractTest::RunTest(const FString& Parameters)
{
    const FPinkCabVehicleVisualProfile Visual = FPinkCabVehicleVisualProfile::Tatra613Donor();
    const FPinkCabChaosPhysicalProfile Physical = FPinkCabChaosPhysicalProfile::ForVariant(EPinkCabCalibrationVariant::Nominal);

    TestEqual(TEXT("desktop TATRA613 does not replace source cockpit geometry"),
        Visual.CockpitBindings.Num(), 0);
    TestTrue(TEXT("scene-preserved profile does not use merged exterior mesh"),
        Visual.ExteriorStaticMesh.IsNull());
    TestEqual(TEXT("scene-preserved profile keeps all 133 source meshes plus four donor wheels"),
        Visual.PresentationParts.Num(), 137);
    TestFalse(TEXT("source steering presentation part is identified"),
        Visual.SteeringPresentationPartId.IsNone());
    const FPinkCabVehiclePresentationPart* SourceSteering =
        FindPart(Visual, Visual.SteeringPresentationPartId);
    TestNotNull(TEXT("source steering part exists in untouched scene"), SourceSteering);
    if (SourceSteering)
    {
        TestTrue(TEXT("source steering is the actual preserved t613_steer mesh"),
            SourceSteering->Mesh.ToSoftObjectPath().ToString().Contains(TEXT("t613_Black_material_021")));
    }
    TestTrue(TEXT("steering pivot axis is finite and normalized"),
        !Visual.SteeringPresentationAxis.ContainsNaN()
        && FMath::IsNearlyEqual(Visual.SteeringPresentationAxis.Size(), 1.0f, 0.001f));

    TestTrue(TEXT("Tatra visual profile is structurally valid"), Visual.IsValid());
    TestEqual(TEXT("source wheelbase is 2980 mm"), Physical.WheelbaseMm.Value, 2980.0f);
    TestEqual(TEXT("source front track is 1520 mm"), Physical.FrontTrackMm.Value, 1520.0f);
    TestEqual(TEXT("source rear track is 1520 mm"), Physical.RearTrackMm.Value, 1520.0f);

    // 205/70 R14: 14*25.4 + 2*(205*0.70) = 642.6 mm outer diameter.
    constexpr float SourceWheelRadiusCm = 32.13f;
    constexpr float SourceWheelWidthCm = 20.5f;
    TestTrue(TEXT("front wheel radius comes from Tatra source geometry"),
        Physical.FrontWheel.WheelRadiusCm.Authority == EPinkCabPhysicalParameterAuthority::Source);
    TestTrue(TEXT("front wheel width comes from Tatra source geometry"),
        Physical.FrontWheel.WheelWidthCm.Authority == EPinkCabPhysicalParameterAuthority::Source);
    TestTrue(TEXT("rear wheel radius comes from Tatra source geometry"),
        Physical.RearWheel.WheelRadiusCm.Authority == EPinkCabPhysicalParameterAuthority::Source);
    TestTrue(TEXT("rear wheel width comes from Tatra source geometry"),
        Physical.RearWheel.WheelWidthCm.Authority == EPinkCabPhysicalParameterAuthority::Source);
    TestTrue(TEXT("front physical radius matches 205/70 R14"),
        FMath::IsNearlyEqual(Physical.FrontWheel.WheelRadiusCm.Value, SourceWheelRadiusCm, 0.01f));
    TestTrue(TEXT("front physical width matches 205/70 R14"),
        FMath::IsNearlyEqual(Physical.FrontWheel.WheelWidthCm.Value, SourceWheelWidthCm, 0.01f));
    TestTrue(TEXT("rear physical radius matches front"),
        FMath::IsNearlyEqual(Physical.RearWheel.WheelRadiusCm.Value, SourceWheelRadiusCm, 0.01f));
    TestTrue(TEXT("rear physical width matches front"),
        FMath::IsNearlyEqual(Physical.RearWheel.WheelWidthCm.Value, SourceWheelWidthCm, 0.01f));

    const FPinkCabVehiclePresentationPart* SourceBodyPart = Visual.PresentationParts.FindByPredicate(
        [](const FPinkCabVehiclePresentationPart& Part)
        {
            return Part.Mesh.ToSoftObjectPath().ToString().Contains(TEXT("/pessimat613_body_material.pessimat613_body_material"));
        });
    TestNotNull(TEXT("source body scene part exists"), SourceBodyPart);
    UStaticMesh* SourceBodyMesh = SourceBodyPart ? SourceBodyPart->Mesh.LoadSynchronous() : nullptr;
    TestNotNull(TEXT("source body scene mesh resolves"), SourceBodyMesh);
    if (SourceBodyMesh)
    {
        TestTrue(TEXT("source body mesh has finite non-degenerate bounds"),
            HasFiniteNonDegenerateBounds(*SourceBodyMesh));
    }

    const FName WheelIds[4] = { TEXT("WheelFL"), TEXT("WheelFR"), TEXT("WheelRL"), TEXT("WheelRR") };
    TSet<FVector> WheelCenters;
    for (const FName WheelId : WheelIds)
    {
        const FPinkCabVehiclePresentationPart* WheelPart = FindPart(Visual, WheelId);
        TestNotNull(*FString::Printf(TEXT("%s donor wheel exists"), *WheelId.ToString()), WheelPart);
        if (!WheelPart)
        {
            return false;
        }
        TestTrue(*FString::Printf(TEXT("%s uses the validated donor wheel"), *WheelId.ToString()),
            WheelPart->Mesh.ToSoftObjectPath().ToString().Contains(TEXT("Tatra613ArchiveV12Clean/Tatra613_V12_Wheel")));
        TestTrue(*FString::Printf(TEXT("%s transform is positive and non-mirrored"), *WheelId.ToString()),
            IsPositiveFiniteScale(WheelPart->LocalTransform));
        WheelCenters.Add(WheelPart->LocalTransform.GetLocation());
    }
    TestEqual(TEXT("all four donor wheel centers are unique"), WheelCenters.Num(), 4);

    const APinkCabChaosTatraPawn* Pawn = GetDefault<APinkCabChaosTatraPawn>();
    const USkeletalMeshComponent* Carrier = Pawn ? Pawn->GetMesh() : nullptr;
    USkeletalMesh* CarrierMesh = Carrier ? Carrier->GetSkeletalMeshAsset() : nullptr;
    TestNotNull(TEXT("temporary Chaos skeletal carrier resolves"), CarrierMesh);
    TestNotNull(TEXT("temporary Chaos physics asset resolves"), Carrier ? Carrier->GetPhysicsAsset() : nullptr);
    if (CarrierMesh)
    {
        const FReferenceSkeleton& RefSkeleton = CarrierMesh->GetRefSkeleton();
        for (const FName Bone : { FName(TEXT("Phys_Wheel_FL")), FName(TEXT("Phys_Wheel_FR")), FName(TEXT("Phys_Wheel_BL")), FName(TEXT("Phys_Wheel_BR")) })
        {
            TestTrue(*FString::Printf(TEXT("required carrier bone %s exists"), *Bone.ToString()),
                RefSkeleton.FindBoneIndex(Bone) != INDEX_NONE);
        }
    }

    const UChaosWheeledVehicleMovementComponent* Movement = Pawn ? Pawn->GetChaosMovement() : nullptr;
    TestNotNull(TEXT("Chaos movement resolves"), Movement);
    TestEqual(TEXT("validator requires four bound wheel setups"), Movement ? Movement->WheelSetups.Num() : 0, 4);
    return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabTatra613DesktopCookContractTest,
    "PinkCab.Vehicle.AssetContract.Tatra613DesktopCook",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabTatra613DesktopCookContractTest::RunTest(const FString& Parameters)
{
    FString DefaultGame;
    const FString ConfigPath = FPaths::Combine(FPaths::ProjectConfigDir(), TEXT("DefaultGame.ini"));
    TestTrue(TEXT("DefaultGame.ini is readable"), FFileHelper::LoadFileToString(DefaultGame, *ConfigPath));
    TestTrue(TEXT("desktop Tatra root is explicitly included in package cook"),
        DefaultGame.Contains(TEXT("+DirectoriesToAlwaysCook=(Path=\"/Game/Dev/Vehicles/Tatra613DesktopScene\")")));
    return true;
}

#endif
