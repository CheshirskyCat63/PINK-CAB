#pragma once

#include "CoreMinimal.h"

/**
 * Single boundary between Unreal/OS mouse-axis samples and driver controls.
 *
 * Permanent driver-space convention:
 *   +X = right
 *   +Y = gearbox forward (1/3/5 row)
 * Physical-control consumers must not reinterpret device signs themselves.
 */
struct PINKCABINTERACTION_API FPinkCabPhysicalInputConvention
{
    static float ResolveActiveDeviceAxis(
        float ProcessedAxis,
        float RawAxis);
    static float SteeringRight(float DeviceX);
    static float GearboxForward(float DeviceY);
    static float HandbrakePull(float DeviceY);
};
