#include "Vehicle/PinkCabVehicleVisualProfile.h"

#include "Engine/StaticMesh.h"

namespace
{
TSoftObjectPtr<UStaticMesh> MeshAt(const TCHAR* Path)
{
    return TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(Path));
}

FPinkCabVehiclePresentationPart Part(
    const TCHAR* Id,
    const TCHAR* MeshPath,
    const FTransform& Transform = FTransform::Identity)
{
    FPinkCabVehiclePresentationPart Result;
    Result.PartId = FName(Id);
    Result.Mesh = MeshAt(MeshPath);
    Result.LocalTransform = Transform;
    return Result;
}

const TCHAR* MasterRoot = TEXT("/Game/Dev/Vehicles/Tatra613Donor/MasterUE2/Tatra613_MASTER_UE2/StaticMeshes/");
const TCHAR* WheelPivotPath = TEXT("/Game/Dev/Vehicles/Tatra613Donor/PivotParts/Wheel/PC_Tatra613_WheelPivot/StaticMeshes/PC_Tatra613_WheelPivot.PC_Tatra613_WheelPivot");
const TCHAR* SteeringPivotPath = TEXT("/Game/Dev/Vehicles/Tatra613Donor/PivotParts/Steering/PC_Tatra613_SteeringPivot/StaticMeshes/PC_Tatra613_SteeringPivot.PC_Tatra613_SteeringPivot");
}

FPinkCabVehicleVisualProfile FPinkCabVehicleVisualProfile::Tatra613Donor()
{
    FPinkCabVehicleVisualProfile Result;
    Result.ProfileId = TEXT("PinkCab.Visual.Tatra613.Donor");
    Result.ExteriorStaticMesh = MeshAt(
        TEXT("/Game/Dev/Vehicles/Tatra613Donor/MasterUE2/Tatra613_MASTER_UE2/StaticMeshes/PC_Tatra613_Main.PC_Tatra613_Main"));
    Result.bUseExteriorAsCabinWhenCabinMissing = true;
    Result.ExteriorTransform = FTransform(FQuat::Identity, FVector::ZeroVector, FVector(1.0f, -1.0f, 1.0f));
    Result.CockpitRootTransform = FTransform::Identity;
    Result.DriverHeadTransform = FTransform(FRotator::ZeroRotator, FVector(-15.0f, -40.0f, 126.0f));

    const FTransform MirrorY(FQuat::Identity, FVector::ZeroVector, FVector(1.0f, -1.0f, 1.0f));
    Result.PresentationParts.Add(Part(TEXT("Underbody"),
        *FString(MasterRoot).Append(TEXT("PC_Tatra613_Underbody.PC_Tatra613_Underbody")), MirrorY));
    Result.PresentationParts.Add(Part(TEXT("GaugeFace"),
        *FString(MasterRoot).Append(TEXT("PC_Tatra613_GaugeFace.PC_Tatra613_GaugeFace")), MirrorY));
    Result.PresentationParts.Add(Part(TEXT("NeedleFuel"),
        *FString(MasterRoot).Append(TEXT("PC_Tatra613_NeedleFuel.PC_Tatra613_NeedleFuel")), MirrorY));
    Result.PresentationParts.Add(Part(TEXT("NeedleSpeedo"),
        *FString(MasterRoot).Append(TEXT("PC_Tatra613_NeedleSpeedo.PC_Tatra613_NeedleSpeedo")), MirrorY));
    Result.PresentationParts.Add(Part(TEXT("NeedleTacho"),
        *FString(MasterRoot).Append(TEXT("PC_Tatra613_NeedleTacho.PC_Tatra613_NeedleTacho")), MirrorY));
    Result.PresentationParts.Add(Part(TEXT("NeedleTemp"),
        *FString(MasterRoot).Append(TEXT("PC_Tatra613_NeedleTemp.PC_Tatra613_NeedleTemp")), MirrorY));

    constexpr float FrontX = -135.012f;
    constexpr float RearX = 162.988f;
    constexpr float HalfTrack = 81.135f;
    constexpr float WheelZ = 32.667f;
    const FRotator LeftWheelFlip(0.0f, 0.0f, 180.0f);
    Result.PresentationParts.Add(Part(TEXT("WheelFR"), WheelPivotPath,
        FTransform(FRotator::ZeroRotator, FVector(FrontX, HalfTrack, WheelZ))));
    Result.PresentationParts.Add(Part(TEXT("WheelFL"), WheelPivotPath,
        FTransform(LeftWheelFlip, FVector(FrontX, -HalfTrack, WheelZ))));
    Result.PresentationParts.Add(Part(TEXT("WheelRR"), WheelPivotPath,
        FTransform(FRotator::ZeroRotator, FVector(RearX, HalfTrack, WheelZ))));
    Result.PresentationParts.Add(Part(TEXT("WheelRL"), WheelPivotPath,
        FTransform(LeftWheelFlip, FVector(RearX, -HalfTrack, WheelZ))));

    FPinkCabCockpitVisualBinding Steering;
    Steering.Slot = EPinkCabCockpitSlot::SteeringWheel;
    Steering.LocalTransform = FTransform(
        FRotator::ZeroRotator, FVector(-48.456f, -44.466f, 82.649f));
    Steering.MeshOverride = MeshAt(SteeringPivotPath);
    Steering.bShowAnchorMesh = true;
    Result.CockpitBindings.Add(Steering);
    return Result;
}
