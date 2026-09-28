#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "EngineUtils.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "ChaosVehicleWheel.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Vehicle/PinkCabPhysicsFixturePawn.h"
#include "Vehicle/PinkCabChaosCockpitBridge.h"
#include "Vehicle/PinkCabChaosVehicleDynamicsProvider.h"
#include "Vehicle/PinkCabChaosVehicleMovementComponent.h"
#include "Vehicle/PinkCabCockpitState.h"
#include "Vehicle/PinkCabVehicleControlState.h"

namespace
{
struct FPinkCabD3Run
{
    float Coupling = 0.0f;
    int32 Gear = 0;
    float DriverThrottle01 = 0.0f;
    int32 Repeat = 0;
    float MeanRearDriveTorqueNm = 0.0f;
    float MeanEngineRpm = 0.0f;
    float EffectiveGearRatio = 0.0f;
    float ResolvedEngineThrottle01 = 0.0f;
    float AvailableEngineTorqueNm = 0.0f;
    int32 ChaosCurrentGear = 0;
    int32 ChaosTargetGear = 0;
    float ResetEngineRpm = 0.0f;
    float ResetMaxDrivenWheelRpm = 0.0f;
};

float MedianD3(TArray<float> Values)
{
    Values.Sort();
    if (Values.Num() == 0)
    {
        return 0.0f;
    }
    const int32 Middle = Values.Num() / 2;
    return Values.Num() % 2 == 0
        ? 0.5f * (Values[Middle - 1] + Values[Middle])
        : Values[Middle];
}

float RelativeStepD3(const float A, const float B, const float Floor)
{
    const float Denominator = FMath::Max3(FMath::Abs(A), FMath::Abs(B), Floor);
    return FMath::Abs(A - B) / Denominator;
}
}

class FPinkCabD3CausalMatrixCommand final : public IAutomationLatentCommand
{
public:
    explicit FPinkCabD3CausalMatrixCommand(FAutomationTestBase* InTest)
        : Test(InTest)
    {
    }

    virtual bool Update() override
    {
        UWorld* World = AutomationCommon::GetAnyGameWorld();
        if (!World)
        {
            return false;
        }

        AActor* FixtureFloor =
            PinkCabPhysicsFixture::FindOrSpawnFlatFloor(*World);
        APinkCabPhysicsFixturePawn* Pawn =
            PinkCabPhysicsFixture::FindOrSpawnPawn(*World);
        if (!FixtureFloor || !Pawn)
        {
            Test->AddError(TEXT("sterile physics fixture failed to spawn"));
            return true;
        }
        PinkCabPhysicsFixture::KeepAwake(*Pawn);

        UChaosWheeledVehicleMovementComponent* Movement = Pawn->GetChaosMovement();
        USkeletalMeshComponent* Mesh = Pawn->GetMesh();
        Test->TestNotNull(TEXT("D3 movement exists"), Movement);
        Test->TestNotNull(TEXT("D3 physics mesh exists"), Mesh);
        Test->TestNotNull(
            TEXT("D3 runtime uses authoritative PinkCab custom movement"),
            Cast<UPinkCabChaosVehicleMovementComponent>(Movement));
        if (!Movement || !Mesh)
        {
            return true;
        }

        if (!bInitialized)
        {
            UGameplayStatics::SetGamePaused(World, false);
            Test->TestTrue(TEXT("D3 starts semantic engine"), Cockpit.StartEngine());

            // Each matrix sample owns a fresh Chaos vehicle instance. Reusing a
            // live engine and trying to rewind it with SetSnapshot does not
            // reset all internal free-engine state and contaminated later cells.
            bInitialized = true;
            BeginFreshRun(*World);
            return false;
        }

        FPinkCabChaosVehicleDynamicsProvider Provider(Movement);
        if (!FPinkCabChaosCockpitBridge::Apply(
                Cockpit, *Movement, Controls, Provider))
        {
            Test->AddError(TEXT("D3 authoritative actuation refresh failed"));
            return true;
        }

        if (bResetting)
        {
            float MaxDrivenWheelRpm = 0.0f;
            for (const UChaosVehicleWheel* Wheel : Movement->Wheels)
            {
                if (!Wheel || !Wheel->bAffectedByEngine)
                {
                    continue;
                }
                MaxDrivenWheelRpm = FMath::Max(
                    MaxDrivenWheelRpm,
                    FMath::Abs(
                        Wheel->GetWheelAngularVelocity()
                        * (60.0f / (2.0f * PI))));
            }

            const float EngineRpm = Movement->GetEngineRotationSpeed();
            const bool bEngineResetObserved =
                FMath::Abs(EngineRpm - InitialEngineRpm)
                    <= ResetEngineRpmTolerance;
            const bool bWheelsResetObserved =
                MaxDrivenWheelRpm <= ResetWheelRpmTolerance;
            const bool bNativeGearNeutral =
                Movement->GetCurrentGear() == 0
                && Movement->GetTargetGear() == 0;

            if (bEngineResetObserved
                && bWheelsResetObserved
                && bNativeGearNeutral)
            {
                CurrentResetEngineRpm = EngineRpm;
                CurrentResetMaxDrivenWheelRpm = MaxDrivenWheelRpm;

                Controls = {};
                Controls.SetThrottle(CurrentThrottle());
                Controls.SetBrake(0.0f);
                Controls.SetHandbrake(0.0f);
                Controls.SetDriveline(
                    CurrentGear(), CurrentGear(), CurrentCoupling());
                Controls.SetDrivetrainTorqueCapacity(1.0f);

                FPinkCabChaosVehicleDynamicsProvider ConditionProvider(Movement);
                if (!FPinkCabChaosCockpitBridge::Apply(
                        Cockpit, *Movement, Controls, ConditionProvider))
                {
                    Test->AddError(
                        TEXT("D3 condition actuation after reset failed"));
                    return true;
                }

                RearTorqueSum = 0.0f;
                EngineRpmSum = 0.0f;
                LastEffectiveGearRatio =
                    ConditionProvider.GetLastCausalActuationTelemetry()
                        .EffectiveGearRatio;
                SampleCount = 0;
                SettleFramesRemaining = SettleFrames;
                bResetting = false;
                return false;
            }

            ++ResetFramesWaited;
            if (ResetFramesWaited < ResetTimeoutFrames)
            {
                return false;
            }

            Test->AddError(FString::Printf(
                TEXT("D3 fixture reset not observed coupling=%.3f gear=%d throttle=%.2f repeat=%d engine_rpm=%.3f max_driven_wheel_rpm=%.3f chaos_current=%d chaos_target=%d"),
                CurrentCoupling(),
                CurrentGear(),
                CurrentThrottle(),
                RepeatIndex + 1,
                EngineRpm,
                MaxDrivenWheelRpm,
                Movement->GetCurrentGear(),
                Movement->GetTargetGear()));
            return true;
        }

        const FPinkCabCausalActuationTelemetry& Actuation =
            Provider.GetLastCausalActuationTelemetry();
        LastEffectiveGearRatio = Actuation.EffectiveGearRatio;

        // D3 is a causal matrix, not a wall-clock benchmark. Sampling by
        // elapsed real time made identical cells observe different counts of
        // Chaos steps on a busy self-hosted runner. Use a fixed number of
        // automation/physics observations instead so every cell has identical
        // measurement cardinality; D5 separately validates 30/60/120 FPS.
        if (SettleFramesRemaining > 0)
        {
            --SettleFramesRemaining;
            return false;
        }

        const FWheelStatus RearLeft = Movement->GetWheelState(2);
        const FWheelStatus RearRight = Movement->GetWheelState(3);
        RearTorqueSum += 0.5f
            * (FMath::Abs(RearLeft.DriveTorque) + FMath::Abs(RearRight.DriveTorque));
        EngineRpmSum += Movement->GetEngineRotationSpeed();
        ++SampleCount;

        if (SampleCount < SampleFrames)
        {
            return false;
        }

        FPinkCabD3Run Run;
        Run.Coupling = CurrentCoupling();
        Run.Gear = CurrentGear();
        Run.DriverThrottle01 = CurrentThrottle();
        Run.Repeat = RepeatIndex;
        Run.MeanRearDriveTorqueNm =
            SampleCount > 0 ? RearTorqueSum / static_cast<float>(SampleCount) : 0.0f;
        Run.MeanEngineRpm =
            SampleCount > 0 ? EngineRpmSum / static_cast<float>(SampleCount) : 0.0f;
        Run.EffectiveGearRatio = LastEffectiveGearRatio;
        Run.ResolvedEngineThrottle01 = Controls.GetResolvedEngineThrottle01();
        Run.AvailableEngineTorqueNm = Controls.GetAvailableEngineTorqueNm();
        Run.ChaosCurrentGear = Movement->GetCurrentGear();
        Run.ChaosTargetGear = Movement->GetTargetGear();
        Run.ResetEngineRpm = CurrentResetEngineRpm;
        Run.ResetMaxDrivenWheelRpm = CurrentResetMaxDrivenWheelRpm;
        Runs.Add(Run);

        Test->AddInfo(FString::Printf(
            TEXT("P02_D3_MATRIX coupling=%.3f gear=%d throttle=%.2f repeat=%d reset_engine_rpm=%.3f reset_max_driven_wheel_rpm=%.3f rear_torque_nm=%.3f engine_rpm=%.3f effective_ratio=%.6f resolved_throttle=%.6f available_engine_torque_nm=%.3f chaos_current=%d chaos_target=%d"),
            Run.Coupling,
            Run.Gear,
            Run.DriverThrottle01,
            Run.Repeat + 1,
            Run.ResetEngineRpm,
            Run.ResetMaxDrivenWheelRpm,
            Run.MeanRearDriveTorqueNm,
            Run.MeanEngineRpm,
            Run.EffectiveGearRatio,
            Run.ResolvedEngineThrottle01,
            Run.AvailableEngineTorqueNm,
            Run.ChaosCurrentGear,
            Run.ChaosTargetGear));

        AdvanceCondition();
        if (!IsFinished())
        {
            BeginFreshRun(*World);
            return false;
        }

        return Evaluate();
    }

private:
    float CurrentCoupling() const { return Couplings[CouplingIndex]; }
    int32 CurrentGear() const { return Gears[GearIndex]; }
    float CurrentThrottle() const { return Throttles[ThrottleIndex]; }

    bool IsFinished() const
    {
        return CouplingIndex >= Couplings.Num();
    }

    void AdvanceCondition()
    {
        ++RepeatIndex;
        if (RepeatIndex < RepeatsPerCondition)
        {
            return;
        }

        RepeatIndex = 0;
        ++ThrottleIndex;
        if (ThrottleIndex < Throttles.Num())
        {
            return;
        }

        ThrottleIndex = 0;
        ++GearIndex;
        if (GearIndex < Gears.Num())
        {
            return;
        }

        GearIndex = 0;
        ++CouplingIndex;
    }

    void BeginFreshRun(UWorld& World)
    {
        // Destroy the previous physics vehicle instead of attempting to rewind
        // hidden Chaos engine state. The next latent tick spawns a completely
        // new production movement/profile/wheel stack for this matrix sample.
        PinkCabPhysicsFixture::DestroyPawns(World);

        Controls = {};
        Controls.SetThrottle(0.0f);
        Controls.SetBrake(0.0f);
        Controls.SetHandbrake(0.0f);
        Controls.SetDriveline(0, 0, 0.0f);
        Controls.SetDrivetrainTorqueCapacity(1.0f);

        RearTorqueSum = 0.0f;
        EngineRpmSum = 0.0f;
        LastEffectiveGearRatio = 0.0f;
        CurrentResetEngineRpm = 0.0f;
        CurrentResetMaxDrivenWheelRpm = 0.0f;
        SampleCount = 0;
        SettleFramesRemaining = 0;
        ResetFramesWaited = 0;
        bResetting = true;
    }

    TArray<float> ValuesFor(
        const int32 Gear,
        const float Throttle,
        const float Coupling) const
    {
        TArray<float> Values;
        for (const FPinkCabD3Run& Run : Runs)
        {
            if (Run.Gear == Gear
                && FMath::IsNearlyEqual(Run.DriverThrottle01, Throttle, 1.0e-4f)
                && FMath::IsNearlyEqual(Run.Coupling, Coupling, 1.0e-4f))
            {
                Values.Add(Run.MeanRearDriveTorqueNm);
            }
        }
        return Values;
    }

    bool Evaluate()
    {
        Test->TestEqual(TEXT("D3 executes complete 240-run matrix"),
            Runs.Num(),
            Couplings.Num() * Gears.Num() * Throttles.Num() * RepeatsPerCondition);

        for (const FPinkCabD3Run& Run : Runs)
        {
            Test->TestEqual(TEXT("native Chaos current gear stays neutral"), Run.ChaosCurrentGear, 0);
            Test->TestEqual(TEXT("native Chaos target gear stays neutral"), Run.ChaosTargetGear, 0);
            Test->TestTrue(TEXT("first gear exposes positive PINK CAB effective ratio"),
                Run.Gear != 1 || Run.EffectiveGearRatio > KINDA_SMALL_NUMBER);
            Test->TestTrue(TEXT("reverse exposes negative PINK CAB effective ratio"),
                Run.Gear != -1 || Run.EffectiveGearRatio < -KINDA_SMALL_NUMBER);
            Test->TestTrue(TEXT("resolved throttle is bounded"),
                Run.ResolvedEngineThrottle01 >= 0.0f
                    && Run.ResolvedEngineThrottle01 <= 1.0f + KINDA_SMALL_NUMBER);
            Test->TestTrue(TEXT("available engine torque is non-negative"),
                Run.AvailableEngineTorqueNm >= 0.0f);
            Test->TestTrue(TEXT("every D3 repeat starts from proven engine RPM reset"),
                FMath::Abs(Run.ResetEngineRpm - InitialEngineRpm)
                    <= ResetEngineRpmTolerance);
            Test->TestTrue(TEXT("every D3 repeat starts from stopped driven wheels"),
                Run.ResetMaxDrivenWheelRpm <= ResetWheelRpmTolerance);
        }

        for (const int32 Gear : Gears)
        {
            for (const float Throttle : Throttles)
            {
                const float OpenMedian = MedianD3(
                    ValuesFor(Gear, Throttle, 0.0f));
                Test->TestTrue(TEXT("open clutch transfers no rear drive torque"),
                    OpenMedian <= OpenTorqueToleranceNm);

                TArray<float> PositiveCouplingMedians;
                for (int32 Index = 1; Index < Couplings.Num(); ++Index)
                {
                    const TArray<float> Values =
                        ValuesFor(Gear, Throttle, Couplings[Index]);
                    Test->TestEqual(TEXT("every D3 cell has five repeats"),
                        Values.Num(), RepeatsPerCondition);
                    PositiveCouplingMedians.Add(MedianD3(Values));
                }

                for (int32 Index = 1; Index < PositiveCouplingMedians.Num(); ++Index)
                {
                    Test->TestTrue(
                        TEXT("rear torque magnitude does not collapse as clutch coupling increases"),
                        PositiveCouplingMedians[Index] + MonotonicToleranceNm
                            >= PositiveCouplingMedians[Index - 1]);
                }

                const float NearFull = PositiveCouplingMedians[
                    PositiveCouplingMedians.Num() - 2];
                const float Full = PositiveCouplingMedians.Last();
                Test->TestTrue(TEXT("D3 0.999 to 1.000 torque remains continuous"),
                    RelativeStepD3(NearFull, Full, 25.0f) <= FullBoundaryRelativeTolerance);
            }
        }

        // 1st and R use mirrored ratios in the accepted profile. Their torque
        // magnitudes therefore should match closely for identical reset state.
        for (const float Coupling : Couplings)
        {
            for (const float Throttle : Throttles)
            {
                const float FirstMedian = MedianD3(ValuesFor(1, Throttle, Coupling));
                const float ReverseMedian = MedianD3(ValuesFor(-1, Throttle, Coupling));
                Test->TestTrue(TEXT("1st and reverse preserve mirrored torque magnitude"),
                    RelativeStepD3(FirstMedian, ReverseMedian, 25.0f)
                        <= FirstReverseRelativeTolerance);
            }
        }

        return true;
    }

    FAutomationTestBase* Test = nullptr;
    FPinkCabCockpitState Cockpit;
    FPinkCabVehicleControlState Controls;
    TArray<FPinkCabD3Run> Runs;

    const TArray<float> Couplings{0.0f, 0.25f, 0.50f, 0.75f, 0.999f, 1.0f};
    const TArray<int32> Gears{1, -1};
    const TArray<float> Throttles{0.0f, 0.25f, 0.50f, 1.0f};

    static constexpr int32 RepeatsPerCondition = 5;
    static constexpr float InitialEngineRpm = 925.0f;
    static constexpr int32 SettleFrames = 3;
    static constexpr int32 SampleFrames = 12;
    static constexpr float OpenTorqueToleranceNm = 1.0f;
    static constexpr float MonotonicToleranceNm = 50.0f;
    static constexpr float FullBoundaryRelativeTolerance = 0.05f;
    static constexpr float FirstReverseRelativeTolerance = 0.08f;
    static constexpr float ResetEngineRpmTolerance = 30.0f;
    static constexpr float ResetWheelRpmTolerance = 2.0f;
    static constexpr int32 ResetTimeoutFrames = 240;

    int32 CouplingIndex = 0;
    int32 GearIndex = 0;
    int32 ThrottleIndex = 0;
    int32 RepeatIndex = 0;
    bool bInitialized = false;
    bool bResetting = false;
    int32 SampleCount = 0;
    int32 SettleFramesRemaining = 0;
    int32 ResetFramesWaited = 0;
    float RearTorqueSum = 0.0f;
    float EngineRpmSum = 0.0f;
    float LastEffectiveGearRatio = 0.0f;
    float CurrentResetEngineRpm = 0.0f;
    float CurrentResetMaxDrivenWheelRpm = 0.0f;
};

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabD3CausalMatrixRuntimeTest,
    "PinkCab.Vehicle.Physics.P02.D3.CausalMatrix",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabD3CausalMatrixRuntimeTest::RunTest(const FString&)
{
    const bool bOpened = AutomationOpenMap(
        PinkCabPhysicsFixture::MapPath,
        true);
    TestTrue(TEXT("D3 runtime map opens"), bOpened);
    if (!bOpened)
    {
        return false;
    }

    ADD_LATENT_AUTOMATION_COMMAND(FPinkCabD3CausalMatrixCommand(this));
    return true;
}

#endif
