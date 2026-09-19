#pragma once

#include "CoreMinimal.h"
#include "Interaction/PinkCabInteractionModel.h"
#include "PinkCabCockpitSlot.generated.h"

class UMaterialInterface;
class UStaticMesh;

UENUM(BlueprintType)
enum class EPinkCabCockpitSlot : uint8
{
    DriverCamera,
    SteeringWheel,
    ClutchPedal,
    BrakePedal,
    ThrottlePedal,
    Gearbox,
    Handbrake,
    Ignition,
    TurnSignals,
    Horn,
    Lights,
    Wipers,
    Washer,
    Taximeter,
    PassengerDoor,
    Dashboard,
    Warnings,
    Navigation,
    Radio,
    RearViewMirror,
    LeftMirror,
    RightMirror,
    TemperatureNeedle,
    FuelNeedle,
    SpeedometerNeedle,
    TachometerNeedle
};

PINKCAB_API FName PinkCabCockpitSlotId(EPinkCabCockpitSlot Slot);

USTRUCT(BlueprintType)
struct PINKCAB_API FPinkCabCockpitSlotDefinition
{
    GENERATED_BODY()

    FPinkCabCockpitSlotDefinition() = default;
    FPinkCabCockpitSlotDefinition(const EPinkCabCockpitSlot InSlot, const FName InStableId)
        : Slot(InSlot), StableId(InStableId) {}

    FPinkCabInteractionControlSpec ToInteractionSpec() const;
    static bool ValidateUnique(const TArray<FPinkCabCockpitSlotDefinition>& Definitions);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Cockpit")
    EPinkCabCockpitSlot Slot = EPinkCabCockpitSlot::DriverCamera;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Cockpit")
    FName StableId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Cockpit")
    FTransform LocalTransform = FTransform::Identity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Cockpit|Assets")
    TSoftObjectPtr<UStaticMesh> MeshOverride;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Cockpit|Assets")
    TSoftObjectPtr<UMaterialInterface> MaterialOverride;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Cockpit|Motion")
    FVector MotionAxis = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Cockpit|Motion")
    FVector MotionPivot = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Cockpit|Motion")
    FVector2D MotionRange = FVector2D::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Cockpit|Interaction")
    bool bSupportsGrip = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Cockpit|Interaction")
    bool bSupportsMomentary = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Cockpit|Interaction")
    bool bSupportsWheel = false;
};
