#include "Vehicle/PinkCabVehicleVisualProfile.h"

#include "Engine/StaticMesh.h"

namespace
{
TSoftObjectPtr<UStaticMesh> MeshAt(const TCHAR* Path)
{
    return TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(Path));
}

FPinkCabVehiclePresentationPart Part(
    const FName Id,
    const TCHAR* MeshPath,
    const FTransform& Transform)
{
    FPinkCabVehiclePresentationPart Result;
    Result.PartId = Id;
    Result.Mesh = MeshAt(MeshPath);
    Result.LocalTransform = Transform;
    return Result;
}

#include "Vehicle/Generated/PinkCabTatra613SceneMeshPaths.inl"

const TCHAR* WheelPath = TEXT(
    "/Game/Dev/Vehicles/Tatra613ArchiveV12Clean/Tatra613_V12_Wheel/StaticMeshes/"
    "Tatra613_V12_Wheel.Tatra613_V12_Wheel");
const TCHAR* SourceSteeringPath = TEXT(
    "/Game/Dev/Vehicles/Tatra613DesktopScene/Tatra613_ScenePreserved/StaticMeshes/"
    "t613_Black_material_019.t613_Black_material_019");
const TCHAR* LiveSteeringPath = TEXT(
    "/Game/Dev/Vehicles/Tatra613ArchiveV12Clean/Tatra613_V12_Steering/StaticMeshes/"
    "Tatra613_V12_Steering.Tatra613_V12_Steering");
constexpr float SourcePresentationScale = 1.0f / 0.5869123935699463f;
}

FPinkCabVehicleVisualProfile FPinkCabVehicleVisualProfile::Tatra613Donor()
{
    FPinkCabVehicleVisualProfile Result;
    Result.ProfileId = TEXT("PinkCab.Visual.Tatra613.ScenePreserved");
    Result.bUseExteriorAsCabinWhenCabinMissing = false;
    Result.CockpitRootTransform = FTransform::Identity;
    Result.DriverHeadTransform = FTransform(
        FRotator::ZeroRotator, FVector(-18.0f, -40.0f, 112.0f));

    const FTransform SceneTransform(
        FRotator(0.0f, -90.0f, 0.0f),
        FVector::ZeroVector,
        FVector(SourcePresentationScale));

    for (int32 Index = 0; Index < UE_ARRAY_COUNT(GTatra613SceneMeshPaths); ++Index)
    {
        const TCHAR* MeshPath = GTatra613SceneMeshPaths[Index];
        // The source-scene steering wheel must not stay static underneath the
        // live steering mesh. t613_Black_material_019 is the exact t613_steer
        // source object identified from the donor glTF hierarchy.
        if (FCString::Strcmp(MeshPath, SourceSteeringPath) == 0)
        {
            continue;
        }
        Result.PresentationParts.Add(Part(
            FName(*FString::Printf(TEXT("Scene_%03d"), Index)),
            MeshPath,
            SceneTransform));
    }

    FPinkCabCockpitVisualBinding SteeringBinding;
    SteeringBinding.Slot = EPinkCabCockpitSlot::SteeringWheel;
    SteeringBinding.MeshOverride = MeshAt(LiveSteeringPath);
    SteeringBinding.bShowAnchorMesh = true;
    if (UStaticMesh* SourceSteering = MeshAt(SourceSteeringPath).LoadSynchronous())
    {
        const FVector SourceCenter = SourceSteering->GetBounds().Origin;
        const FVector SteeringCenter = SceneTransform.TransformPosition(SourceCenter);
        SteeringBinding.LocalTransform = FTransform(
            SceneTransform.GetRotation(),
            SteeringCenter,
            SceneTransform.GetScale3D());
    }
    Result.CockpitBindings.Add(SteeringBinding);

    constexpr float FrontX = 135.012f;
    constexpr float RearX = -162.988f;
    constexpr float HalfTrack = 76.0f;
    constexpr float WheelZ = 32.667f;
    const FVector WheelScale(1.0189712f, 1.5406767f, 1.5990789f);
    const FRotator RightWheelRot(0.0f, -90.0f, 0.0f);
    const FRotator LeftWheelRot(0.0f, -90.0f, 180.0f);

    Result.PresentationParts.Add(Part(
        TEXT("WheelFR"), WheelPath,
        FTransform(RightWheelRot, FVector(FrontX, HalfTrack, WheelZ), WheelScale)));
    Result.PresentationParts.Add(Part(
        TEXT("WheelFL"), WheelPath,
        FTransform(LeftWheelRot, FVector(FrontX, -HalfTrack, WheelZ), WheelScale)));
    Result.PresentationParts.Add(Part(
        TEXT("WheelRR"), WheelPath,
        FTransform(RightWheelRot, FVector(RearX, HalfTrack, WheelZ), WheelScale)));
    Result.PresentationParts.Add(Part(
        TEXT("WheelRL"), WheelPath,
        FTransform(LeftWheelRot, FVector(RearX, -HalfTrack, WheelZ), WheelScale)));

    return Result;
}
