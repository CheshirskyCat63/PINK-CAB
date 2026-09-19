#pragma once

#include "CoreMinimal.h"
#include "Cockpit/PinkCabCockpitSlot.h"
#include "PinkCabCockpitVisualBinding.generated.h"

class UMaterialInterface;
class UStaticMesh;

USTRUCT(BlueprintType)
struct PINKCAB_API FPinkCabCockpitVisualBinding
{
    GENERATED_BODY()

    static bool ValidateUnique(TConstArrayView<FPinkCabCockpitVisualBinding> Bindings);
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Cockpit|Visual")
    EPinkCabCockpitSlot Slot = EPinkCabCockpitSlot::SteeringWheel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Cockpit|Visual")
    FTransform LocalTransform = FTransform::Identity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Cockpit|Visual")
    TSoftObjectPtr<UStaticMesh> MeshOverride;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Cockpit|Visual")
    TSoftObjectPtr<UMaterialInterface> MaterialOverride;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Cockpit|Visual")
    bool bShowAnchorMesh = true;
};
