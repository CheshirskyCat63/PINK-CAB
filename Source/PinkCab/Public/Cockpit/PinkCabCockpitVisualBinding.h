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

    static bool ValidateUnique(TConstArrayView<FPinkCabCockpitVisualBinding> Bindings)
    {
        TSet<uint8> Seen;
        for (const FPinkCabCockpitVisualBinding& Binding : Bindings)
        {
            const uint8 Raw = static_cast<uint8>(Binding.Slot);
            if (Binding.Slot == EPinkCabCockpitSlot::DriverCamera || Seen.Contains(Raw)
                || Binding.LocalTransform.ContainsNaN())
            {
                return false;
            }
            Seen.Add(Raw);
        }
        return true;
    }
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
