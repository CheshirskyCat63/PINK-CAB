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
    "t613_Black_material_021.t613_Black_material_021");
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

    // source_scene_report.json: t613_steer -> t613_Black_material.021
    // Object origin is the real steering pivot. Rotation -75 deg about source X
    // makes the steering-column normal point 15 deg upward.
    const FVector SourceSteeringPivotCm(27.012235f, -23.020297f, 49.803603f);
    const FVector SourceSteeringAxis(0.0f, 0.9659258f, 0.2588190f);
    Result.SteeringPresentationPivot = SceneTransform.TransformPosition(SourceSteeringPivotCm);
    Result.SteeringPresentationAxis =
        SceneTransform.TransformVectorNoScale(SourceSteeringAxis).GetSafeNormal();

    for (int32 Index = 0; Index < UE_ARRAY_COUNT(GTatra613SceneMeshPaths); ++Index)
    {
        const TCHAR* MeshPath = GTatra613SceneMeshPaths[Index];
        const FName PartId(*FString::Printf(TEXT("Scene_%03d"), Index));
        Result.PresentationParts.Add(Part(
            PartId,
            MeshPath,
            SceneTransform));
        if (FCString::Strcmp(MeshPath, SourceSteeringPath) == 0)
        {
            Result.SteeringPresentationPartId = PartId;
        }
    }

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
