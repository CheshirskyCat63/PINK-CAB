#include "Vehicle/PinkCabVehicleHealthService.h"

namespace
{
struct FPinkCabSyntheticZoneMapping
{
    const TCHAR* ZoneName;
    EPinkCabVehicleHealthChannel Channel;
};

const FPinkCabSyntheticZoneMapping SyntheticZoneMappings[] = {
    {TEXT("FrontLeftWheel"), EPinkCabVehicleHealthChannel::Wheel},
    {TEXT("Tire"), EPinkCabVehicleHealthChannel::Tire},
    {TEXT("Alignment"), EPinkCabVehicleHealthChannel::Alignment},
    {TEXT("Suspension"), EPinkCabVehicleHealthChannel::Suspension},
    {TEXT("Brake"), EPinkCabVehicleHealthChannel::Brake},
    {TEXT("BrakeHeat"), EPinkCabVehicleHealthChannel::BrakeHeat},
    {TEXT("BrakeHydraulic"), EPinkCabVehicleHealthChannel::BrakeHydraulic},
    {TEXT("Clutch"), EPinkCabVehicleHealthChannel::Clutch},
    {TEXT("Gearbox"), EPinkCabVehicleHealthChannel::Gearbox},
    {TEXT("Door"), EPinkCabVehicleHealthChannel::Door},
    {TEXT("Lamp"), EPinkCabVehicleHealthChannel::Lamp},
    {TEXT("Glass"), EPinkCabVehicleHealthChannel::Glass},
    {TEXT("EngineOil"), EPinkCabVehicleHealthChannel::EngineOil},
    {TEXT("EngineHead"), EPinkCabVehicleHealthChannel::EngineHead},
    {TEXT("EngineFan"), EPinkCabVehicleHealthChannel::EngineFan},
    {TEXT("OilCooler"), EPinkCabVehicleHealthChannel::OilCooler},
    {TEXT("Airflow"), EPinkCabVehicleHealthChannel::Airflow},
    {TEXT("CosmeticBody"), EPinkCabVehicleHealthChannel::CosmeticBody},
};

bool TryResolveSyntheticChannel(
    FName ZoneId,
    EPinkCabVehicleHealthChannel& OutChannel)
{
    for (const FPinkCabSyntheticZoneMapping& Mapping : SyntheticZoneMappings)
    {
        if (ZoneId == FName(Mapping.ZoneName))
        {
            OutChannel = Mapping.Channel;
            return true;
        }
    }
    return false;
}
} // namespace

bool FPinkCabVehicleHealthService::ApplyHit(
    FPinkCabVehicleHealthState& State,
    const FPinkCabVehicleHitEvent& Event) const
{
    if (Event.Severity <= 0.0f) return false;
    if (Event.bCosmeticOnly
        || Event.Channel == EPinkCabVehicleHealthChannel::CosmeticBody)
    {
        return true;
    }
    return State.ApplyFunctionalDamage(Event.Channel, Event.Severity);
}

bool FPinkCabVehicleHealthService::ResolveSyntheticHitZone(
    FName ZoneId,
    float Severity,
    FPinkCabVehicleHitEvent& OutEvent) const
{
    EPinkCabVehicleHealthChannel Channel = EPinkCabVehicleHealthChannel::Count;
    if (!TryResolveSyntheticChannel(ZoneId, Channel)) return false;

    OutEvent = FPinkCabVehicleHitEvent(
        Channel,
        FMath::Clamp(Severity, 0.0f, 1.0f),
        Channel == EPinkCabVehicleHealthChannel::CosmeticBody);
    return true;
}

bool FPinkCabVehicleHealthService::ResolveAuthoredHitZone(
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

float FPinkCabVehicleHealthService::GetBrakeEffectiveness01(
    const FPinkCabVehicleHealthState& State) const
{
    return FMath::Min3(
        State.GetHealth(EPinkCabVehicleHealthChannel::Brake),
        State.GetHealth(EPinkCabVehicleHealthChannel::BrakeHeat),
        State.GetHealth(EPinkCabVehicleHealthChannel::BrakeHydraulic));
}

float FPinkCabVehicleHealthService::GetEnginePowerEffectiveness01(
    const FPinkCabVehicleHealthState& State) const
{
    float Result = State.GetHealth(EPinkCabVehicleHealthChannel::EngineOil);
    Result = FMath::Min(Result, State.GetHealth(EPinkCabVehicleHealthChannel::EngineHead));
    Result = FMath::Min(Result, State.GetHealth(EPinkCabVehicleHealthChannel::EngineFan));
    Result = FMath::Min(Result, State.GetHealth(EPinkCabVehicleHealthChannel::OilCooler));
    Result = FMath::Min(Result, State.GetHealth(EPinkCabVehicleHealthChannel::Airflow));
    return Result;
}

bool FPinkCabVehicleHealthService::HasCapability(
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

void FPinkCabVehicleHealthService::ApplyCapabilitiesToControls(
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

    const bool bCanDrive =
        HasCapability(State, EPinkCabVehicleCapability::RunEngine)
        && HasCapability(State, EPinkCabVehicleCapability::Roll)
        && HasCapability(State, EPinkCabVehicleCapability::Drive);
    Controls.SetThrottle(
        bCanDrive
            ? FMath::Min(Controls.Throttle, GetEnginePowerEffectiveness01(State))
            : 0.0f);
}

bool FPinkCabVehicleHealthService::IsTerminal(
    const FPinkCabVehicleHealthState& State) const
{
    return !HasCapability(State, EPinkCabVehicleCapability::Roll)
        || !HasCapability(State, EPinkCabVehicleCapability::Steer)
        || !HasCapability(State, EPinkCabVehicleCapability::Brake)
        || !HasCapability(State, EPinkCabVehicleCapability::RunEngine)
        || !HasCapability(State, EPinkCabVehicleCapability::Drive);
}
