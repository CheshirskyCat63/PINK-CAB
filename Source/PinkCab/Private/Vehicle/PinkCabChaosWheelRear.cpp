#include "Vehicle/PinkCabChaosWheelRear.h"

#include "Vehicle/PinkCabChaosPhysicalProfile.h"

UPinkCabChaosWheelRear::UPinkCabChaosWheelRear()
{
    const FPinkCabChaosPhysicalProfile Profile =
        FPinkCabChaosPhysicalProfile::ForVariant(EPinkCabCalibrationVariant::Nominal);
    Profile.ApplyToRearWheel(*this);
}
