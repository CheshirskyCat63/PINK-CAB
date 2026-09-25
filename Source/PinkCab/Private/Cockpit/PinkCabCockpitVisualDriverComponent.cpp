#include "Cockpit/PinkCabCockpitVisualDriverComponent.h"

#include "Cockpit/PinkCabCockpitAssemblyComponent.h"
#include "Cockpit/PinkCabCockpitPresentationState.h"
#include "Cockpit/PinkCabCockpitSlot.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"

UPinkCabCockpitVisualDriverComponent::UPinkCabCockpitVisualDriverComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

float UPinkCabCockpitVisualDriverComponent::SteeringAngleDegrees(const float Steering)
{
    // Semantic steering stays +right. The preserved Tatra wheel mesh rotates
    // rightward around its authored column on the negative local angle.
    return FMath::Clamp(Steering, -1.0f, 1.0f) * -450.0f;
}

float UPinkCabCockpitVisualDriverComponent::PedalTravelDegrees(const float Value)
{
    return FMath::Clamp(Value, 0.0f, 1.0f) * 18.0f;
}

float UPinkCabCockpitVisualDriverComponent::HandbrakeAngleDegrees(const float Amount)
{
    return FMath::Clamp(Amount, 0.0f, 1.0f) * -20.0f;
}

float UPinkCabCockpitVisualDriverComponent::TemperatureNeedleAngleDegrees(const float Temperature01)
{
    return FMath::Lerp(-60.0f, 60.0f, FMath::Clamp(Temperature01, 0.0f, 1.0f));
}

float UPinkCabCockpitVisualDriverComponent::FuelNeedleAngleDegrees(const float Fuel01)
{
    return FMath::Lerp(-60.0f, 60.0f, FMath::Clamp(Fuel01, 0.0f, 1.0f));
}

float UPinkCabCockpitVisualDriverComponent::SpeedometerNeedleAngleDegrees(const float SpeedKmh)
{
    return FMath::Lerp(-125.0f, 125.0f, FMath::Clamp(FMath::Abs(SpeedKmh) / 220.0f, 0.0f, 1.0f));
}

float UPinkCabCockpitVisualDriverComponent::TachometerNeedleAngleDegrees(const float EngineRpm)
{
    return FMath::Lerp(-125.0f, 125.0f, FMath::Clamp(EngineRpm / 7000.0f, 0.0f, 1.0f));
}

FVector UPinkCabCockpitVisualDriverComponent::PivotCompensatedLocation(
    const FTransform& BaseTransform,
    const FVector& MeshLocalCenter,
    const FRotator& RotationOffset)
{
    const FVector FixedCenter = BaseTransform.TransformPosition(MeshLocalCenter);
    const FVector ScaledCenter = MeshLocalCenter * BaseTransform.GetScale3D();
    const FQuat NewRotation = (BaseTransform.Rotator() + RotationOffset).Quaternion();
    return FixedCenter - NewRotation.RotateVector(ScaledCenter);
}
FVector2D UPinkCabCockpitVisualDriverComponent::GearCursorForGear(const int32 Gear)
{
    // Use the same extended cross-gate coordinate system as the runtime H-gate:
    // 1/2=-1, 3/4=+1 (the old 5/R location), 5/R=+2.
    switch (FMath::Clamp(Gear, -1, 5))
    {
    case -1: return FVector2D(2.0f, -1.0f);
    case 1: return FVector2D(-1.0f, 1.0f);
    case 2: return FVector2D(-1.0f, -1.0f);
    case 3: return FVector2D(1.0f, 1.0f);
    case 4: return FVector2D(1.0f, -1.0f);
    case 5: return FVector2D(2.0f, 1.0f);
    default: return FVector2D(1.0f, 0.0f);
    }
}

FVector UPinkCabCockpitVisualDriverComponent::GearLeverOffsetFromCursor(FVector2D Cursor)
{
    Cursor.X = FMath::Clamp(Cursor.X, -1.0f, 2.0f);
    Cursor.Y = FMath::Clamp(Cursor.Y, -1.0f, 1.0f);
    // The old 5/R visual position was +5.25 cm. That exact position is now
    // the 3/4 rail, and 5/R sits one equal 5.25 cm step farther right.
    return FVector(Cursor.X * 5.25f, -Cursor.Y * 6.0f, 0.0f);
}

FVector UPinkCabCockpitVisualDriverComponent::GearLeverOffset(const int32 Gear)
{
    return GearLeverOffsetFromCursor(GearCursorForGear(Gear));
}

void UPinkCabCockpitVisualDriverComponent::SetSteeringVisualComponent(USceneComponent* Component)
{
    SteeringVisualComponent = Component;
    bSteeringVisualBaseValid = false;
}

void UPinkCabCockpitVisualDriverComponent::CacheBaseTransforms(
    UPinkCabCockpitAssemblyComponent& Assembly)
{
    if (BaseTransforms.Num() > 0)
    {
        return;
    }
    for (uint8 Raw = 0; Raw <= static_cast<uint8>(EPinkCabCockpitSlot::TachometerNeedle); ++Raw)
    {
        if (USceneComponent* Component = Assembly.GetSlotComponent(static_cast<EPinkCabCockpitSlot>(Raw)))
        {
            BaseTransforms.Add(Raw, Component->GetRelativeTransform());
        }
    }
}
const FTransform* UPinkCabCockpitVisualDriverComponent::GetBaseTransform(
    const EPinkCabCockpitSlot Slot) const
{
    return BaseTransforms.Find(static_cast<uint8>(Slot));
}

void UPinkCabCockpitVisualDriverComponent::ApplyRotationOffset(
    UPinkCabCockpitAssemblyComponent& Assembly,
    const EPinkCabCockpitSlot Slot,
    const FRotator& Offset) const
{
    USceneComponent* Component = Assembly.GetSlotComponent(Slot);
    const FTransform* Base = GetBaseTransform(Slot);
    if (Component && Base) Component->SetRelativeRotation(Base->Rotator() + Offset);
}

void UPinkCabCockpitVisualDriverComponent::ApplyLocationOffset(
    UPinkCabCockpitAssemblyComponent& Assembly,
    const EPinkCabCockpitSlot Slot,
    const FVector& Offset) const
{
    USceneComponent* Component = Assembly.GetSlotComponent(Slot);
    const FTransform* Base = GetBaseTransform(Slot);
    if (Component && Base) Component->SetRelativeLocation(Base->GetLocation() + Offset);
}

void UPinkCabCockpitVisualDriverComponent::ApplySteeringState(
    UPinkCabCockpitAssemblyComponent& Assembly,
    const FPinkCabCockpitPresentationState& State)
{
    if (USceneComponent* Steering = SteeringVisualComponent.Get())
    {
        if (!bSteeringVisualBaseValid)
        {
            SteeringVisualBaseTransform = Steering->GetRelativeTransform();
            bSteeringVisualBaseValid = true;
        }
        const float AngleRadians = FMath::DegreesToRadians(SteeringAngleDegrees(State.Steering));
        const FQuat LocalTurn(FVector::ForwardVector, AngleRadians);
        Steering->SetRelativeLocation(SteeringVisualBaseTransform.GetLocation());
        Steering->SetRelativeRotation(SteeringVisualBaseTransform.GetRotation() * LocalTurn);
        return;
    }

    USceneComponent* FallbackSteering =
        Assembly.GetSlotComponent(EPinkCabCockpitSlot::SteeringWheel);
    const FTransform* Base = GetBaseTransform(EPinkCabCockpitSlot::SteeringWheel);
    if (!FallbackSteering || !Base) return;
    const FRotator Offset(0.0f, 0.0f, SteeringAngleDegrees(State.Steering));
    FallbackSteering->SetRelativeLocationAndRotation(
        Base->GetLocation(), Base->Rotator() + Offset);
}

void UPinkCabCockpitVisualDriverComponent::ApplyControlMotion(
    UPinkCabCockpitAssemblyComponent& Assembly,
    const FPinkCabCockpitPresentationState& State) const
{
    ApplyRotationOffset(Assembly, EPinkCabCockpitSlot::ClutchPedal,
        FRotator(PedalTravelDegrees(State.Clutch), 0.0f, 0.0f));
    ApplyRotationOffset(Assembly, EPinkCabCockpitSlot::BrakePedal,
        FRotator(PedalTravelDegrees(State.Brake), 0.0f, 0.0f));
    ApplyRotationOffset(Assembly, EPinkCabCockpitSlot::ThrottlePedal,
        FRotator(PedalTravelDegrees(State.Throttle), 0.0f, 0.0f));

    const FVector GearOffset = State.bGearLeverDragging
        ? GearLeverOffsetFromCursor(State.GearLeverCursor)
        : GearLeverOffset(State.SelectedGear);
    ApplyLocationOffset(Assembly, EPinkCabCockpitSlot::Gearbox, GearOffset);
    ApplyRotationOffset(Assembly, EPinkCabCockpitSlot::Handbrake,
        FRotator(HandbrakeAngleDegrees(State.Handbrake), 0.0f, 0.0f));
    ApplyRotationOffset(Assembly, EPinkCabCockpitSlot::TemperatureNeedle,
        FRotator(0.0f, 0.0f, TemperatureNeedleAngleDegrees(State.EngineTemperature01)));
    ApplyRotationOffset(Assembly, EPinkCabCockpitSlot::FuelNeedle,
        FRotator(0.0f, 0.0f, FuelNeedleAngleDegrees(State.Fuel01)));
    ApplyRotationOffset(Assembly, EPinkCabCockpitSlot::SpeedometerNeedle,
        FRotator(0.0f, 0.0f, SpeedometerNeedleAngleDegrees(State.SpeedKmh)));
    ApplyRotationOffset(Assembly, EPinkCabCockpitSlot::TachometerNeedle,
        FRotator(0.0f, 0.0f, TachometerNeedleAngleDegrees(State.EngineRpm)));
    ApplyRotationOffset(Assembly, EPinkCabCockpitSlot::Ignition,
        FRotator(0.0f, State.bIgnitionRunning ? 42.0f : 0.0f, 0.0f));

    const float SignalAngle = State.bTurnSignalLeft ? -24.0f : (State.bTurnSignalRight ? 24.0f : 0.0f);
    ApplyRotationOffset(Assembly, EPinkCabCockpitSlot::TurnSignals,
        FRotator(0.0f, SignalAngle, 0.0f));
    ApplyLocationOffset(Assembly, EPinkCabCockpitSlot::Horn,
        FVector(State.bHornActive ? -1.5f : 0.0f, 0.0f, 0.0f));
    ApplyRotationOffset(Assembly, EPinkCabCockpitSlot::Lights,
        FRotator(State.bLightsOn ? 32.0f : 0.0f, 0.0f, 0.0f));
    ApplyRotationOffset(Assembly, EPinkCabCockpitSlot::Wipers,
        FRotator(State.bWipersOn ? 32.0f : 0.0f, 0.0f, 0.0f));
    ApplyLocationOffset(Assembly, EPinkCabCockpitSlot::Washer,
        FVector(State.bWasherActive ? -1.0f : 0.0f, 0.0f, 0.0f));
    ApplyRotationOffset(Assembly, EPinkCabCockpitSlot::PassengerDoor,
        FRotator(0.0f, State.bPassengerDoorOpen ? 38.0f : 0.0f, 0.0f));
}

void UPinkCabCockpitVisualDriverComponent::ApplyAvailabilityState(
    UPinkCabCockpitAssemblyComponent& Assembly,
    const FPinkCabCockpitPresentationState& State) const
{
    if (USceneComponent* Meter = Assembly.GetSlotComponent(EPinkCabCockpitSlot::Taximeter))
        Meter->SetVisibility(State.bMeterAvailable, true);
    if (USceneComponent* Door = Assembly.GetSlotComponent(EPinkCabCockpitSlot::PassengerDoor))
        Door->SetVisibility(State.bPassengerDoorAvailable, true);
    if (USceneComponent* Navigation = Assembly.GetSlotComponent(EPinkCabCockpitSlot::Navigation))
        Navigation->SetVisibility(State.bRouteAvailable, true);
    if (USceneComponent* Radio = Assembly.GetSlotComponent(EPinkCabCockpitSlot::Radio))
        Radio->SetVisibility(State.bRadioAvailable, true);

    const EPinkCabCockpitSlot MirrorSlots[] = {
        EPinkCabCockpitSlot::RearViewMirror,
        EPinkCabCockpitSlot::LeftMirror,
        EPinkCabCockpitSlot::RightMirror};
    for (const EPinkCabCockpitSlot MirrorSlot : MirrorSlots)
    {
        if (USceneComponent* Mirror = Assembly.GetSlotComponent(MirrorSlot))
            Mirror->SetVisibility(State.bMirrorsAvailable, true);
    }

    if (USceneComponent* Warnings = Assembly.GetSlotComponent(EPinkCabCockpitSlot::Warnings))
    {
        Warnings->SetVisibility(
            State.bIgnitionRunning || State.bMeterRunning || State.bPassengerDoorOpen, true);
    }
}

void UPinkCabCockpitVisualDriverComponent::Apply(
    UPinkCabCockpitAssemblyComponent& Assembly,
    const FPinkCabCockpitPresentationState& State)
{
    CacheBaseTransforms(Assembly);
    ApplySteeringState(Assembly, State);
    ApplyControlMotion(Assembly, State);
    ApplyAvailabilityState(Assembly, State);
}
