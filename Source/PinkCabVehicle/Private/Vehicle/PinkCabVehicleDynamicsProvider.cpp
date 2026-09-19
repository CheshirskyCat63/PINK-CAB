#include "Vehicle/PinkCabVehicleDynamicsProvider.h"

IPinkCabVehicleDynamicsProvider::~IPinkCabVehicleDynamicsProvider() = default;

EPinkCabMechanicalClutchCapability IPinkCabVehicleDynamicsProvider::GetMechanicalClutchCapability() const
{
    return EPinkCabMechanicalClutchCapability::Unsupported;
}

FPinkCabVehicleDynamicsProviderHandle::FPinkCabVehicleDynamicsProviderHandle(
    IPinkCabVehicleDynamicsProvider* InProvider)
    : Provider(InProvider)
{
}

EPinkCabVehicleDynamicsProviderState FPinkCabVehicleDynamicsProviderHandle::GetState() const
{
    return Provider
        ? EPinkCabVehicleDynamicsProviderState::Ready
        : EPinkCabVehicleDynamicsProviderState::NoProvider;
}

bool FPinkCabVehicleDynamicsProviderHandle::ApplyControls(
    const FPinkCabVehicleControlState& Controls)
{
    return Provider ? Provider->ApplyControls(Controls) : false;
}

bool FPinkCabVehicleDynamicsProviderHandle::ReadTelemetry(
    FPinkCabVehicleTelemetry& OutTelemetry) const
{
    return Provider ? Provider->ReadTelemetry(OutTelemetry) : false;
}
