#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Engine/StaticMesh.h"
#include "Runtime/PinkCabVehicleVisualProfile.h"
#include "Vehicle/PinkCabVehicleDefinition.h"

namespace
{
FPinkCabVehiclePresentationPart MakePart(const TCHAR* Id)
{
    FPinkCabVehiclePresentationPart Part;
    Part.PartId = FName(Id);
    Part.Mesh = TSoftObjectPtr<UStaticMesh>(
        FSoftObjectPath(TEXT("/Engine/BasicShapes/Cube.Cube")));
    return Part;
}

FPinkCabVehicleWheelBinding MakeWheel(
    const TCHAR* WheelId,
    const TCHAR* BoneName,
    const TCHAR* PartId)
{
    FPinkCabVehicleWheelBinding Wheel;
    Wheel.WheelId = FName(WheelId);
    Wheel.BoneName = FName(BoneName);
    Wheel.PresentationPartId = FName(PartId);
    return Wheel;
}

UPinkCabVehicleDefinition* MakeValidDefinition()
{
    UPinkCabVehicleDefinition* Definition = NewObject<UPinkCabVehicleDefinition>();
    Definition->VehicleId = TEXT("PinkCab.Vehicle.Fixture");
    Definition->PhysicsCarrierMesh = TSoftObjectPtr<USkeletalMesh>(
        FSoftObjectPath(TEXT("/Game/Vehicles/SportsCar/SKM_SportsCar.SKM_SportsCar")));
    Definition->PhysicsAsset = TSoftObjectPtr<UPhysicsAsset>(
        FSoftObjectPath(TEXT("/Game/Vehicles/SportsCar/PA_SportsCar.PA_SportsCar")));
    Definition->DriverMesh = TSoftObjectPtr<USkeletalMesh>(
        FSoftObjectPath(TEXT("/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple.SKM_Manny_Simple")));
    Definition->DriverTransform = FTransform(
        FRotator::ZeroRotator, FVector(-35.0f, -38.0f, -55.0f), FVector(0.92f));
    Definition->DriverHeadTransform = FTransform(
        FRotator::ZeroRotator, FVector(-18.0f, -40.0f, 112.0f));

    Definition->VisualProfile.ProfileId = TEXT("PinkCab.Visual.Fixture");
    Definition->VisualProfile.PresentationParts = {
        MakePart(TEXT("WheelFL")),
        MakePart(TEXT("WheelFR")),
        MakePart(TEXT("WheelRL")),
        MakePart(TEXT("WheelRR"))
    };

    Definition->Wheels = {
        MakeWheel(TEXT("WheelFL"), TEXT("Phys_Wheel_FL"), TEXT("WheelFL")),
        MakeWheel(TEXT("WheelFR"), TEXT("Phys_Wheel_FR"), TEXT("WheelFR")),
        MakeWheel(TEXT("WheelRL"), TEXT("Phys_Wheel_BL"), TEXT("WheelRL")),
        MakeWheel(TEXT("WheelRR"), TEXT("Phys_Wheel_BR"), TEXT("WheelRR"))
    };
    return Definition;
}

FPinkCabVehicleArticulationDefinition MakeHinge(
    const TCHAR* Id,
    const TCHAR* PartId,
    const FVector Axis = FVector::UpVector)
{
    FPinkCabVehicleArticulationDefinition Hinge;
    Hinge.ArticulationId = FName(Id);
    Hinge.PivotLocal = FVector(10.0f, 20.0f, 30.0f);
    Hinge.AxisLocal = Axis;
    Hinge.OpenAngleDegrees = 65.0f;
    Hinge.TravelSeconds = 0.5f;
    Hinge.PartIds = { FName(PartId) };
    return Hinge;
}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabVehicleDefinitionValidTest,
    "PinkCab.Vehicle.Definition.Valid",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabVehicleDefinitionValidTest::RunTest(const FString& Parameters)
{
    UPinkCabVehicleDefinition* Definition = MakeValidDefinition();
    FString Reason;
    TestTrue(TEXT("fixture definition validates"), Definition->IsValid(&Reason));
    TestTrue(TEXT("valid definition has no failure reason"), Reason.IsEmpty());

    const FPinkCabVehicleVisualProfile Profile = Definition->BuildVisualProfile();
    TestEqual(TEXT("profile identity survives definition conversion"),
        Profile.ProfileId, FName(TEXT("PinkCab.Visual.Fixture")));
    TestEqual(TEXT("definition driver head becomes profile driver head"),
        Profile.DriverHeadTransform.GetLocation(),
        FVector(-18.0f, -40.0f, 112.0f));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabVehicleDefinitionDuplicatePartTest,
    "PinkCab.Vehicle.Definition.RejectsDuplicatePart",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabVehicleDefinitionDuplicatePartTest::RunTest(const FString& Parameters)
{
    UPinkCabVehicleDefinition* Definition = MakeValidDefinition();
    Definition->VisualProfile.PresentationParts.Add(MakePart(TEXT("WheelFL")));
    FString Reason;
    TestFalse(TEXT("duplicate part id is rejected"), Definition->IsValid(&Reason));
    TestFalse(TEXT("duplicate part failure is explained"), Reason.IsEmpty());
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabVehicleDefinitionDuplicateArticulationOwnerTest,
    "PinkCab.Vehicle.Definition.RejectsDuplicateArticulationMembership",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabVehicleDefinitionDuplicateArticulationOwnerTest::RunTest(const FString& Parameters)
{
    UPinkCabVehicleDefinition* Definition = MakeValidDefinition();
    Definition->VisualProfile.Articulations.Add(MakeHinge(TEXT("DoorA"), TEXT("WheelFL")));
    Definition->VisualProfile.Articulations.Add(MakeHinge(TEXT("DoorB"), TEXT("WheelFL")));
    FString Reason;
    TestFalse(TEXT("one part cannot belong to two articulations"), Definition->IsValid(&Reason));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabVehicleDefinitionBadAxisTest,
    "PinkCab.Vehicle.Definition.RejectsBadHingeAxis",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabVehicleDefinitionBadAxisTest::RunTest(const FString& Parameters)
{
    UPinkCabVehicleDefinition* Definition = MakeValidDefinition();
    Definition->VisualProfile.Articulations.Add(
        MakeHinge(TEXT("DoorA"), TEXT("WheelFL"), FVector::ZeroVector));
    FString Reason;
    TestFalse(TEXT("zero hinge axis is rejected"), Definition->IsValid(&Reason));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabVehicleDefinitionMissingWheelTest,
    "PinkCab.Vehicle.Definition.RejectsMissingWheelAnchor",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabVehicleDefinitionMissingWheelTest::RunTest(const FString& Parameters)
{
    UPinkCabVehicleDefinition* Definition = MakeValidDefinition();
    Definition->Wheels.Pop();
    FString Reason;
    TestFalse(TEXT("exactly four wheel anchors are required"), Definition->IsValid(&Reason));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabVehicleDefinitionUnresolvedAssetTest,
    "PinkCab.Vehicle.Definition.RejectsUnresolvedRequiredAsset",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabVehicleDefinitionUnresolvedAssetTest::RunTest(const FString& Parameters)
{
    UPinkCabVehicleDefinition* Definition = MakeValidDefinition();
    Definition->PhysicsCarrierMesh = TSoftObjectPtr<USkeletalMesh>(
        FSoftObjectPath(TEXT("/Game/DoesNotExist/SKM_Missing.SKM_Missing")));
    FString Reason;
    TestFalse(TEXT("required carrier mesh must resolve"), Definition->IsValid(&Reason));
    return true;
}

#endif
