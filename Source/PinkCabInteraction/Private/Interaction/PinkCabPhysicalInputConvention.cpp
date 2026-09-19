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

    return FMath::IsNearlyZero(
        ProcessedAxis,
        0.0001f)
        ? 0.0f
        : RawAxis;
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
