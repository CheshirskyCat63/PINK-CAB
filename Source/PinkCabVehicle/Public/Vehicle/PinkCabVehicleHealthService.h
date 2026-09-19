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

struct FPinkCabVehicleHealthService
{
    bool ApplyHit(
        FPinkCabVehicleHealthState& State,
        const FPinkCabVehicleHitEvent& Event) const
    {
        if (Event.Severity <= 0.0f)
        {
            return false;
        }
        if (Event.bCosmeticOnly
            || Event.Channel == EPinkCabVehicleHealthChannel::CosmeticBody)
        {
            return true;
        }
        return State.ApplyFunctionalDamage(Event.Channel, Event.Severity);
    }

    bool ResolveSyntheticHitZone(
        FName ZoneId,
        float Severity,
        FPinkCabVehicleHitEvent& OutEvent) const
    {
        EPinkCabVehicleHealthChannel Channel;
        if (ZoneId == FName(TEXT("FrontLeftWheel"))) Channel = EPinkCabVehicleHealthChannel::Wheel;
        else if (ZoneId == FName(TEXT("Tire"))) Channel = EPinkCabVehicleHealthChannel::Tire;
        else if (ZoneId == FName(TEXT("Alignment"))) Channel = EPinkCabVehicleHealthChannel::Alignment;
        else if (ZoneId == FName(TEXT("Suspension"))) Channel = EPinkCabVehicleHealthChannel::Suspension;
        else if (ZoneId == FName(TEXT("Brake"))) Channel = EPinkCabVehicleHealthChannel::Brake;
        else if (ZoneId == FName(TEXT("BrakeHeat"))) Channel = EPinkCabVehicleHealthChannel::BrakeHeat;
        else if (ZoneId == FName(TEXT("BrakeHydraulic"))) Channel = EPinkCabVehicleHealthChannel::BrakeHydraulic;
        else if (ZoneId == FName(TEXT("Clutch"))) Channel = EPinkCabVehicleHealthChannel::Clutch;
        else if (ZoneId == FName(TEXT("Gearbox"))) Channel = EPinkCabVehicleHealthChannel::Gearbox;
        else if (ZoneId == FName(TEXT("Door"))) Channel = EPinkCabVehicleHealthChannel::Door;
        else if (ZoneId == FName(TEXT("Lamp"))) Channel = EPinkCabVehicleHealthChannel::Lamp;
        else if (ZoneId == FName(TEXT("Glass"))) Channel = EPinkCabVehicleHealthChannel::Glass;
        else if (ZoneId == FName(TEXT("EngineOil"))) Channel = EPinkCabVehicleHealthChannel::EngineOil;
        else if (ZoneId == FName(TEXT("EngineHead"))) Channel = EPinkCabVehicleHealthChannel::EngineHead;
        else if (ZoneId == FName(TEXT("EngineFan"))) Channel = EPinkCabVehicleHealthChannel::EngineFan;
        else if (ZoneId == FName(TEXT("OilCooler"))) Channel = EPinkCabVehicleHealthChannel::OilCooler;
        else if (ZoneId == FName(TEXT("Airflow"))) Channel = EPinkCabVehicleHealthChannel::Airflow;
        else if (ZoneId == FName(TEXT("CosmeticBody"))) Channel = EPinkCabVehicleHealthChannel::CosmeticBody;
        else return false;

        OutEvent = FPinkCabVehicleHitEvent(
            Channel,
            FMath::Clamp(Severity, 0.0f, 1.0f),
            Channel == EPinkCabVehicleHealthChannel::CosmeticBody);
        return true;
    }

    bool ResolveAuthoredHitZone(
        FName ZoneId,
        float CollisionSeverity,
        float FunctionalThreshold,
        FPinkCabVehicleHitEvent& OutEvent) const
    {
        if (!FMath::IsFinite(CollisionSeverity)
            || !FMath::IsFinite(FunctionalThreshold)
            || FunctionalThreshold < 0.0f
            || FunctionalThreshold > 1.0f
            || CollisionSeverity <= FunctionalThreshold)
        {
            return false;
        }
        return ResolveSyntheticHitZone(ZoneId, CollisionSeverity, OutEvent);
    }

    float GetBrakeEffectiveness01(const FPinkCabVehicleHealthState& State) const
    {
        return FMath::Min3(
            State.GetHealth(EPinkCabVehicleHealthChannel::Brake),
            State.GetHealth(EPinkCabVehicleHealthChannel::BrakeHeat),
            State.GetHealth(EPinkCabVehicleHealthChannel::BrakeHydraulic));
    }

    float GetEnginePowerEffectiveness01(const FPinkCabVehicleHealthState& State) const
    {
        float Result = State.GetHealth(EPinkCabVehicleHealthChannel::EngineOil);
        Result = FMath::Min(Result, State.GetHealth(EPinkCabVehicleHealthChannel::EngineHead));
        Result = FMath::Min(Result, State.GetHealth(EPinkCabVehicleHealthChannel::EngineFan));
        Result = FMath::Min(Result, State.GetHealth(EPinkCabVehicleHealthChannel::OilCooler));
        Result = FMath::Min(Result, State.GetHealth(EPinkCabVehicleHealthChannel::Airflow));
        return Result;
    }

    bool HasCapability(
        const FPinkCabVehicleHealthState& State,
        EPinkCabVehicleCapability Capability) const
    {
        switch (Capability)
        {
        case EPinkCabVehicleCapability::Roll:
            return State.GetHealth(EPinkCabVehicleHealthChannel::Wheel) > 0.0f
                && State.GetHealth(EPinkCabVehicleHealthChannel::Tire) > 0.0f;
        case EPinkCabVehicleCapability::Steer:
            return State.GetHealth(EPinkCabVehicleHealthChannel::Alignment) > 0.0f
                && State.GetHealth(EPinkCabVehicleHealthChannel::Suspension) > 0.0f;
        case EPinkCabVehicleCapability::Brake:
            return GetBrakeEffectiveness01(State) > 0.0f;
        case EPinkCabVehicleCapability::RunEngine:
            return GetEnginePowerEffectiveness01(State) > 0.0f;
        case EPinkCabVehicleCapability::Drive:
            return State.GetHealth(EPinkCabVehicleHealthChannel::Clutch) > 0.0f
                && State.GetHealth(EPinkCabVehicleHealthChannel::Gearbox) > 0.0f;
        default:
            return false;
        }
    }

    void ApplyCapabilitiesToControls(
        const FPinkCabVehicleHealthState& State,
        FPinkCabVehicleControlState& Controls) const
    {
        if (!HasCapability(State, EPinkCabVehicleCapability::Steer))
        {
            Controls.SetSteering(0.0f);
        }

        const float BrakeEffectiveness = GetBrakeEffectiveness01(State);
        Controls.SetBrake(
            BrakeEffectiveness > 0.0f
                ? FMath::Min(Controls.Brake, BrakeEffectiveness)
                : 0.0f);

        if (!HasCapability(State, EPinkCabVehicleCapability::RunEngine)
            || !HasCapability(State, EPinkCabVehicleCapability::Roll)
            || !HasCapability(State, EPinkCabVehicleCapability::Drive))
        {
            Controls.SetThrottle(0.0f);
        }
        else
        {
            Controls.SetThrottle(
                FMath::Min(
                    Controls.Throttle,
                    GetEnginePowerEffectiveness01(State)));
        }
    }

    bool IsTerminal(const FPinkCabVehicleHealthState& State) const
    {
        return !HasCapability(State, EPinkCabVehicleCapability::Roll)
            || !HasCapability(State, EPinkCabVehicleCapability::Steer)
            || !HasCapability(State, EPinkCabVehicleCapability::Brake)
            || !HasCapability(State, EPinkCabVehicleCapability::RunEngine)
            || !HasCapability(State, EPinkCabVehicleCapability::Drive);
    }
};
