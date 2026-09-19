#pragma once

#include "Vehicle/PinkCabVehicleControlState.h"
#include "Vehicle/PinkCabVehicleTelemetry.h"

enum class EPinkCabVehicleDynamicsProviderState : uint8
{
    NoProvider,
    Ready
};

enum class EPinkCabMechanicalClutchCapability : uint8
{
    Unsupported,
    EmulatedNeutralGate,
    ContinuousExternalTorque,
    Native
};

class IPinkCabVehicleDynamicsProvider
{
public:
    virtual ~IPinkCabVehicleDynamicsProvider() = default;

    // Sole gameplay-frame write seam for vehicle dynamics. Engine-specific
    // steering signs, pedal mapping and wheel torque writes stay behind it.
    virtual bool ApplyControls(const FPinkCabVehicleControlState& Controls) = 0;

    // Read-only normalized telemetry seam; never mutates engine state.
    virtual bool ReadTelemetry(FPinkCabVehicleTelemetry& OutTelemetry) const = 0;
    virtual EPinkCabMechanicalClutchCapability GetMechanicalClutchCapability() const
    {
        return EPinkCabMechanicalClutchCapability::Unsupported;
    }
};

class FPinkCabVehicleDynamicsProviderHandle
{
public:
    explicit FPinkCabVehicleDynamicsProviderHandle(IPinkCabVehicleDynamicsProvider* InProvider = nullptr)
        : Provider(InProvider)
    {
    }

    EPinkCabVehicleDynamicsProviderState GetState() const
    {
        return Provider ? EPinkCabVehicleDynamicsProviderState::Ready
                        : EPinkCabVehicleDynamicsProviderState::NoProvider;
    }

    bool ApplyControls(const FPinkCabVehicleControlState& Controls)
    {
        return Provider ? Provider->ApplyControls(Controls) : false;
    }

    bool ReadTelemetry(FPinkCabVehicleTelemetry& OutTelemetry) const
    {
        return Provider ? Provider->ReadTelemetry(OutTelemetry) : false;
    }

private:
    IPinkCabVehicleDynamicsProvider* Provider = nullptr;
};
