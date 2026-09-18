#pragma once

#include "CoreMinimal.h"
#include "Cockpit/PinkCabCockpitVisualBinding.h"
#include "PinkCabVehicleVisualProfile.generated.h"

class UStaticMesh;
class USkeletalMesh;

USTRUCT(BlueprintType)
struct PINKCAB_API FPinkCabVehiclePresentationPart
{
    GENERATED_BODY()

    bool IsValid() const
    {
        return !PartId.IsNone() && !Mesh.IsNull() && !LocalTransform.ContainsNaN()
            && !(bOwnerNoSee && bOnlyOwnerSee);
    }

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Vehicle|Visual")
    FName PartId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Vehicle|Visual")
    TSoftObjectPtr<UStaticMesh> Mesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Vehicle|Visual")
    FTransform LocalTransform = FTransform::Identity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Vehicle|Visual")
    bool bOwnerNoSee = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Vehicle|Visual")
    bool bOnlyOwnerSee = false;
};

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

    static FPinkCabVehicleVisualProfile Tatra613Donor();

    bool IsValid() const
    {
        if (ProfileId.IsNone() || !FPinkCabCockpitVisualBinding::ValidateUnique(CockpitBindings)) return false;
        TSet<FName> Seen;
        for (const FPinkCabVehiclePresentationPart& Part : PresentationParts)
        {
            if (!Part.IsValid() || Seen.Contains(Part.PartId)) return false;
            Seen.Add(Part.PartId);
        }
        return true;
    }

    bool HasExteriorAsset() const { return !ExteriorStaticMesh.IsNull() || !ExteriorSkeletalMesh.IsNull(); }
    bool HasVisualAsset() const { return HasExteriorAsset() || PresentationParts.Num() > 0; }
    bool HasCabinAsset() const
    {
        return !CabinStaticMesh.IsNull() || !CabinSkeletalMesh.IsNull()
            || (bUseExteriorAsCabinWhenCabinMissing && HasExteriorAsset());
    }

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
    bool bUseExteriorAsCabinWhenCabinMissing = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Vehicle|Visual")
    FTransform ExteriorTransform = FTransform::Identity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Vehicle|Visual")
    FTransform CabinTransform = FTransform::Identity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Vehicle|Visual")
    TArray<FPinkCabVehiclePresentationPart> PresentationParts;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Vehicle|Visual")
    TArray<FPinkCabCockpitVisualBinding> CockpitBindings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Vehicle|Visual")
    FTransform CockpitRootTransform = FTransform::Identity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Vehicle|Visual")
    FTransform DriverHeadTransform = FTransform(FRotator::ZeroRotator, FVector(-15.0f, -38.0f, 128.0f));

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Vehicle|Visual")
    bool bHidePhysicsChassisWhenExteriorPresent = true;
};
