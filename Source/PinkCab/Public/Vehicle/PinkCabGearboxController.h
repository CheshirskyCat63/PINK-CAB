#pragma once

#include "CoreMinimal.h"

enum class EPinkCabGearEngagementResult : uint8
{
    None,
    Neutral,
    ClutchDisengagedAccepted,
    MatchedClutchlessAccepted,
    GrindRefused,
    ReverseLockout,
    DangerousOverrev
};

struct FPinkCabGearEngagementContext
{
    float ClutchPedal = 0.0f; // 0 released/coupled, 1 pressed/disengaged
    float EngineRpm = 0.0f;
    float SpeedKmh = 0.0f;
    float Throttle = 0.0f;
    float Brake = 0.0f;
    float GearboxHealth = 1.0f;
};

struct FPinkCabGearboxControllerConfig
{
    float ClutchDisengagedThreshold = 0.85f;
    float ClutchCurveExponent = 1.35f;
    float ClutchlessMatchToleranceRpm = 250.0f;
    float ClutchlessLoadThreshold = 0.15f;
    float ReverseLockoutSpeedKmh = 5.0f;
    float MaxSafeEngineRpm = 6500.0f;
    float IdleRpm = 750.0f;
    // Derived from the canonical physical profile:
    // wheel radius 32.13 cm, final drive 3.2,
    // forward 4.6/2.2/1.5/1.1/0.85, reverse 4.6.
    float RpmPerKmh[6] = {
        0.0f, 121.5251f, 58.1207f, 39.6277f, 29.0603f, 22.4557f};
    float ReverseRpmPerKmh = 121.5251f;
};

class FPinkCabGearboxController
{
public:
    explicit FPinkCabGearboxController(const FPinkCabGearboxControllerConfig& InConfig = {})
        : Config(InConfig)
    {
    }

    int32 GetRequestedGear() const { return RequestedGear; }
    int32 GetEngagedGear() const { return EngagedGear; }
    EPinkCabGearEngagementResult GetLastResult() const { return LastResult; }
    float GetExpectedCoupledRpm() const { return ExpectedCoupledRpm; }
    float GetMaxSafeEngineRpm() const { return Config.MaxSafeEngineRpm; }
    uint32 GetEventSerial() const { return EventSerial; }

    float ComputeClutchCoupling(float ClutchPedal) const
    {
        const float Released = FMath::Clamp(1.0f - ClutchPedal, 0.0f, 1.0f);
        return FMath::Pow(Released, FMath::Max(Config.ClutchCurveExponent, 1.0f));
    }

    float ExpectedEngineRpmForGear(int32 Gear, float SpeedKmh) const
    {
        if (Gear == 0)
        {
            return Config.IdleRpm;
        }

        float RpmPerKmh = Config.ReverseRpmPerKmh;
        if (Gear > 0 && Gear <= 5)
        {
            RpmPerKmh = Config.RpmPerKmh[Gear];
        }

        return FMath::Max(Config.IdleRpm, FMath::Abs(SpeedKmh) * RpmPerKmh);
    }
    bool MoveGate(float X, float Y)
    {
        const int32 NewRow = Y > 0.35f ? 1 : (Y < -0.35f ? -1 : 0);
        const int32 Column = X < -0.33f ? 0 : (X > 0.33f ? 2 : 1);
        if (LastGateRow != 0 && NewRow != 0
            && (NewRow != LastGateRow || Column != LastGateColumn))
        {
            return false;
        }

        if (NewRow == 0)
        {
            RequestedGear = 0;
            LastGateRow = 0;
            LastGateColumn = Column;
            return true;
        }

        if (NewRow > 0)
        {
            RequestedGear = Column == 0 ? 1 : (Column == 1 ? 3 : 5);
        }
        else
        {
            RequestedGear = Column == 0 ? 2 : (Column == 1 ? 4 : -1);
        }
        LastGateRow = NewRow;
        LastGateColumn = Column;
        return true;
    }

    bool ApplyLeverMouseDelta(float MouseDeltaX, float MouseDeltaY)
    {
        constexpr float CountsX = 160.0f;
        constexpr float CountsY = 140.0f;
        const float GateDx = MouseDeltaX / CountsX;
        // UE mouse Y is screen-space here: moving the physical mouse away/up
        // produces a negative delta. Convert exactly once so the top H row
        // (1/3/5) remains logical +Y and the bottom row is 2/4/R.
        const float GateDy = -MouseDeltaY / CountsY;
        const int32 Steps = FMath::Max(
            1,
            FMath::CeilToInt(FMath::Max(FMath::Abs(GateDx), FMath::Abs(GateDy)) / 0.20f));
        const float StepX = GateDx / static_cast<float>(Steps);
        const float StepY = GateDy / static_cast<float>(Steps);
        const int32 Before = RequestedGear;

        for (int32 Index = 0; Index < Steps; ++Index)
        {
            if (LastGateRow != 0)
            {
                const float CandidateY = FMath::Clamp(LeverY + StepY, -1.0f, 1.0f);
                if (MoveGate(LeverX, CandidateY))
                {
                    LeverY = CandidateY;
                }
                if (LastGateRow != 0)
                {
                    continue;
                }
            }

            const float CandidateX = FMath::Clamp(LeverX + StepX, -1.0f, 1.0f);
            if (MoveGate(CandidateX, LeverY))
            {
                LeverX = CandidateX;
            }

            const float CandidateY = FMath::Clamp(LeverY + StepY, -1.0f, 1.0f);
            if (MoveGate(LeverX, CandidateY))
            {
                LeverY = CandidateY;
            }
        }
        return RequestedGear != Before;
    }

    void CancelPendingRequest()
    {
        RequestedGear = EngagedGear;
        LastGateRow = EngagedGear == 0 ? 0
            : (EngagedGear == 1 || EngagedGear == 3 || EngagedGear == 5 ? 1 : -1);
        LastGateColumn = GearColumn(EngagedGear);
        LeverX = LastGateColumn == 0 ? -1.0f : (LastGateColumn == 2 ? 1.0f : 0.0f);
        LeverY = LastGateRow > 0 ? 1.0f : (LastGateRow < 0 ? -1.0f : 0.0f);
    }

    float GetLeverX() const { return LeverX; }
    float GetLeverY() const { return LeverY; }

    bool RequestGear(int32 Gear, const FPinkCabGearEngagementContext& Context)
    {
        if (Gear < -1 || Gear > 5)
        {
            return false;
        }

        RequestedGear = Gear;
        ++EventSerial;
        if (Gear == 0)
        {
            EngagedGear = 0;
            ExpectedCoupledRpm = Config.IdleRpm;
            LastResult = EPinkCabGearEngagementResult::Neutral;
            return true;
        }

        ExpectedCoupledRpm = ExpectedEngineRpmForGear(Gear, Context.SpeedKmh);

        if (Gear == -1 && FMath::Abs(Context.SpeedKmh) > Config.ReverseLockoutSpeedKmh)
        {
            LastResult = EPinkCabGearEngagementResult::ReverseLockout;
            return false;
        }

        if (Context.ClutchPedal >= Config.ClutchDisengagedThreshold)
        {
            EngagedGear = Gear;
            LastResult = EPinkCabGearEngagementResult::ClutchDisengagedAccepted;
            return true;
        }
        const float RpmMismatch = FMath::Abs(Context.EngineRpm - ExpectedCoupledRpm);
        const float AppliedLoad = FMath::Max(FMath::Abs(Context.Throttle), FMath::Abs(Context.Brake));
        const float HealthPenalty = FMath::Lerp(0.65f, 1.0f, FMath::Clamp(Context.GearboxHealth, 0.0f, 1.0f));
        const float AllowedMismatch = Config.ClutchlessMatchToleranceRpm * HealthPenalty;

        if (AppliedLoad <= Config.ClutchlessLoadThreshold && RpmMismatch <= AllowedMismatch)
        {
            EngagedGear = Gear;
            LastResult = EPinkCabGearEngagementResult::MatchedClutchlessAccepted;
            return true;
        }

        LastResult = EPinkCabGearEngagementResult::GrindRefused;
        return false;
    }

    bool RequestGearByDelta(int32 Delta, const FPinkCabGearEngagementContext& Context)
    {
        if (Delta == 0)
        {
            return false;
        }
        const int32 Candidate = FMath::Clamp(RequestedGear + Delta, -1, 5);
        return RequestGear(Candidate, Context);
    }

    EPinkCabGearEngagementResult EvaluateCurrentEngagement(
        const FPinkCabGearEngagementContext& Context)
    {
        if (RequestedGear != EngagedGear)
        {
            const float PendingRpm = ExpectedEngineRpmForGear(RequestedGear, Context.SpeedKmh);
            const float RpmMismatch = FMath::Abs(Context.EngineRpm - PendingRpm);
            const float AppliedLoad = FMath::Max(FMath::Abs(Context.Throttle), FMath::Abs(Context.Brake));
            const float HealthPenalty = FMath::Lerp(
                0.65f, 1.0f, FMath::Clamp(Context.GearboxHealth, 0.0f, 1.0f));
            const bool bCanRetry =
                Context.ClutchPedal >= Config.ClutchDisengagedThreshold
                || (AppliedLoad <= Config.ClutchlessLoadThreshold
                    && RpmMismatch <= Config.ClutchlessMatchToleranceRpm * HealthPenalty);
            if (bCanRetry)
            {
                RequestGear(RequestedGear, Context);
            }
            return LastResult;
        }

        if (EngagedGear == 0)
        {
            ExpectedCoupledRpm = Config.IdleRpm;
            LastResult = EPinkCabGearEngagementResult::Neutral;
            return LastResult;
        }

        ExpectedCoupledRpm = ExpectedEngineRpmForGear(EngagedGear, Context.SpeedKmh);
        const float Coupling = ComputeClutchCoupling(Context.ClutchPedal);
        if (Coupling > 0.75f && ExpectedCoupledRpm > Config.MaxSafeEngineRpm)
        {
            if (LastResult != EPinkCabGearEngagementResult::DangerousOverrev)
            {
                ++EventSerial;
            }
            LastResult = EPinkCabGearEngagementResult::DangerousOverrev;
        }
        return LastResult;
    }

    void ForceState(int32 InRequestedGear, int32 InEngagedGear)
    {
        RequestedGear = FMath::Clamp(InRequestedGear, -1, 5);
        EngagedGear = FMath::Clamp(InEngagedGear, -1, 5);
        LastGateRow = RequestedGear == 0 ? 0
            : (RequestedGear == 1 || RequestedGear == 3 || RequestedGear == 5 ? 1 : -1);
        LastGateColumn = GearColumn(RequestedGear);
        LeverX = LastGateColumn == 0 ? -1.0f : (LastGateColumn == 2 ? 1.0f : 0.0f);
        LeverY = LastGateRow > 0 ? 1.0f : (LastGateRow < 0 ? -1.0f : 0.0f);
    }

private:
    static int32 GearColumn(int32 Gear)
    {
        if (Gear == 1 || Gear == 2) return 0;
        if (Gear == 5 || Gear == -1) return 2;
        return 1;
    }
    FPinkCabGearboxControllerConfig Config;
    int32 RequestedGear = 0;
    int32 EngagedGear = 0;
    int32 LastGateRow = 0;
    int32 LastGateColumn = 1;
    float LeverX = 0.0f;
    float LeverY = 0.0f;
    float ExpectedCoupledRpm = 850.0f;
    uint32 EventSerial = 0;
    EPinkCabGearEngagementResult LastResult = EPinkCabGearEngagementResult::None;
};
