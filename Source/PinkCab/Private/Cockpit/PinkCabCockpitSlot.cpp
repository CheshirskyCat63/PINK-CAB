#include "Cockpit/PinkCabCockpitSlot.h"

FName PinkCabCockpitSlotId(const EPinkCabCockpitSlot Slot)
{
    static const FName SlotIds[] = {
        TEXT("DriverCamera"), TEXT("SteeringWheel"), TEXT("ClutchPedal"), TEXT("BrakePedal"),
        TEXT("ThrottlePedal"), TEXT("Gearbox"), TEXT("Handbrake"), TEXT("Ignition"),
        TEXT("TurnSignals"), TEXT("Horn"), TEXT("Lights"), TEXT("Wipers"), TEXT("Washer"),
        TEXT("Taximeter"), TEXT("PassengerDoor"), TEXT("Dashboard"), TEXT("Warnings"),
        TEXT("Navigation"), TEXT("Radio"), TEXT("RearViewMirror"), TEXT("LeftMirror"),
        TEXT("RightMirror"), TEXT("TemperatureNeedle"), TEXT("FuelNeedle"),
        TEXT("SpeedometerNeedle"), TEXT("TachometerNeedle")
    };
    const uint8 Raw = static_cast<uint8>(Slot);
    return Raw < UE_ARRAY_COUNT(SlotIds) ? SlotIds[Raw] : NAME_None;
}

FPinkCabInteractionControlSpec FPinkCabCockpitSlotDefinition::ToInteractionSpec() const
{
    return {StableId, bSupportsGrip, bSupportsMomentary, bSupportsWheel};
}

bool FPinkCabCockpitSlotDefinition::ValidateUnique(
    const TArray<FPinkCabCockpitSlotDefinition>& Definitions)
{
    TSet<FName> SeenIds;
    TSet<uint8> SeenSlots;
    for (const FPinkCabCockpitSlotDefinition& Definition : Definitions)
    {
        const uint8 RawSlot = static_cast<uint8>(Definition.Slot);
        if (Definition.StableId.IsNone() || SeenIds.Contains(Definition.StableId)
            || SeenSlots.Contains(RawSlot))
        {
            return false;
        }
        SeenIds.Add(Definition.StableId);
        SeenSlots.Add(RawSlot);
    }
    return true;
}
