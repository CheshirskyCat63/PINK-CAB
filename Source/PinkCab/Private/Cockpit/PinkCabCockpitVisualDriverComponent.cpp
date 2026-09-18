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
    return FMath::Clamp(Steering, -1.0f, 1.0f) * 450.0f;
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
    switch (FMath::Clamp(Gear, -1, 5))
    {
    case -1: return FVector2D(0.75f, -1.0f);
    case 1: return FVector2D(-0.75f, 1.0f);
    case 2: return FVector2D(-0.75f, -1.0f);
    case 3: return FVector2D(0.0f, 1.0f);
    case 4: return FVector2D(0.0f, -1.0f);
    case 5: return FVector2D(0.75f, 1.0f);
    default: return FVector2D::ZeroVector;
    }
}

FVector2D UPinkCabCockpitVisualDriverComponent::IntegrateGearCursor(
    FVector2D Current, const float MouseDeltaX, const float MouseDeltaY, const float Gain)
{
    Current.X = FMath::Clamp(Current.X + MouseDeltaX * Gain, -1.30f, 1.0f);
    // Screen-space mouse Y grows downward while physical gearbox forward is +Y.
    Current.Y = FMath::Clamp(Current.Y - MouseDeltaY * Gain, -1.0f, 1.0f);
    if (FMath::Abs(Current.Y) < 0.22f)
    {
        Current.Y = FMath::Lerp(Current.Y, 0.0f, 0.35f);
    }
    else if (FMath::Abs(Current.Y) > 0.45f)
    {
        const float LaneX = Current.X < -0.35f ? -0.75f : (Current.X < 0.38f ? 0.0f : 0.75f);
        Current.X = FMath::Lerp(Current.X, LaneX, 0.18f);
    }
    return Current;
}

FVector UPinkCabCockpitVisualDriverComponent::GearLeverOffsetFromCursor(FVector2D Cursor)
{
    Cursor.X = FMath::Clamp(Cursor.X, -1.30f, 1.0f);
    Cursor.Y = FMath::Clamp(Cursor.Y, -1.0f, 1.0f);
    // Tatra cockpit local axes: X crosses the H gate left/right, while negative Y
    // points forward. Preserve the canonical 1/3/5 forward and 2/4/R rearward layout.
    return FVector(Cursor.X * 7.0f, -Cursor.Y * 6.0f, 0.0f);
}

int32 UPinkCabCockpitVisualDriverComponent::GearForCursor(FVector2D Cursor)
{
    Cursor.X = FMath::Clamp(Cursor.X, -1.30f, 1.0f);
    Cursor.Y = FMath::Clamp(Cursor.Y, -1.0f, 1.0f);
    if (FMath::Abs(Cursor.Y) < 0.32f) return 0;
    if (Cursor.X < -0.35f) return Cursor.Y > 0.0f ? 1 : 2;
    if (Cursor.X < 0.38f) return Cursor.Y > 0.0f ? 3 : 4;
    return Cursor.Y > 0.0f ? 5 : -1;
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
    if (USceneComponent* Steering = SteeringVisualComponent.Get())
    {
        if (!bSteeringVisualBaseValid)
        {
            SteeringVisualBaseTransform = Steering->GetRelativeTransform();
            bSteeringVisualBaseValid = true;
        }

        // The source steering pivot is authored with local X along the
        // steering-column axis. Rotate the pivot around that axis only.
        const float AngleRadians = FMath::DegreesToRadians(SteeringAngleDegrees(State.Steering));
        const FQuat LocalTurn(FVector::ForwardVector, AngleRadians);
        Steering->SetRelativeLocation(SteeringVisualBaseTransform.GetLocation());
        Steering->SetRelativeRotation(SteeringVisualBaseTransform.GetRotation() * LocalTurn);
    }
    else if (USceneComponent* FallbackSteering = Assembly.GetSlotComponent(EPinkCabCockpitSlot::SteeringWheel))
    {
        if (const FTransform* Base = GetBase(EPinkCabCockpitSlot::SteeringWheel))
        {
            const FRotator Offset(0.0f, 0.0f, SteeringAngleDegrees(State.Steering));
            FallbackSteering->SetRelativeLocationAndRotation(Base->GetLocation(), Base->Rotator() + Offset);
        }
    }
    SetRotOffset(EPinkCabCockpitSlot::ClutchPedal,
        FRotator(PedalTravelDegrees(State.Clutch), 0.0f, 0.0f));
    SetRotOffset(EPinkCabCockpitSlot::BrakePedal,
        FRotator(PedalTravelDegrees(State.Brake), 0.0f, 0.0f));
    SetRotOffset(EPinkCabCockpitSlot::ThrottlePedal,
        FRotator(PedalTravelDegrees(State.Throttle), 0.0f, 0.0f));
    SetLocOffset(EPinkCabCockpitSlot::Gearbox,
        State.bGearLeverDragging ? GearLeverOffsetFromCursor(State.GearLeverCursor) : GearLeverOffset(State.SelectedGear));
    SetRotOffset(EPinkCabCockpitSlot::Handbrake,
        FRotator(HandbrakeAngleDegrees(State.Handbrake), 0.0f, 0.0f));
    SetRotOffset(EPinkCabCockpitSlot::TemperatureNeedle,
        FRotator(0.0f, 0.0f, TemperatureNeedleAngleDegrees(State.EngineTemperature01)));
    SetRotOffset(EPinkCabCockpitSlot::FuelNeedle,
        FRotator(0.0f, 0.0f, FuelNeedleAngleDegrees(State.Fuel01)));
    SetRotOffset(EPinkCabCockpitSlot::SpeedometerNeedle,
        FRotator(0.0f, 0.0f, SpeedometerNeedleAngleDegrees(State.SpeedKmh)));
    SetRotOffset(EPinkCabCockpitSlot::TachometerNeedle,
        FRotator(0.0f, 0.0f, TachometerNeedleAngleDegrees(State.EngineRpm)));
    SetRotOffset(EPinkCabCockpitSlot::Ignition,
        FRotator(0.0f, State.bIgnitionRunning ? 42.0f : 0.0f, 0.0f));
    SetRotOffset(EPinkCabCockpitSlot::TurnSignals,
        FRotator(0.0f, State.bTurnSignalLeft ? -24.0f : (State.bTurnSignalRight ? 24.0f : 0.0f), 0.0f));
    SetLocOffset(EPinkCabCockpitSlot::Horn,
        FVector(State.bHornActive ? -1.5f : 0.0f, 0.0f, 0.0f));
    SetRotOffset(EPinkCabCockpitSlot::Lights,
        FRotator(State.bLightsOn ? 32.0f : 0.0f, 0.0f, 0.0f));
    SetRotOffset(EPinkCabCockpitSlot::Wipers,
        FRotator(State.bWipersOn ? 32.0f : 0.0f, 0.0f, 0.0f));
    SetLocOffset(EPinkCabCockpitSlot::Washer,
        FVector(State.bWasherActive ? -1.0f : 0.0f, 0.0f, 0.0f));
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
