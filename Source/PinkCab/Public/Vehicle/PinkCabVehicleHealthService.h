#pragma once

#include "CoreMinimal.h"
#include "Vehicle/PinkCabVehicleHealthState.h"
#include "Vehicle/PinkCabVehicleHitEvent.h"

enum class EPinkCabVehicleCapability : uint8
{
    Roll,
    Steer,
    Brake,
    RunEngine
};

struct FPinkCabVehicleHealthService
{
    bool ApplyHit(FPinkCabVehicleHealthState& State, const FPinkCabVehicleHitEvent& Event) const
    {
        if (Event.Severity <= 0.0f)
        {
            return false;
        }
        if (Event.bCosmeticOnly || Event.Channel == EPinkCabVehicleHealthChannel::CosmeticBody)
        {
            return true;
        }
        return State.ApplyFunctionalDamage(Event.Channel, Event.Severity);
    }

    bool ResolveSyntheticHitZone(FName ZoneId, float Severity, FPinkCabVehicleHitEvent& OutEvent) const
    {
        EPinkCabVehicleHealthChannel Channel;
        if (ZoneId == FName(TEXT("FrontLeftWheel"))) Channel = EPinkCabVehicleHealthChannel::Wheel;
        else if (ZoneId == FName(TEXT("Tire"))) Channel = EPinkCabVehicleHealthChannel::Tire;
        else if (ZoneId == FName(TEXT("Alignment"))) Channel = EPinkCabVehicleHealthChannel::Alignment;
        else if (ZoneId == FName(TEXT("Suspension"))) Channel = EPinkCabVehicleHealthChannel::Suspension;
        else if (ZoneId == FName(TEXT("Brake"))) Channel = EPinkCabVehicleHealthChannel::Brake;
        else if (ZoneId == FName(TEXT("Door"))) Channel = EPinkCabVehicleHealthChannel::Door;
        else if (ZoneId == FName(TEXT("Lamp"))) Channel = EPinkCabVehicleHealthChannel::Lamp;
        else if (ZoneId == FName(TEXT("EngineOil"))) Channel = EPinkCabVehicleHealthChannel::EngineOil;
        else if (ZoneId == FName(TEXT("EngineFan"))) Channel = EPinkCabVehicleHealthChannel::EngineFan;
        else if (ZoneId == FName(TEXT("OilCooler"))) Channel = EPinkCabVehicleHealthChannel::OilCooler;
        else if (ZoneId == FName(TEXT("Airflow"))) Channel = EPinkCabVehicleHealthChannel::Airflow;
        else if (ZoneId == FName(TEXT("CosmeticBody"))) Channel = EPinkCabVehicleHealthChannel::CosmeticBody;
        else return false;

        OutEvent = FPinkCabVehicleHitEvent(Channel, FMath::Clamp(Severity, 0.0f, 1.0f),
            Channel == EPinkCabVehicleHealthChannel::CosmeticBody);
        return true;
    }

    bool HasCapability(const FPinkCabVehicleHealthState& State, EPinkCabVehicleCapability Capability) const
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
            return State.GetHealth(EPinkCabVehicleHealthChannel::Brake) > 0.0f;
        case EPinkCabVehicleCapability::RunEngine:
            return State.GetHealth(EPinkCabVehicleHealthChannel::EngineOil) > 0.0f
                && State.GetHealth(EPinkCabVehicleHealthChannel::EngineFan) > 0.0f
                && State.GetHealth(EPinkCabVehicleHealthChannel::OilCooler) > 0.0f
                && State.GetHealth(EPinkCabVehicleHealthChannel::Airflow) > 0.0f;
        default:
            return false;
        }
    }

    bool IsTerminal(const FPinkCabVehicleHealthState& State) const
    {
        return !HasCapability(State, EPinkCabVehicleCapability::Roll)
            || !HasCapability(State, EPinkCabVehicleCapability::Steer)
            || !HasCapability(State, EPinkCabVehicleCapability::Brake)
            || !HasCapability(State, EPinkCabVehicleCapability::RunEngine);
    }
};
