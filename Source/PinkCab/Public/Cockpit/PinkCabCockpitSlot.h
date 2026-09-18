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

inline FName PinkCabCockpitSlotId(const EPinkCabCockpitSlot Slot)
{
    switch (Slot)
    {
    case EPinkCabCockpitSlot::DriverCamera: return TEXT("DriverCamera");
    case EPinkCabCockpitSlot::SteeringWheel: return TEXT("SteeringWheel");
    case EPinkCabCockpitSlot::ClutchPedal: return TEXT("ClutchPedal");
    case EPinkCabCockpitSlot::BrakePedal: return TEXT("BrakePedal");
    case EPinkCabCockpitSlot::ThrottlePedal: return TEXT("ThrottlePedal");
    case EPinkCabCockpitSlot::Gearbox: return TEXT("Gearbox");
    case EPinkCabCockpitSlot::Handbrake: return TEXT("Handbrake");
    case EPinkCabCockpitSlot::Ignition: return TEXT("Ignition");
    case EPinkCabCockpitSlot::TurnSignals: return TEXT("TurnSignals");
    case EPinkCabCockpitSlot::Horn: return TEXT("Horn");
    case EPinkCabCockpitSlot::Lights: return TEXT("Lights");
    case EPinkCabCockpitSlot::Wipers: return TEXT("Wipers");
    case EPinkCabCockpitSlot::Washer: return TEXT("Washer");
    case EPinkCabCockpitSlot::Taximeter: return TEXT("Taximeter");
    case EPinkCabCockpitSlot::PassengerDoor: return TEXT("PassengerDoor");
    case EPinkCabCockpitSlot::Dashboard: return TEXT("Dashboard");
    case EPinkCabCockpitSlot::Warnings: return TEXT("Warnings");
    case EPinkCabCockpitSlot::Navigation: return TEXT("Navigation");
    case EPinkCabCockpitSlot::Radio: return TEXT("Radio");
    case EPinkCabCockpitSlot::RearViewMirror: return TEXT("RearViewMirror");
    case EPinkCabCockpitSlot::LeftMirror: return TEXT("LeftMirror");
    case EPinkCabCockpitSlot::RightMirror: return TEXT("RightMirror");
    case EPinkCabCockpitSlot::TemperatureNeedle: return TEXT("TemperatureNeedle");
    case EPinkCabCockpitSlot::FuelNeedle: return TEXT("FuelNeedle");
    case EPinkCabCockpitSlot::SpeedometerNeedle: return TEXT("SpeedometerNeedle");
    case EPinkCabCockpitSlot::TachometerNeedle: return TEXT("TachometerNeedle");
    default: return NAME_None;
    }
}

USTRUCT(BlueprintType)
struct PINKCAB_API FPinkCabCockpitSlotDefinition
{
    GENERATED_BODY()

    FPinkCabCockpitSlotDefinition() = default;
    FPinkCabCockpitSlotDefinition(const EPinkCabCockpitSlot InSlot, const FName InStableId)
        : Slot(InSlot), StableId(InStableId) {}

    FPinkCabInteractionControlSpec ToInteractionSpec() const
    {
        return {StableId, bSupportsGrip, bSupportsMomentary, bSupportsWheel};
    }

    static bool ValidateUnique(const TArray<FPinkCabCockpitSlotDefinition>& Definitions)
    {
        TSet<FName> SeenIds;
        TSet<uint8> SeenSlots;
        for (const FPinkCabCockpitSlotDefinition& Definition : Definitions)
        {
            const uint8 RawSlot = static_cast<uint8>(Definition.Slot);
            if (Definition.StableId.IsNone() || SeenIds.Contains(Definition.StableId) || SeenSlots.Contains(RawSlot))
            {
                return false;
            }
            SeenIds.Add(Definition.StableId);
            SeenSlots.Add(RawSlot);
        }
        return true;
    }

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
