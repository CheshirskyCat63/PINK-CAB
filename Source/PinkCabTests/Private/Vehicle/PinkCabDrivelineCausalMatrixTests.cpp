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
    float ResetBodyLinearSpeedCmPerSec = 0.0f;
    float ResetBodyAngularSpeedDegPerSec = 0.0f;
    int64 ResetMechanicalSteps = 0;
    int32 ResetStableMechanicalSteps = 0;
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
        UPinkCabChaosVehicleMovementComponent* PinkCabMovement =
            Cast<UPinkCabChaosVehicleMovementComponent>(Movement);
        Test->TestNotNull(
            TEXT("D3 runtime uses authoritative PinkCab custom movement"),
            PinkCabMovement);
        if (!Movement || !Mesh || !PinkCabMovement)
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
            ++ResetPollCount;
            const bool bPhysicallySettled =
                RestGate.Update(*Pawn);
            const FPinkCabPhysicsFixtureRestObservation& Rest =
                RestGate.GetObservation();

            if (bPhysicallySettled)
            {
                CurrentResetEngineRpm = Rest.EngineRpm;
                CurrentResetMaxDrivenWheelRpm =
                    Rest.MaxDrivenWheelRpm;
                CurrentResetBodyLinearSpeedCmPerSec =
                    Rest.BodyLinearSpeedCmPerSec;
                CurrentResetBodyAngularSpeedDegPerSec =
                    Rest.BodyAngularSpeedDegPerSec;
                CurrentResetMechanicalSteps =
                    Rest.ElapsedMechanicalSteps;
                CurrentResetStableMechanicalSteps =
                    RestGate.GetStableMechanicalSteps();
                LastMechanicalStep = Rest.MechanicalStep;

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
                SettleMechanicalStepsRemaining = SettleMechanicalSteps;
                bResetting = false;
                return false;
            }

            if (Rest.ElapsedMechanicalSteps
                    < ResetTimeoutMechanicalSteps
                && ResetPollCount < ResetPollLimit)
            {
                return false;
            }

            Test->AddError(FString::Printf(
                TEXT("D3 fixture physical rest not observed coupling=%.3f gear=%d throttle=%.2f repeat=%d engine_rpm=%.3f target_idle_rpm=%.3f max_driven_wheel_rpm=%.3f body_linear_cm_s=%.3f body_angular_deg_s=%.3f chaos_current=%d chaos_target=%d mechanical_steps=%lld stable_steps=%d polls=%d"),
                CurrentCoupling(),
                CurrentGear(),
                CurrentThrottle(),
                RepeatIndex + 1,
                Rest.EngineRpm,
                Rest.TargetIdleRpm,
                Rest.MaxDrivenWheelRpm,
                Rest.BodyLinearSpeedCmPerSec,
                Rest.BodyAngularSpeedDegPerSec,
                Rest.NativeCurrentGear,
                Rest.NativeTargetGear,
                static_cast<long long>(Rest.ElapsedMechanicalSteps),
                RestGate.GetStableMechanicalSteps(),
                ResetPollCount));
            return true;
        }

        const int64 MechanicalStep =
            PinkCabMovement->GetPinkCabMechanicalIntegrationStepCount();
        if (MechanicalStep == LastMechanicalStep)
        {
            return false;
        }

        const int64 MechanicalStepDelta = MechanicalStep - LastMechanicalStep;
        if (MechanicalStepDelta != 1)
        {
            Test->AddError(FString::Printf(
                TEXT("D3 lost mechanical-step alignment coupling=%.3f gear=%d throttle=%.2f repeat=%d previous_step=%lld current_step=%lld delta=%lld"),
                CurrentCoupling(),
                CurrentGear(),
                CurrentThrottle(),
                RepeatIndex + 1,
                static_cast<long long>(LastMechanicalStep),
                static_cast<long long>(MechanicalStep),
                static_cast<long long>(MechanicalStepDelta)));
            return true;
        }
        LastMechanicalStep = MechanicalStep;

        const FPinkCabCausalActuationTelemetry& Actuation =
            Provider.GetLastCausalActuationTelemetry();
        LastEffectiveGearRatio = Actuation.EffectiveGearRatio;

        // D3 is a causal matrix, not a wall-clock benchmark. A sample is
        // admitted only after exactly one real ProcessMechanicalSimulation()
        // integration. Scheduler delay can therefore make the test fail loud,
        // but can never silently change the physical-step cardinality.
        if (SettleMechanicalStepsRemaining > 0)
        {
            --SettleMechanicalStepsRemaining;
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
        Run.ResetBodyLinearSpeedCmPerSec =
            CurrentResetBodyLinearSpeedCmPerSec;
        Run.ResetBodyAngularSpeedDegPerSec =
            CurrentResetBodyAngularSpeedDegPerSec;
        Run.ResetMechanicalSteps = CurrentResetMechanicalSteps;
        Run.ResetStableMechanicalSteps =
            CurrentResetStableMechanicalSteps;
        Runs.Add(Run);

        Test->AddInfo(FString::Printf(
            TEXT("P02_D3_MATRIX coupling=%.3f gear=%d throttle=%.2f repeat=%d reset_engine_rpm=%.3f reset_max_driven_wheel_rpm=%.3f reset_body_linear_cm_s=%.3f reset_body_angular_deg_s=%.3f reset_mechanical_steps=%lld reset_stable_steps=%d rear_torque_nm=%.3f engine_rpm=%.3f effective_ratio=%.6f resolved_throttle=%.6f available_engine_torque_nm=%.3f chaos_current=%d chaos_target=%d"),
            Run.Coupling,
            Run.Gear,
            Run.DriverThrottle01,
            Run.Repeat + 1,
            Run.ResetEngineRpm,
            Run.ResetMaxDrivenWheelRpm,
            Run.ResetBodyLinearSpeedCmPerSec,
            Run.ResetBodyAngularSpeedDegPerSec,
            static_cast<long long>(Run.ResetMechanicalSteps),
            Run.ResetStableMechanicalSteps,
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
        // The fixture spawns above the floor. Hold the ordinary service brake
        // during the reset/landing phase so the chassis can reach a real,
        // repeatable stationary state instead of rolling indefinitely from
        // suspension/contact transients. The brake is released when the test
        // condition is applied; no hidden force is injected.
        Controls.SetBrake(1.0f);
        Controls.SetHandbrake(0.0f);
        Controls.SetDriveline(0, 0, 0.0f);
        Controls.SetDrivetrainTorqueCapacity(1.0f);

        RearTorqueSum = 0.0f;
        EngineRpmSum = 0.0f;
        LastEffectiveGearRatio = 0.0f;
        CurrentResetEngineRpm = 0.0f;
        CurrentResetMaxDrivenWheelRpm = 0.0f;
        CurrentResetBodyLinearSpeedCmPerSec = 0.0f;
        CurrentResetBodyAngularSpeedDegPerSec = 0.0f;
        CurrentResetMechanicalSteps = 0;
        CurrentResetStableMechanicalSteps = 0;
        SampleCount = 0;
        SettleMechanicalStepsRemaining = 0;
        LastMechanicalStep = -1;
        ResetPollCount = 0;
        RestGate.Reset();
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
            Test->TestTrue(TEXT("every D3 repeat starts from settled chassis linear state"),
                Run.ResetBodyLinearSpeedCmPerSec <= ResetBodyLinearToleranceCmPerSec);
            Test->TestTrue(TEXT("every D3 repeat starts from settled chassis angular state"),
                Run.ResetBodyAngularSpeedDegPerSec <= ResetBodyAngularToleranceDegPerSec);
            Test->TestTrue(TEXT("every D3 repeat proves a physical settle window"),
                Run.ResetMechanicalSteps >= MinimumResetMechanicalSteps
                    && Run.ResetStableMechanicalSteps >= MinimumStableResetMechanicalSteps);
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
    static constexpr int32 SettleMechanicalSteps = 3;
    static constexpr int32 SampleFrames = 12;
    static constexpr float OpenTorqueToleranceNm = 1.0f;
    static constexpr float MonotonicToleranceNm = 50.0f;
    static constexpr float FullBoundaryRelativeTolerance = 0.05f;
    static constexpr float FirstReverseRelativeTolerance = 0.08f;
    static constexpr float ResetEngineRpmTolerance = 30.0f;
    static constexpr float ResetWheelRpmTolerance = 2.0f;
    static constexpr float ResetBodyLinearToleranceCmPerSec = 5.0f;
    static constexpr float ResetBodyAngularToleranceDegPerSec = 2.0f;
    static constexpr int64 MinimumResetMechanicalSteps = 20;
    static constexpr int32 MinimumStableResetMechanicalSteps = 5;
    static constexpr int64 ResetTimeoutMechanicalSteps = 240;
    static constexpr int32 ResetPollLimit = 2400;

    int32 CouplingIndex = 0;
    int32 GearIndex = 0;
    int32 ThrottleIndex = 0;
    int32 RepeatIndex = 0;
    bool bInitialized = false;
    bool bResetting = false;
    int32 SampleCount = 0;
    int32 SettleMechanicalStepsRemaining = 0;
    int64 LastMechanicalStep = -1;
    int32 ResetPollCount = 0;
    FPinkCabPhysicsFixtureRestGate RestGate;
    float RearTorqueSum = 0.0f;
    float EngineRpmSum = 0.0f;
    float LastEffectiveGearRatio = 0.0f;
    float CurrentResetEngineRpm = 0.0f;
    float CurrentResetMaxDrivenWheelRpm = 0.0f;
    float CurrentResetBodyLinearSpeedCmPerSec = 0.0f;
    float CurrentResetBodyAngularSpeedDegPerSec = 0.0f;
    int64 CurrentResetMechanicalSteps = 0;
    int32 CurrentResetStableMechanicalSteps = 0;
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
