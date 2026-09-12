#include "Vehicle/PinkCabChaosWheelFront.h"

UPinkCabChaosWheelFront::UPinkCabChaosWheelFront()
{
    AxleType = EAxleType::Front;
    bAffectedBySteering = true;
    bAffectedByEngine = false;
    bAffectedByBrake = true;
    bAffectedByHandbrake = false;

    // CALIBRATION seed only; CD-787 owns final Tatra values.
    WheelRadius = 34.0f;
    WheelWidth = 18.5f;
    MaxSteerAngle = 41.0f;
    FrictionForceMultiplier = 2.0f;
    MaxBrakeTorque = 2400.0f;
}
