#pragma once

#include "CoreMinimal.h"
#include "Vehicle/PinkCabVehicleMotionClassifier.h"

class PINKCABVEHICLE_API FPinkCabLaunchController
{
public:
    static constexpr float ThrottleDoseStep = 0.05f;
    static constexpr float InitialThrottleTarget = 0.45f;

    bool BeginLaunchAttempt();
    bool ApplyThrottleDoseSteps(int32 SignedSteps);
    void NotifyMotionMode(EPinkCabVehicleMotionMode InMode);

    EPinkCabVehicleMotionMode GetMotionMode() const;
    uint32 GetLaunchSerial() const;
    bool IsLaunchActive() const;
    bool RequiresThrottleDose() const;
    float GetThrottleTarget() const;

private:
    EPinkCabVehicleMotionMode MotionMode = EPinkCabVehicleMotionMode::Stationary;
    uint32 LaunchSerial = 0;
    float ThrottleTarget = InitialThrottleTarget;
    bool bLaunchActive = false;
    bool bThrottleDoseRequired = false;
};
