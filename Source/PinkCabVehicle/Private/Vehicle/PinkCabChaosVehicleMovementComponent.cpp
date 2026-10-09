#include "Vehicle/PinkCabChaosVehicleMovementComponent.h"
#include "ChaosVehicleWheel.h"

UPinkCabChaosVehicleMovementComponent::UPinkCabChaosVehicleMovementComponent(
    const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    // A manual car does not apply service brakes just because throttle/speed is low.
    // Engine braking stays in the native mechanical driveline, not this brake input.
    IdleBrakeInput = 0.0f;
    StopThreshold = 0.0f;
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

void UPinkCabChaosVehicleMovementComponent::SetPinkCabHandbrakeInput(float Value)
{
    AnalogHandbrakeCommand = FMath::IsFinite(Value) ? FMath::Clamp(Value, 0.0f, 1.0f) : 0.0f;
    SetHandbrakeInput(false);
}

void UPinkCabChaosVehicleMovementComponent::UpdateState(float DeltaTime)
{
    Super::UpdateState(DeltaTime);
    // This is the native game-thread input preparation hook. Chaos carries this
    // float in its normal async control packet and applies its own rear braking.
    HandbrakeInput = (!bRequiresControllerForInputs || GetController())
        ? AnalogHandbrakeCommand : 0.0f;
}

void UPinkCabChaosVehicleMovementComponent::ClearRawInput()
{
    Super::ClearRawInput();
    AnalogHandbrakeCommand = 0.0f;
}
