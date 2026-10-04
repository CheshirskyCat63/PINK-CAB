#include "Runtime/PinkCabVehicleArticulationComponent.h"

#include "Runtime/PinkCabVehicleVisualProfile.h"
#include "Runtime/PinkCabVehicleVisualShellComponent.h"

UPinkCabVehicleArticulationComponent::UPinkCabVehicleArticulationComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    PrimaryComponentTick.TickGroup = TG_PostPhysics;
}

void UPinkCabVehicleArticulationComponent::BeginPlay()
{
    Super::BeginPlay();
    RefreshStateInventory();
}

void UPinkCabVehicleArticulationComponent::SetVisualShell(
    UPinkCabVehicleVisualShellComponent* InVisualShell)
{
    VisualShell = InVisualShell;
    RefreshStateInventory();
}

void UPinkCabVehicleArticulationComponent::RefreshStateInventory()
{
    if (!VisualShell)
    {
        return;
    }

    TSet<FName> ActiveIds;
    for (const FPinkCabVehicleArticulationDefinition& Definition
        : VisualShell->GetProfile().Articulations)
    {
        ActiveIds.Add(Definition.ArticulationId);
        States.FindOrAdd(Definition.ArticulationId);
    }

    for (auto It = States.CreateIterator(); It; ++It)
    {
        if (!ActiveIds.Contains(It.Key()))
        {
            It.RemoveCurrent();
        }
    }
}

bool UPinkCabVehicleArticulationComponent::SetPanelOpen(
    const FName ArticulationId,
    const bool bOpen)
{
    if (FPinkCabVehicleArticulationState* State = States.Find(ArticulationId))
    {
        return State->SetTarget(bOpen ? 1.0f : 0.0f);
    }
    return false;
}

bool UPinkCabVehicleArticulationComponent::TogglePanel(const FName ArticulationId)
{
    if (FPinkCabVehicleArticulationState* State = States.Find(ArticulationId))
    {
        State->Toggle();
        return true;
    }
    return false;
}

float UPinkCabVehicleArticulationComponent::GetPanelFraction(const FName ArticulationId) const
{
    if (const FPinkCabVehicleArticulationState* State = States.Find(ArticulationId))
    {
        return State->Current;
    }
    return 0.0f;
}

void UPinkCabVehicleArticulationComponent::CloseAll()
{
    for (TPair<FName, FPinkCabVehicleArticulationState>& Pair : States)
    {
        Pair.Value.SetTarget(0.0f);
    }
}

void UPinkCabVehicleArticulationComponent::TickComponent(
    const float DeltaTime,
    const ELevelTick TickType,
    FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!VisualShell)
    {
        return;
    }

    for (const FPinkCabVehicleArticulationDefinition& Definition
        : VisualShell->GetProfile().Articulations)
    {
        FPinkCabVehicleArticulationState& State =
            States.FindOrAdd(Definition.ArticulationId);

        if (!State.Advance(DeltaTime, Definition.TravelSeconds))
        {
            continue;
        }
        VisualShell->SetArticulationFraction(
            Definition.ArticulationId,
            State.Current);
    }
}
