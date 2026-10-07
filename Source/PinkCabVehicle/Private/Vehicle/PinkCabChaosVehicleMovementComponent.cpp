#include "Vehicle/PinkCabChaosVehicleMovementComponent.h"
#include "ChaosVehicleWheel.h"

UPinkCabChaosVehicleMovementComponent::UPinkCabChaosVehicleMovementComponent(
    const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}

bool UPinkCabChaosVehicleMovementComponent::GetWheelPresentationCenter(
    const int32 WheelIndex, FVector& OutCenter)
{
    if (!WheelSetups.IsValidIndex(WheelIndex) || !Wheels.IsValidIndex(WheelIndex) || !Wheels[WheelIndex])
    {
        return false;
    }
    // Chaos suspension offset is positive on compression, opposite to its
    // downward suspension axis. Use native rest geometry, not authored Z.
    OutCenter = GetWheelRestingPosition(WheelSetups[WheelIndex])
        - Wheels[WheelIndex]->GetSuspensionAxis() * GetSuspensionOffset(WheelIndex);
    return !OutCenter.ContainsNaN();
}
