#pragma once
#include "CoreMinimal.h"

// Physics-step evidence. This is not a second persistent vehicle state.
struct FPinkCabDrivelineStepTelemetry
{
    uint64 CommandSequence = 0;
    uint64 PhysicsStep = 0;
    float Steering = 0.0f;
    float Throttle = 0.0f;
    float Brake = 0.0f;
    float Handbrake = 0.0f;
    float Coupling = 0.0f;
    int32 Gear = 0;
    bool bCombustionAllowed = false;
    bool bNativeJointApplied = false;
    bool bCommandMatchesPreparedFrame = false;
    int32 NativeGear = 0;
    int32 NativeWheelCount = 0;
    double NativeWheelDriveNm[4] = {};
    double NativeWheelOmegaRad[4] = {};
    double EffectiveGearRatio = 0.0;
    double EngineOmegaBefore = 0.0;
    double EngineOmegaAfter = 0.0;
    double ShaftOmega = 0.0;
    double TransferredTorqueNm = 0.0;
    double CapacityNm = 0.0;
    double DissipatedEnergyJ = 0.0;
    double MomentumResidual = 0.0;
    double ConnectionEnergyDeltaJ = 0.0;
    double GearLossJ = 0.0;
    bool bNativeResponseConverged = false;
    double NativeCouplingResidualNm = 0.0;
    double PredictedShaftOmega = 0.0;
};
