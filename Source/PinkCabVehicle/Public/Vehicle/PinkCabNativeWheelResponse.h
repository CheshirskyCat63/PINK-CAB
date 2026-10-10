#pragma once
#include "CoreMinimal.h"
#include "SimpleVehicle.h"
#include "ChaosWheeledVehicleMovementComponent.h"

namespace PinkCabNativeWheelBoundary
{
struct FNativeWheelResponse
{
    double ShaftOmega = 0.0;
    double WheelWorkJ = 0.0;
    double TorqueFactor = 1.0;
    double PredictedOmega[4] = {};
    bool bValid = false;
};

// Pure native-component response used by the existing vehicle integration.
PINKCABVEHICLE_API FNativeWheelResponse EvaluateNativeWheels(
    const Chaos::FSimpleWheeledVehicle& Vehicle, const FWheelState& State,
    double ClutchTorque, double Ratio, double Efficiency, double Dt);
}
