#include "Runtime/PinkCabChaosTatraPawn.h"
#include "Runtime/PinkCabDriverUiComponent.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "Cockpit/PinkCabCockpitInteractionComponent.h"
#include "Cockpit/PinkCabCockpitVisualDriverComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/PlayerController.h"
#include "Interaction/PinkCabInteractionModel.h"
#include "Vehicle/PinkCabChaosCockpitBridge.h"
#include "Vehicle/PinkCabCockpitInteractionRouter.h"

void APinkCabChaosTatraPawn::SetSystemMenuOpen(const bool bOpen)
{
    ResetTransientCockpitInput();
    if (DriverUi)
    {
        if (APlayerController* PC = Cast<APlayerController>(GetController()))
        {
            DriverUi->SetSystemMenuOpen(*PC, bOpen);
        }
    }
}

bool APinkCabChaosTatraPawn::IsSystemMenuOpen() const
{
    return DriverUi && DriverUi->IsSystemMenuOpen();
}

void APinkCabChaosTatraPawn::ResetTransientCockpitInput()
{
    if (DriverUi)
    {
        if (APlayerController* PC = Cast<APlayerController>(GetController()))
        {
            DriverUi->ResetTransientInput(*PC);
        }
    }
    VehicleControlRuntime.ResetThrottleInputLatch();
    PendingMouseWheelSteps = 0;
    bGearLeverDragging = false;
    GearLeverCursor = UPinkCabCockpitVisualDriverComponent::GearCursorForGear(
        CockpitState.GetSelectedGear());
    if (!CockpitInteraction)
    {
        return;
    }
    TArray<FPinkCabInteractionEvent> ReleaseEvents;
    CockpitInteraction->ResetTransientInputState(&ReleaseEvents);
    for (const FPinkCabInteractionEvent& ReleaseEvent : ReleaseEvents)
    {
        ApplyCockpitInteraction(ReleaseEvent);
    }
    const bool bControlsChanged = VehicleControlRuntime.ResetTransient(CockpitState);
    GearLeverCursor = UPinkCabCockpitVisualDriverComponent::GearCursorForGear(
        CockpitState.GetSelectedGear());
    if (bControlsChanged)
    {
        SyncCockpitToChaos();
    }
}

bool APinkCabChaosTatraPawn::ApplyCockpitInteraction(const FPinkCabInteractionEvent& Event)
{
    if (!FPinkCabCockpitInteractionRouter::Apply(Event, CockpitState))
    {
        return false;
    }
    SyncCockpitToChaos();
    return true;
}

void APinkCabChaosTatraPawn::SyncCockpitToChaos()
{
    if (UChaosWheeledVehicleMovementComponent* Movement = GetChaosMovement())
    {
        VehicleControlRuntime.ApplyHealthCapabilities(
            VehicleHealthService, GetVehicleHealthState());

        const FPinkCabVehicleControlState& Controls =
            VehicleControlRuntime.GetControlState();
        const bool bDriverRequestsDriveWake =
            CockpitState.GetIgnitionState() == EPinkCabIgnitionState::Running
            && Controls.Throttle > KINDA_SMALL_NUMBER;
        if (bDriverRequestsDriveWake)
        {
            if (USkeletalMeshComponent* VehicleMesh = GetMesh())
            {
                VehicleMesh->WakeAllRigidBodies();
            }
        }

        FPinkCabChaosCockpitBridge::Apply(
            CockpitState,
            *Movement,
            VehicleControlRuntime.GetMutableControlState(),
            DynamicsProvider);
        if (!VehicleHealthService.HasCapability(
                GetVehicleHealthState(), EPinkCabVehicleCapability::RunEngine))
        {
            Movement->EnableMechanicalSim(false);
        }
    }
}
