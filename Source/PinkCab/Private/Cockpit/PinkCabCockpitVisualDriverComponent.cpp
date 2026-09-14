#include "Cockpit/PinkCabCockpitVisualDriverComponent.h"

#include "Cockpit/PinkCabCockpitAssemblyComponent.h"
#include "Cockpit/PinkCabCockpitPresentationState.h"
#include "Cockpit/PinkCabCockpitSlot.h"
#include "Components/SceneComponent.h"

UPinkCabCockpitVisualDriverComponent::UPinkCabCockpitVisualDriverComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

float UPinkCabCockpitVisualDriverComponent::SteeringAngleDegrees(const float Steering)
{
    return FMath::Clamp(Steering, -1.0f, 1.0f) * 450.0f;
}

float UPinkCabCockpitVisualDriverComponent::PedalTravelDegrees(const float Value)
{
    return FMath::Clamp(Value, 0.0f, 1.0f) * 18.0f;
}

float UPinkCabCockpitVisualDriverComponent::HandbrakeAngleDegrees(const bool bEngaged)
{
    return bEngaged ? -32.0f : 0.0f;
}
FVector UPinkCabCockpitVisualDriverComponent::GearLeverOffset(const int32 Gear)
{
    switch (FMath::Clamp(Gear, -1, 5))
    {
    case -1: return FVector(-4.0f, -7.0f, 0.0f);
    case 1: return FVector(-5.0f, -6.0f, 0.0f);
    case 2: return FVector(-5.0f, 6.0f, 0.0f);
    case 3: return FVector(0.0f, -6.0f, 0.0f);
    case 4: return FVector(0.0f, 6.0f, 0.0f);
    case 5: return FVector(5.0f, -6.0f, 0.0f);
    default: return FVector::ZeroVector;
    }
}

void UPinkCabCockpitVisualDriverComponent::CacheBaseTransforms(
    UPinkCabCockpitAssemblyComponent& Assembly)
{
    if (BaseTransforms.Num() > 0)
    {
        return;
    }
    for (uint8 Raw = 0; Raw <= static_cast<uint8>(EPinkCabCockpitSlot::RightMirror); ++Raw)
    {
        if (USceneComponent* Component = Assembly.GetSlotComponent(static_cast<EPinkCabCockpitSlot>(Raw)))
        {
            BaseTransforms.Add(Raw, Component->GetRelativeTransform());
        }
    }
}
void UPinkCabCockpitVisualDriverComponent::Apply(
    UPinkCabCockpitAssemblyComponent& Assembly,
    const FPinkCabCockpitPresentationState& State)
{
    CacheBaseTransforms(Assembly);
    const auto GetBase = [this](const EPinkCabCockpitSlot Slot) -> const FTransform*
    {
        return BaseTransforms.Find(static_cast<uint8>(Slot));
    };
    const auto SetRotOffset = [&Assembly, &GetBase](const EPinkCabCockpitSlot Slot, const FRotator Offset)
    {
        if (USceneComponent* Component = Assembly.GetSlotComponent(Slot))
        {
            if (const FTransform* Base = GetBase(Slot))
            {
                Component->SetRelativeRotation(Base->Rotator() + Offset);
            }
        }
    };
    const auto SetLocOffset = [&Assembly, &GetBase](const EPinkCabCockpitSlot Slot, const FVector Offset)
    {
        if (USceneComponent* Component = Assembly.GetSlotComponent(Slot))
        {
            if (const FTransform* Base = GetBase(Slot))
            {
                Component->SetRelativeLocation(Base->GetLocation() + Offset);
            }
        }
    };
    SetRotOffset(EPinkCabCockpitSlot::SteeringWheel,
        FRotator(0.0f, 0.0f, SteeringAngleDegrees(State.Steering)));
    SetRotOffset(EPinkCabCockpitSlot::ClutchPedal,
        FRotator(PedalTravelDegrees(State.Clutch), 0.0f, 0.0f));
    SetRotOffset(EPinkCabCockpitSlot::BrakePedal,
        FRotator(PedalTravelDegrees(State.Brake), 0.0f, 0.0f));
    SetRotOffset(EPinkCabCockpitSlot::ThrottlePedal,
        FRotator(PedalTravelDegrees(State.Throttle), 0.0f, 0.0f));
    SetLocOffset(EPinkCabCockpitSlot::Gearbox, GearLeverOffset(State.SelectedGear));
    SetRotOffset(EPinkCabCockpitSlot::Handbrake,
        FRotator(0.0f, HandbrakeAngleDegrees(State.bHandbrakeEngaged), 0.0f));
    SetRotOffset(EPinkCabCockpitSlot::Ignition,
        FRotator(0.0f, State.bIgnitionRunning ? 42.0f : 0.0f, 0.0f));
    SetRotOffset(EPinkCabCockpitSlot::PassengerDoor,
        FRotator(0.0f, State.bPassengerDoorOpen ? 38.0f : 0.0f, 0.0f));

    if (USceneComponent* Meter = Assembly.GetSlotComponent(EPinkCabCockpitSlot::Taximeter))
    {
        Meter->SetVisibility(State.bMeterAvailable, true);
    }
    if (USceneComponent* Door = Assembly.GetSlotComponent(EPinkCabCockpitSlot::PassengerDoor))
    {
        Door->SetVisibility(State.bPassengerDoorAvailable, true);
    }
    if (USceneComponent* Navigation = Assembly.GetSlotComponent(EPinkCabCockpitSlot::Navigation))
    {
        Navigation->SetVisibility(State.bRouteAvailable, true);
    }
    if (USceneComponent* Radio = Assembly.GetSlotComponent(EPinkCabCockpitSlot::Radio))
    {
        Radio->SetVisibility(State.bRadioAvailable, true);
    }
    const EPinkCabCockpitSlot MirrorSlots[] = {
        EPinkCabCockpitSlot::RearViewMirror,
        EPinkCabCockpitSlot::LeftMirror,
        EPinkCabCockpitSlot::RightMirror};
    for (const EPinkCabCockpitSlot MirrorSlot : MirrorSlots)
    {
        if (USceneComponent* Mirror = Assembly.GetSlotComponent(MirrorSlot))
        {
            Mirror->SetVisibility(State.bMirrorsAvailable, true);
        }
    }
    if (USceneComponent* Warnings = Assembly.GetSlotComponent(EPinkCabCockpitSlot::Warnings))
    {
        Warnings->SetVisibility(State.bIgnitionRunning || State.bMeterRunning || State.bPassengerDoorOpen, true);
    }
}
