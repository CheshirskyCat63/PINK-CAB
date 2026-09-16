#pragma once

#include "CoreMinimal.h"
#include "PinkCabVehicleVisualProfile.generated.h"

class UStaticMesh;
class USkeletalMesh;

USTRUCT(BlueprintType)
struct PINKCAB_API FPinkCabVehicleVisualProfile
{
    GENERATED_BODY()

    static FPinkCabVehicleVisualProfile Fallback()
    {
        FPinkCabVehicleVisualProfile Result;
        Result.ProfileId = TEXT("PinkCab.Visual.Fallback");
        return Result;
    }

    bool IsValid() const { return !ProfileId.IsNone(); }
    bool HasExteriorAsset() const { return !ExteriorStaticMesh.IsNull() || !ExteriorSkeletalMesh.IsNull(); }
    bool HasCabinAsset() const { return !CabinStaticMesh.IsNull() || !CabinSkeletalMesh.IsNull(); }

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Vehicle|Visual")
    FName ProfileId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Vehicle|Visual")
    TSoftObjectPtr<UStaticMesh> ExteriorStaticMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Vehicle|Visual")
    TSoftObjectPtr<USkeletalMesh> ExteriorSkeletalMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Vehicle|Visual")
    TSoftObjectPtr<UStaticMesh> CabinStaticMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Vehicle|Visual")
    TSoftObjectPtr<USkeletalMesh> CabinSkeletalMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Vehicle|Visual")
    FTransform ExteriorTransform = FTransform::Identity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Vehicle|Visual")
    FTransform CabinTransform = FTransform::Identity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Vehicle|Visual")
    FTransform CockpitRootTransform = FTransform::Identity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Vehicle|Visual")
    FTransform DriverHeadTransform = FTransform(FRotator::ZeroRotator, FVector(-15.0f, -38.0f, 128.0f));

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Vehicle|Visual")
    bool bHidePhysicsChassisWhenExteriorPresent = true;
};
