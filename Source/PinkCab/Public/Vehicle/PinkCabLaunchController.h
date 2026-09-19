#pragma once

#include "CoreMinimal.h"
#include "Vehicle/PinkCabVehicleMotionClassifier.h"

class FPinkCabLaunchController
{
public:
    static constexpr float DefaultThrottleTarget = 0.45f;
    static constexpr float ThrottleDoseStep = 0.05f;

    bool BeginLaunchAttempt()
    {
        if (MotionMode != EPinkCabVehicleMotionMode::Stationary || bLaunchActive)
        {
            return false;
        }
        bLaunchActive = true;
        bThrottleDoseRequired = false;
        ThrottleTarget = DefaultThrottleTarget;
        ++LaunchSerial;
        return true;
    }

    bool ApplyThrottleDoseSteps(int32 SignedSteps)
    {
        if (SignedSteps == 0)
        {
            return false;
        }
        ThrottleTarget = FMath::Clamp(
            ThrottleTarget + static_cast<float>(SignedSteps) * ThrottleDoseStep,
            0.0f,
            1.0f);
        bThrottleDoseRequired = ThrottleTarget <= KINDA_SMALL_NUMBER;
        return true;
    }

    void NotifyMotionMode(EPinkCabVehicleMotionMode InMode)
    {
        MotionMode = InMode;
        if (MotionMode == EPinkCabVehicleMotionMode::Moving)
        {
            bLaunchActive = false;
        }
    }

    EPinkCabVehicleMotionMode GetMotionMode() const { return MotionMode; }
    uint32 GetLaunchSerial() const { return LaunchSerial; }
    bool IsLaunchActive() const { return bLaunchActive; }
    bool RequiresThrottleDose() const { return bThrottleDoseRequired; }
    float GetThrottleTarget() const { return ThrottleTarget; }

private:
    EPinkCabVehicleMotionMode MotionMode = EPinkCabVehicleMotionMode::Stationary;
    uint32 LaunchSerial = 0;
    float ThrottleTarget = 0.0f;
    bool bLaunchActive = false;
    bool bThrottleDoseRequired = false;
};
