#include "Runtime/PinkCabChaosTatraPawn.h"
#include "Runtime/PinkCabVehicleVisualProfile.h"
#include "Runtime/PinkCabVehicleVisualShellComponent.h"
#include "Cockpit/PinkCabCockpitPresentationState.h"
#include "Cockpit/PinkCabCockpitVisualDriverComponent.h"

namespace
{
FQuat Rig06GearLeverBasis(FVector2D Cursor)
{
    Cursor.X = FMath::Clamp(Cursor.X, -1.0f, 2.0f);
    Cursor.Y = FMath::Clamp(Cursor.Y, -1.0f, 1.0f);
    constexpr float ThrowDegrees = 13.75f;
    const float LateralDegrees = Cursor.X <= 1.0f
        ? ThrowDegrees * (1.0f - Cursor.X) * 0.5f
        : -ThrowDegrees * (Cursor.X - 1.0f);
    const float LongitudinalDegrees = ThrowDegrees * Cursor.Y;
    return (
        FQuat(FVector::UpVector, FMath::DegreesToRadians(LongitudinalDegrees))
        * FQuat(FVector::ForwardVector, FMath::DegreesToRadians(LateralDegrees))
    ).GetNormalized();
}

void ApplyRig06InstrumentNeedles(
    const FPinkCabCockpitPresentationState& Presentation,
    const TMap<FName, FTransform>& RestBoneTransforms,
    UPinkCabVehicleVisualShellComponent& Shell)
{
    const auto ApplyNeedle = [&RestBoneTransforms, &Shell](const FName BoneName, const float AngleDegrees)
    {
        const FTransform* Rest = RestBoneTransforms.Find(BoneName);
        if (!Rest) return;
        FTransform Dynamic = *Rest;
        const FQuat LocalTurn(
            FVector::ZAxisVector,
            FMath::DegreesToRadians(AngleDegrees));
        Dynamic.SetRotation(
            (Rest->GetRotation() * LocalTurn).GetNormalized());
        Shell.SetPoseableBoneTransform(BoneName, Dynamic);
    };

    ApplyNeedle(
        TEXT("Cabin_SpeedometerNeedle"),
        UPinkCabCockpitVisualDriverComponent::SpeedometerNeedleAngleDegrees(
            Presentation.SpeedKmh)
        - UPinkCabCockpitVisualDriverComponent::SpeedometerNeedleAngleDegrees(0.0f));
    ApplyNeedle(
        TEXT("Cabin_TachometerNeedle"),
        UPinkCabCockpitVisualDriverComponent::TachometerNeedleAngleDegrees(
            Presentation.EngineRpm)
        - UPinkCabCockpitVisualDriverComponent::TachometerNeedleAngleDegrees(0.0f));
    ApplyNeedle(
        TEXT("Cabin_FuelNeedle"),
        UPinkCabCockpitVisualDriverComponent::FuelNeedleAngleDegrees(
            Presentation.Fuel01)
        - UPinkCabCockpitVisualDriverComponent::FuelNeedleAngleDegrees(0.0f));
    ApplyNeedle(
        TEXT("Cabin_TemperatureNeedle"),
        UPinkCabCockpitVisualDriverComponent::TemperatureNeedleAngleDegrees(
            Presentation.EngineTemperature01)
        - UPinkCabCockpitVisualDriverComponent::TemperatureNeedleAngleDegrees(0.0f));
}
} // namespace

void APinkCabChaosTatraPawn::SyncRig06CockpitBones(
    const FPinkCabCockpitPresentationState& Presentation)
{
    if (!VehicleVisualShell
        || !VehicleVisualShell->GetProfile().bUsePoseableSkeletalPresentation
        || VehicleVisualShell->GetProfile().ProfileId
            != FName(TEXT("PinkCab.Visual.Tatra613.Rig06.TexturedOpenables")))
    {
        return;
    }

    // V22 authored pedal pivots share the vehicle's transverse axis. Preserve
    // their measured 24/20/28 degree downward strokes in component space;
    // the imported bone basis is not a second input or physics authority.
    const FName Pedals[] = { TEXT("Cabin_ClutchPedal"), TEXT("Cabin_BrakePedal"), TEXT("Cabin_ThrottlePedal") };
    const float Travel[] = { 24.0f, 20.0f, 28.0f };
    const float Inputs[] = { Presentation.Clutch, Presentation.Brake, Presentation.Throttle };
    for (int32 Index = 0; Index < UE_ARRAY_COUNT(Pedals); ++Index)
    {
        if (const FTransform* Rest = Rig06RestBoneTransforms.Find(Pedals[Index]))
        {
            FTransform Pedal = *Rest;
            const FQuat Press(FVector::YAxisVector,
                FMath::DegreesToRadians(-Travel[Index] * FMath::Clamp(Inputs[Index], 0.0f, 1.0f)));
            Pedal.SetRotation((Press * Rest->GetRotation()).GetNormalized());
            VehicleVisualShell->SetPoseableBoneTransform(Pedals[Index], Pedal);
        }
    }

    if (const FTransform* SteeringRest =
        Rig06RestBoneTransforms.Find(TEXT("Steering_Wheel")))
    {
        FTransform Steering = *SteeringRest;
        // RIG06 Steering_Wheel is authored around local +Y, opposite to
        // the legacy preserved-scene steering pivot. Keep semantic +right and
        // adapt only at this model boundary: +right must rotate the visible
        // wheel clockwise/right from the driver seat.
        const float AngleDegrees =
            FMath::Clamp(Presentation.Steering, -1.0f, 1.0f) * 450.0f;
        const FQuat LocalTurn(
            FVector::YAxisVector,
            FMath::DegreesToRadians(AngleDegrees));
        Steering.SetRotation(
            (SteeringRest->GetRotation() * LocalTurn).GetNormalized());
        VehicleVisualShell->SetPoseableBoneTransform(
            TEXT("Steering_Wheel"),
            Steering);
    }

    const FVector2D GearCursor = Presentation.bGearLeverDragging
        ? Presentation.GearLeverCursor
        : UPinkCabCockpitVisualDriverComponent::GearCursorForGear(
            Presentation.SelectedGear);
    ApplyRig06AuthoredLocalBasis(
        TEXT("Cabin_GearLever"),
        TEXT("root"),
        FTransform(Rig06GearLeverBasis(GearCursor)));

    // RIG11/V22 authored handbrake: lower physical hinge, local +Y axis,
    // continuous 0..1 pull reaching -32 degrees at full application.
    ApplyRig06AuthoredLocalBasis(
        TEXT("Cabin_Handbrake"),
        TEXT("root"),
        FTransform(FQuat(
            FVector::YAxisVector,
            FMath::DegreesToRadians(
                -32.0f * FMath::Clamp(Presentation.Handbrake, 0.0f, 1.0f)))));

    // Existing cockpit state remains authoritative. RIG11 authored the left
    // stalk at +/-18 degrees and the two wiper detents at +14/+26 degrees.
    const float LeftStalkDegrees = Presentation.bTurnSignalLeft
        ? -18.0f
        : (Presentation.bTurnSignalRight ? 18.0f : 0.0f);
    ApplyRig06AuthoredLocalBasis(
        TEXT("Cabin_Stalk_L"),
        TEXT("root"),
        FTransform(FQuat(
            FVector::ForwardVector,
            FMath::DegreesToRadians(LeftStalkDegrees))));

    const int32 WiperMode = FMath::Clamp(Presentation.WiperMode, 0, 2);
    const float RightStalkDegrees =
        WiperMode == 2 ? 26.0f : (WiperMode == 1 ? 14.0f : 0.0f);
    ApplyRig06AuthoredLocalBasis(
        TEXT("Cabin_Stalk_R"),
        TEXT("root"),
        FTransform(FQuat(
            FVector::ForwardVector,
            FMath::DegreesToRadians(RightStalkDegrees))));

    // The accepted cockpit state owns the one passenger-door function. The
    // RIG06 donor exposes that function as the authored front-right + rear-right
    // pair; presentation follows state without creating a second door owner.
    const float PassengerDoorTarget = Presentation.bPassengerDoorOpen ? 1.0f : 0.0f;
    Rig06OpenableTargets.FindOrAdd(TEXT("Door_FR")) = PassengerDoorTarget;
    Rig06OpenableTargets.FindOrAdd(TEXT("Door_RR")) = PassengerDoorTarget;

    ApplyRig06InstrumentNeedles(Presentation, Rig06RestBoneTransforms, *VehicleVisualShell);
}
