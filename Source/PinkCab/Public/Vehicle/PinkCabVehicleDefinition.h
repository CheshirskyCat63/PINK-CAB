#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Runtime/PinkCabVehicleVisualProfile.h"
#include "PinkCabVehicleDefinition.generated.h"

class UPhysicsAsset;
class USkeletalMesh;

USTRUCT(BlueprintType)
struct PINKCAB_API FPinkCabVehicleWheelBinding
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Vehicle|Definition")
    FName WheelId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Vehicle|Definition")
    FName BoneName = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Vehicle|Definition")
    FName PresentationPartId = NAME_None;

    bool IsValid() const;
};

UCLASS(BlueprintType)
class PINKCAB_API UPinkCabVehicleDefinition : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    bool IsValid(FString* OutReason = nullptr) const;
    FPinkCabVehicleVisualProfile BuildVisualProfile() const;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Vehicle|Definition")
    FName VehicleId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Vehicle|Physics")
    TSoftObjectPtr<USkeletalMesh> PhysicsCarrierMesh;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Vehicle|Physics")
    TSoftObjectPtr<UPhysicsAsset> PhysicsAsset;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Vehicle|Physics")
    TArray<FPinkCabVehicleWheelBinding> Wheels;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Vehicle|Presentation")
    FPinkCabVehicleVisualProfile VisualProfile;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Vehicle|Driver")
    TSoftObjectPtr<USkeletalMesh> DriverMesh;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Vehicle|Driver")
    FTransform DriverTransform = FTransform::Identity;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Vehicle|Driver")
    FTransform DriverHeadTransform = FTransform::Identity;
};
