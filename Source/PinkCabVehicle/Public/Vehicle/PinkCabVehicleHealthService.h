#pragma once

#include "CoreMinimal.h"
#include "Vehicle/PinkCabVehicleControlState.h"
#include "Vehicle/PinkCabVehicleHealthState.h"
#include "Vehicle/PinkCabVehicleHitEvent.h"

enum class EPinkCabVehicleCapability : uint8
{
    Roll,
    Steer,
    Brake,
    RunEngine,
    Drive
};

struct PINKCABVEHICLE_API FPinkCabVehicleHealthService
{
    bool ApplyHit(
        FPinkCabVehicleHealthState& State,
        const FPinkCabVehicleHitEvent& Event) const;
    bool ResolveSyntheticHitZone(
        FName ZoneId,
        float Severity,
        FPinkCabVehicleHitEvent& OutEvent) const;
    bool ResolveAuthoredHitZone(
        FName ZoneId,
        float CollisionSeverity,
        float FunctionalThreshold,
        FPinkCabVehicleHitEvent& OutEvent) const;
    float GetBrakeEffectiveness01(const FPinkCabVehicleHealthState& State) const;
    float GetEnginePowerEffectiveness01(const FPinkCabVehicleHealthState& State) const;
    bool HasCapability(
        const FPinkCabVehicleHealthState& State,
        EPinkCabVehicleCapability Capability) const;
    void ApplyCapabilitiesToControls(
        const FPinkCabVehicleHealthState& State,
        FPinkCabVehicleControlState& Controls) const;
    bool IsTerminal(const FPinkCabVehicleHealthState& State) const;
};
