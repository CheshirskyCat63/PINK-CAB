#include "Vehicle/PinkCabChaosWheelFront.h"

#include "Vehicle/PinkCabChaosPhysicalProfile.h"

UPinkCabChaosWheelFront::UPinkCabChaosWheelFront()
{
    const FPinkCabChaosPhysicalProfile Profile =
        FPinkCabChaosPhysicalProfile::ForVariant(EPinkCabCalibrationVariant::Nominal);
    Profile.ApplyToFrontWheel(*this);
}
