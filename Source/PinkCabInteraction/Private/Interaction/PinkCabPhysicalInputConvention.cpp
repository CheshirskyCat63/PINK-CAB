#include "Interaction/PinkCabPhysicalInputConvention.h"

float FPinkCabPhysicalInputConvention::ResolveActiveDeviceAxis(
    float ProcessedAxis,
    float RawAxis)
{
    if (!FMath::IsFinite(ProcessedAxis)
        || !FMath::IsFinite(RawAxis))
    {
        return 0.0f;
    }

    // Raw MouseX/MouseY is the physical-device stream used by steering and
    // lever manipulation. A processed axis value of zero is not a freshness
    // signal for that raw stream and must never suppress a valid device delta.
    if (!FMath::IsNearlyZero(RawAxis, 0.0001f))
    {
        return RawAxis;
    }

    // Keep the processed frame delta as a safe fallback for input paths where
    // PlayerInput does not expose a raw value (including some synthetic tests).
    return ProcessedAxis;
}

float FPinkCabPhysicalInputConvention::SteeringRight(float DeviceX)
{
    return DeviceX;
}

float FPinkCabPhysicalInputConvention::GearboxForward(float DeviceY)
{
    return DeviceY;
}

float FPinkCabPhysicalInputConvention::HandbrakePull(float DeviceY)
{
    return -DeviceY;
}
