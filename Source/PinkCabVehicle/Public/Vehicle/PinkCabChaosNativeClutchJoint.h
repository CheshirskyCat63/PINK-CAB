#pragma once
#include "CoreMinimal.h"

struct FPinkCabNativeClutchResult
{
    bool bValid = false;
    double EngineOmega = 0.0;
    double TransferredTorqueNm = 0.0;
    double DissipatedEnergyJ = 0.0;
    double MomentumResidual = 0.0;
};

// Native holding/limited-slip joint. Mode selection is a pressure-capacity adapter.
// A gripping native wheel exposes a prescribed road-speed boundary, not a free rotor.
// Its transient rotors gather existing shaft state, never own world bodies.
class PINKCABVEHICLE_API FPinkCabChaosNativeClutchJoint
{
public:
    FPinkCabChaosNativeClutchJoint();
    ~FPinkCabChaosNativeClutchJoint();
    FPinkCabNativeClutchResult Solve(double EngineOmega, double ShaftOmega,
        double EngineInertiaKgM2, double ShaftInertiaKgM2, double CapacityNm,
        double SynchronizationSeconds, double DeltaSeconds, bool bPrescribedRoadShaft = false);
private:
    struct FImplementation;
    TUniquePtr<FImplementation> Implementation;
};
