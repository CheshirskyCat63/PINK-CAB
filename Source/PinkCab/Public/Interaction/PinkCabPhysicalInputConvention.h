#pragma once

#include "CoreMinimal.h"

/**
 * Single boundary between Unreal/OS mouse-axis samples and driver-space controls.
 *
 * Driver-space convention is permanent:
 *   +X = right
 *   +Y = gearbox forward (1/3/5 row)
 * Physical-control consumers must not reinterpret device signs themselves.
 */
struct FPinkCabPhysicalInputConvention
{
    static float ResolveActiveDeviceAxis(const float ProcessedAxis, const float RawAxis)
    {
        if (!FMath::IsFinite(ProcessedAxis) || !FMath::IsFinite(RawAxis))
        {
            return 0.0f;
        }

        // Raw input can retain a cached value on frames where the corresponding
        // processed axis did not move. Only accept raw magnitude on an active axis.
        return FMath::IsNearlyZero(ProcessedAxis, 0.0001f) ? 0.0f : RawAxis;
    }

    static float SteeringRight(const float DeviceX)
    {
        return DeviceX;
    }

    static float GearboxForward(const float DeviceY)
    {
        // Owner-calibrated PINK CAB convention: positive device Y is the
        // forward/top H-gate gesture. This sign is owned only here.
        return DeviceY;
    }

    static float HandbrakePull(const float DeviceY)
    {
        // Preserve the existing handbrake physical gesture while keeping its
        // device-space conversion out of the actuator itself.
        return -DeviceY;
    }
};
