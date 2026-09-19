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

class PINKCABVEHICLE_API IPinkCabVehicleDynamicsProvider
{
public:
    virtual ~IPinkCabVehicleDynamicsProvider();

    virtual bool ApplyControls(const FPinkCabVehicleControlState& Controls) = 0;
    virtual bool ReadTelemetry(FPinkCabVehicleTelemetry& OutTelemetry) const = 0;
    virtual EPinkCabMechanicalClutchCapability GetMechanicalClutchCapability() const;
};

class PINKCABVEHICLE_API FPinkCabVehicleDynamicsProviderHandle
{
public:
    explicit FPinkCabVehicleDynamicsProviderHandle(IPinkCabVehicleDynamicsProvider* InProvider = nullptr);

    EPinkCabVehicleDynamicsProviderState GetState() const;
    bool ApplyControls(const FPinkCabVehicleControlState& Controls);
    bool ReadTelemetry(FPinkCabVehicleTelemetry& OutTelemetry) const;

private:
    IPinkCabVehicleDynamicsProvider* Provider = nullptr;
};
