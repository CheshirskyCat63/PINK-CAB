#pragma once

#include "CoreMinimal.h"

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
    RightMirror
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
    default: return NAME_None;
    }
}

struct FPinkCabCockpitSlotDefinition
{
    FPinkCabCockpitSlotDefinition() = default;
    FPinkCabCockpitSlotDefinition(EPinkCabCockpitSlot InSlot, FName InStableId)
        : Slot(InSlot), StableId(InStableId) {}

    static bool ValidateUnique(const TArray<FPinkCabCockpitSlotDefinition>& Definitions)
    {
        TSet<FName> SeenIds;
        for (const FPinkCabCockpitSlotDefinition& Definition : Definitions)
        {
            if (Definition.StableId.IsNone() || SeenIds.Contains(Definition.StableId))
            {
                return false;
            }
            SeenIds.Add(Definition.StableId);
        }
        return true;
    }

    EPinkCabCockpitSlot Slot = EPinkCabCockpitSlot::DriverCamera;
    FName StableId = NAME_None;
};
