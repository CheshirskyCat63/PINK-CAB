#include "Vehicle/PinkCabChaosWheelRear.h"

UPinkCabChaosWheelRear::UPinkCabChaosWheelRear()
{
    AxleType = EAxleType::Rear;
    bAffectedBySteering = false;
    bAffectedByEngine = true;
    bAffectedByBrake = true;
    bAffectedByHandbrake = true;

    // CALIBRATION seed only; CD-787 owns final Tatra values.
    WheelRadius = 34.0f;
    WheelWidth = 18.5f;
    FrictionForceMultiplier = 2.0f;
    MaxBrakeTorque = 2200.0f;
    MaxHandBrakeTorque = 1700.0f;
}
