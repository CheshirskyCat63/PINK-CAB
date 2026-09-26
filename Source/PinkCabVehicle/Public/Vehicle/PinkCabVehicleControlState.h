#pragma once

#include "CoreMinimal.h"

struct FPinkCabVehicleControlState
{
    void SetSteering(float Value) { Steering = FMath::Clamp(Value, -1.0f, 1.0f); }
    void SetThrottle(float Value) { Throttle = FMath::Clamp(Value, 0.0f, 1.0f); }
    void SetBrake(float Value) { Brake = FMath::Clamp(Value, 0.0f, 1.0f); }
    void SetClutch(float Value) { Clutch = FMath::Clamp(Value, 0.0f, 1.0f); }
    void SetHandbrake(float Value) { Handbrake = FMath::Clamp(Value, 0.0f, 1.0f); }
    void SetDriveline(int32 InRequestedGear, int32 InEngagedGear, float InClutchCoupling)
    {
        RequestedGear = FMath::Clamp(InRequestedGear, -1, 5);
        EngagedGear = FMath::Clamp(InEngagedGear, -1, 5);
        ClutchCoupling = FMath::Clamp(InClutchCoupling, 0.0f, 1.0f);
    }
    void SetDrivetrainTorqueCapacity(float Value)
    {
        DrivetrainTorqueCapacity = FMath::Clamp(Value, 0.0f, 1.0f);
    }
    void SetExternalRearDriveTorquePerWheel(float TorqueNm)
    {
        ExternalRearDriveTorquePerWheelNm = TorqueNm;
    }
    void SetResolvedEngineActuation(
        bool bAllowed,
        float PreLimiterThrottle01,
        float FinalThrottle01,
        float TorqueCurveNm,
        float AvailableTorqueNm)
    {
        bCombustionAllowed = bAllowed;
        ResolvedEngineThrottlePreLimiter01 =
            FMath::Clamp(PreLimiterThrottle01, 0.0f, 1.0f);
        ResolvedEngineThrottle01 =
            FMath::Clamp(FinalThrottle01, 0.0f, 1.0f);
        ResolvedEngineTorqueCurveNm = FMath::Max(TorqueCurveNm, 0.0f);
        AvailableEngineTorqueNm = FMath::Max(AvailableTorqueNm, 0.0f);
    }

    bool IsCombustionAllowed() const { return bCombustionAllowed; }
    float GetResolvedEngineThrottlePreLimiter01() const
    {
        return ResolvedEngineThrottlePreLimiter01;
    }
    float GetResolvedEngineThrottle01() const { return ResolvedEngineThrottle01; }
    float GetResolvedEngineTorqueCurveNm() const { return ResolvedEngineTorqueCurveNm; }
    float GetAvailableEngineTorqueNm() const { return AvailableEngineTorqueNm; }

    float Steering = 0.0f;
    float Throttle = 0.0f;
    float Brake = 0.0f;
    float Clutch = 0.0f;
    float Handbrake = 0.0f;
    int32 RequestedGear = 0;
    int32 EngagedGear = 0;
    float ClutchCoupling = 1.0f;
    float DrivetrainTorqueCapacity = 1.0f;
    float ExternalRearDriveTorquePerWheelNm = 0.0f;
    bool bCombustionAllowed = false;
    float ResolvedEngineThrottlePreLimiter01 = 0.0f;
    float ResolvedEngineThrottle01 = 0.0f;
    float ResolvedEngineTorqueCurveNm = 0.0f;
    float AvailableEngineTorqueNm = 0.0f;
};
