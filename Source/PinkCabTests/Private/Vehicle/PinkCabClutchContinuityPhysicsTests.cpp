#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "EngineUtils.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "ChaosVehicleWheel.h"
#include "ChaosVehicleManagerAsyncCallback.h"
#include "Components/SkeletalMeshComponent.h"
#include "HAL/PlatformTime.h"
#include "Kismet/GameplayStatics.h"
#include "Runtime/PinkCabChaosTatraPawn.h"
#include "SnapshotData.h"
#include "Vehicle/PinkCabChaosCockpitBridge.h"
#include "Vehicle/PinkCabChaosVehicleDynamicsProvider.h"
#include "Vehicle/PinkCabCockpitState.h"
#include "Vehicle/PinkCabVehicleControlState.h"

namespace
{
struct FPinkCabClutchBoundaryRun
{
    float Coupling = 0.0f;
    float MeanRearDriveTorqueNm = 0.0f;
    float MeanEngineRpm = 0.0f;
    float MeanChaosEngineTorqueNm = 0.0f;
    float MeanChaosTransmissionTorqueNm = 0.0f;
    float MeanChaosTransmissionRpm = 0.0f;
    float ResolvedEngineThrottle01 = 0.0f;
    float AuthoritativeAvailableEngineTorqueNm = 0.0f;
    double EndTranslationalKineticEnergyJ = 0.0;
    float EndSpeedCmPerSec = 0.0f;
};

float Median(TArray<float> Values)
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

double MedianDouble(TArray<double> Values)
{
    Values.Sort();
    if (Values.Num() == 0)
    {
        return 0.0;
    }
    const int32 Middle = Values.Num() / 2;
    return Values.Num() % 2 == 0
        ? 0.5 * (Values[Middle - 1] + Values[Middle])
        : Values[Middle];
}

float RelativeDelta(const double A, const double B, const double Floor)
{
    const double Denominator = FMath::Max3(FMath::Abs(A), FMath::Abs(B), Floor);
    return static_cast<float>(FMath::Abs(A - B) / Denominator);
}
}

class FPinkCabClutchBoundaryContinuityCommand final : public IAutomationLatentCommand
{
public:
    explicit FPinkCabClutchBoundaryContinuityCommand(FAutomationTestBase* InTest)
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

        APinkCabChaosTatraPawn* Pawn = nullptr;
        for (TActorIterator<APinkCabChaosTatraPawn> It(World); It; ++It)
        {
            Pawn = *It;
            break;
        }
        if (!Pawn)
        {
            return false;
        }

        UChaosWheeledVehicleMovementComponent* Movement = Pawn->GetChaosMovement();
        USkeletalMeshComponent* Mesh = Pawn->GetMesh();
        Test->TestNotNull(TEXT("P02 live Chaos movement exists"), Movement);
        Test->TestNotNull(TEXT("P02 live vehicle mesh exists"), Mesh);
        if (!Movement || !Mesh)
        {
            return true;
        }

        if (!bInitialized)
        {
            Pawn->SetSystemMenuOpen(false);
            UGameplayStatics::SetGamePaused(World, false);
            Pawn->SetActorTickEnabled(false);
            Mesh->WakeAllRigidBodies();

            Baseline = Movement->GetSnapshot();
            Baseline.LinearVelocity = FVector::ZeroVector;
            Baseline.AngularVelocity = FVector::ZeroVector;
            Baseline.EngineRPM = 2500.0f;
            Baseline.SelectedGear = 0;
            for (FWheelSnapshot& Wheel : Baseline.WheelSnapshots)
            {
                Wheel.WheelAngularVelocity = 0.0f;
            }

            Test->TestEqual(TEXT("P02 fixture has four wheel snapshots"),
                Baseline.WheelSnapshots.Num(), 4);
            Test->TestTrue(TEXT("P02 fixture starts local engine"), Cockpit.StartEngine());

            bInitialized = true;
            BeginRun(*Movement, *Mesh);
            return false;
        }

        FPinkCabChaosVehicleDynamicsProvider Provider(Movement);
        if (!FPinkCabChaosCockpitBridge::Apply(
                Cockpit, *Movement, Controls, Provider))
        {
            Test->AddError(TEXT("P02 clutch boundary actuation refresh failed"));
            return true;
        }

        const double Elapsed = FPlatformTime::Seconds() - RunStartSeconds;
        if (Elapsed < SettleSeconds)
        {
            return false;
        }

        const FWheelStatus RearLeft = Movement->GetWheelState(2);
        const FWheelStatus RearRight = Movement->GetWheelState(3);
        RearTorqueSum += 0.5f
            * (FMath::Abs(RearLeft.DriveTorque) + FMath::Abs(RearRight.DriveTorque));
        EngineRpmSum += Movement->GetEngineRotationSpeed();
        if (const TUniquePtr<FPhysicsVehicleOutput>& PhysicsOutput = Movement->PhysicsVehicleOutput())
        {
            if (PhysicsOutput.IsValid())
            {
                ChaosEngineTorqueSum += PhysicsOutput->EngineTorque;
                ChaosTransmissionTorqueSum += PhysicsOutput->TransmissionTorque;
                ChaosTransmissionRpmSum += PhysicsOutput->TransmissionRPM;
            }
        }
        ++SampleCount;

        if (Elapsed < RunSeconds)
        {
            return false;
        }

        FPinkCabClutchBoundaryRun Result;
        Result.Coupling = CurrentCoupling();
        Result.MeanRearDriveTorqueNm =
            SampleCount > 0 ? RearTorqueSum / static_cast<float>(SampleCount) : 0.0f;
        Result.MeanEngineRpm =
            SampleCount > 0 ? EngineRpmSum / static_cast<float>(SampleCount) : 0.0f;
        Result.MeanChaosEngineTorqueNm =
            SampleCount > 0 ? ChaosEngineTorqueSum / static_cast<float>(SampleCount) : 0.0f;
        Result.MeanChaosTransmissionTorqueNm =
            SampleCount > 0 ? ChaosTransmissionTorqueSum / static_cast<float>(SampleCount) : 0.0f;
        Result.MeanChaosTransmissionRpm =
            SampleCount > 0 ? ChaosTransmissionRpmSum / static_cast<float>(SampleCount) : 0.0f;
        Result.ResolvedEngineThrottle01 = Controls.GetResolvedEngineThrottle01();
        Result.AuthoritativeAvailableEngineTorqueNm = Controls.GetAvailableEngineTorqueNm();
        Result.EndSpeedCmPerSec = Mesh->GetPhysicsLinearVelocity().Size2D();
        const double SpeedMps = Result.EndSpeedCmPerSec * 0.01;
        Result.EndTranslationalKineticEnergyJ =
            0.5 * static_cast<double>(Movement->Mass) * SpeedMps * SpeedMps;
        Runs.Add(Result);

        Test->AddInfo(FString::Printf(
            TEXT("P02_PHY009_BOUNDARY coupling=%.3f repeat=%d mean_rear_drive_torque_nm=%.3f mean_engine_rpm=%.3f resolved_throttle=%.6f authoritative_available_engine_torque_nm=%.3f chaos_engine_torque_nm=%.3f chaos_transmission_torque_nm=%.3f chaos_transmission_rpm=%.3f end_speed_cm_s=%.3f end_ke_j=%.3f"),
            Result.Coupling,
            RepeatIndex + 1,
            Result.MeanRearDriveTorqueNm,
            Result.MeanEngineRpm,
            Result.ResolvedEngineThrottle01,
            Result.AuthoritativeAvailableEngineTorqueNm,
            Result.MeanChaosEngineTorqueNm,
            Result.MeanChaosTransmissionTorqueNm,
            Result.MeanChaosTransmissionRpm,
            Result.EndSpeedCmPerSec,
            Result.EndTranslationalKineticEnergyJ));

        ++RepeatIndex;
        if (RepeatIndex >= RepeatsPerCondition)
        {
            RepeatIndex = 0;
            ++ConditionIndex;
        }

        if (ConditionIndex < Couplings.Num())
        {
            BeginRun(*Movement, *Mesh);
            return false;
        }

        return EvaluateBoundary();
    }

private:
    float CurrentCoupling() const
    {
        return Couplings[FMath::Clamp(ConditionIndex, 0, Couplings.Num() - 1)];
    }

    void BeginRun(
        UChaosWheeledVehicleMovementComponent& Movement,
        USkeletalMeshComponent& Mesh)
    {
        Movement.SetSnapshot(Baseline);
        Mesh.WakeAllRigidBodies();

        Controls = {};
        Controls.SetThrottle(0.50f);
        Controls.SetBrake(0.0f);
        Controls.SetHandbrake(0.0f);
        Controls.SetDriveline(1, 1, CurrentCoupling());
        Controls.SetDrivetrainTorqueCapacity(1.0f);

        FPinkCabChaosVehicleDynamicsProvider Provider(&Movement);
        if (!FPinkCabChaosCockpitBridge::Apply(
                Cockpit, Movement, Controls, Provider))
        {
            Test->AddError(TEXT("P02 clutch boundary initial actuation failed"));
        }

        RearTorqueSum = 0.0f;
        EngineRpmSum = 0.0f;
        ChaosEngineTorqueSum = 0.0f;
        ChaosTransmissionTorqueSum = 0.0f;
        ChaosTransmissionRpmSum = 0.0f;
        SampleCount = 0;
        RunStartSeconds = FPlatformTime::Seconds();
    }

    bool EvaluateBoundary()
    {
        TArray<float> PartialTorque;
        TArray<float> FullTorque;
        TArray<float> PartialRpm;
        TArray<float> FullRpm;
        TArray<double> PartialEnergy;
        TArray<double> FullEnergy;

        for (const FPinkCabClutchBoundaryRun& Run : Runs)
        {
            TArray<float>& Torque = Run.Coupling < 1.0f ? PartialTorque : FullTorque;
            TArray<float>& Rpm = Run.Coupling < 1.0f ? PartialRpm : FullRpm;
            TArray<double>& Energy = Run.Coupling < 1.0f ? PartialEnergy : FullEnergy;
            Torque.Add(Run.MeanRearDriveTorqueNm);
            Rpm.Add(Run.MeanEngineRpm);
            Energy.Add(Run.EndTranslationalKineticEnergyJ);
        }

        const float PartialTorqueMedian = Median(PartialTorque);
        const float FullTorqueMedian = Median(FullTorque);
        const float PartialRpmMedian = Median(PartialRpm);
        const float FullRpmMedian = Median(FullRpm);
        const double PartialEnergyMedian = MedianDouble(PartialEnergy);
        const double FullEnergyMedian = MedianDouble(FullEnergy);

        const float TorqueDelta = RelativeDelta(
            PartialTorqueMedian, FullTorqueMedian, 25.0);
        const float RpmDelta = RelativeDelta(
            PartialRpmMedian, FullRpmMedian, 250.0);
        const float EnergyDelta = RelativeDelta(
            PartialEnergyMedian, FullEnergyMedian, 250.0);

        Test->AddInfo(FString::Printf(
            TEXT("P02_PHY009_BOUNDARY_MEDIAN partial_torque=%.3f full_torque=%.3f torque_delta=%.6f partial_rpm=%.3f full_rpm=%.3f rpm_delta=%.6f partial_ke=%.3f full_ke=%.3f ke_delta=%.6f"),
            PartialTorqueMedian,
            FullTorqueMedian,
            TorqueDelta,
            PartialRpmMedian,
            FullRpmMedian,
            RpmDelta,
            PartialEnergyMedian,
            FullEnergyMedian,
            EnergyDelta));

        // PHY-009 predeclared RED criterion: a 0.1% clutch-state difference
        // may not cause more than a 10% median step in delivered rear-wheel
        // torque, engine RPM response, or resulting translational energy.
        // Five identical-reset repeats per condition are required.
        Test->TestEqual(TEXT("P02 has five 0.999 repeats"),
            PartialTorque.Num(), RepeatsPerCondition);
        Test->TestEqual(TEXT("P02 has five 1.000 repeats"),
            FullTorque.Num(), RepeatsPerCondition);
        Test->TestTrue(TEXT("P02 0.999 path produces measurable rear torque"),
            PartialTorqueMedian > 25.0f);
        Test->TestTrue(TEXT("P02 1.000 path produces measurable rear torque"),
            FullTorqueMedian > 25.0f);
        Test->TestTrue(TEXT("PHY-009 rear-wheel torque is continuous across 0.999 to 1.000"),
            TorqueDelta <= MaxBoundaryStep);
        Test->TestTrue(TEXT("PHY-009 engine RPM response is continuous across 0.999 to 1.000"),
            RpmDelta <= MaxBoundaryStep);
        Test->TestTrue(TEXT("PHY-009 translational energy is continuous across 0.999 to 1.000"),
            EnergyDelta <= MaxBoundaryStep);
        return true;
    }

    FAutomationTestBase* Test = nullptr;
    FPinkCabCockpitState Cockpit;
    FPinkCabVehicleControlState Controls;
    FWheeledSnaphotData Baseline;
    TArray<FPinkCabClutchBoundaryRun> Runs;

    const TArray<float> Couplings{0.999f, 1.0f};
    static constexpr int32 RepeatsPerCondition = 5;
    static constexpr double SettleSeconds = 0.15;
    static constexpr double RunSeconds = 0.65;
    static constexpr float MaxBoundaryStep = 0.10f;

    bool bInitialized = false;
    int32 ConditionIndex = 0;
    int32 RepeatIndex = 0;
    int32 SampleCount = 0;
    float RearTorqueSum = 0.0f;
    float EngineRpmSum = 0.0f;
    float ChaosEngineTorqueSum = 0.0f;
    float ChaosTransmissionTorqueSum = 0.0f;
    float ChaosTransmissionRpmSum = 0.0f;
    double RunStartSeconds = 0.0;
};

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabClutchBoundaryContinuityRuntimeTest,
    "PinkCab.Vehicle.Physics.P02.ClutchBoundaryContinuity",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabClutchBoundaryContinuityRuntimeTest::RunTest(const FString& Parameters)
{
    const bool bOpened = AutomationOpenMap(
        TEXT("/Game/Dev/Maps/L_PinkCab_ChaosWeave"),
        true);
    TestTrue(TEXT("P02 vehicle runtime map opens"), bOpened);
    if (!bOpened)
    {
        return false;
    }

    ADD_LATENT_AUTOMATION_COMMAND(
        FPinkCabClutchBoundaryContinuityCommand(this));
    return true;
}


struct FPinkCabPartialClutchReactionRun
{
    float InitialSpeedKmh = 0.0f;
    float SeedWheelDerivedEngineRpm = 0.0f;
    float MeanEngineRpm = 0.0f;
    float MeanDrivenWheelRpm = 0.0f;
    float MeanWheelDerivedEngineRpm = 0.0f;
};

class FPinkCabPartialClutchWheelReactionCommand final : public IAutomationLatentCommand
{
public:
    explicit FPinkCabPartialClutchWheelReactionCommand(FAutomationTestBase* InTest)
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

        APinkCabChaosTatraPawn* Pawn = nullptr;
        for (TActorIterator<APinkCabChaosTatraPawn> It(World); It; ++It)
        {
            Pawn = *It;
            break;
        }
        if (!Pawn)
        {
            return false;
        }

        UChaosWheeledVehicleMovementComponent* Movement = Pawn->GetChaosMovement();
        USkeletalMeshComponent* Mesh = Pawn->GetMesh();
        Test->TestNotNull(TEXT("P02 reaction movement exists"), Movement);
        Test->TestNotNull(TEXT("P02 reaction mesh exists"), Mesh);
        if (!Movement || !Mesh)
        {
            return true;
        }

        if (!bInitialized)
        {
            Pawn->SetSystemMenuOpen(false);
            UGameplayStatics::SetGamePaused(World, false);
            Pawn->SetActorTickEnabled(false);
            Mesh->WakeAllRigidBodies();

            Baseline = Movement->GetSnapshot();
            Baseline.LinearVelocity = FVector::ZeroVector;
            Baseline.AngularVelocity = FVector::ZeroVector;
            Baseline.EngineRPM = InitialEngineRpm;
            Baseline.SelectedGear = 0;
            for (FWheelSnapshot& Wheel : Baseline.WheelSnapshots)
            {
                Wheel.WheelAngularVelocity = 0.0f;
            }

            Test->TestEqual(TEXT("P02 reaction fixture has four wheel snapshots"),
                Baseline.WheelSnapshots.Num(), 4);
            Test->TestTrue(TEXT("P02 reaction fixture starts local engine"),
                Cockpit.StartEngine());

            bInitialized = true;
            BeginRun(*Pawn, *Movement, *Mesh);
            return false;
        }

        FPinkCabChaosVehicleDynamicsProvider Provider(Movement);
        if (!FPinkCabChaosCockpitBridge::Apply(
                Cockpit, *Movement, Controls, Provider))
        {
            Test->AddError(TEXT("P02 partial-clutch reaction actuation refresh failed"));
            return true;
        }

        const double Elapsed = FPlatformTime::Seconds() - RunStartSeconds;
        if (Elapsed < SettleSeconds)
        {
            return false;
        }

        float DrivenWheelRpmSum = 0.0f;
        int32 DrivenWheelCount = 0;
        for (int32 WheelIndex = 0; WheelIndex < Movement->Wheels.Num(); ++WheelIndex)
        {
            const UChaosVehicleWheel* Wheel = Movement->Wheels[WheelIndex];
            if (!Wheel || !Wheel->bAffectedByEngine)
            {
                continue;
            }
            DrivenWheelRpmSum += FMath::Abs(
                Wheel->GetWheelAngularVelocity() * (60.0f / (2.0f * PI)));
            ++DrivenWheelCount;
        }

        if (DrivenWheelCount > 0)
        {
            const float MeanWheelRpm =
                DrivenWheelRpmSum / static_cast<float>(DrivenWheelCount);
            DrivenWheelRpmSumSamples += MeanWheelRpm;
            const float EffectiveRatio = FMath::Abs(
                Movement->TransmissionSetup.GetGearRatio(1));
            WheelDerivedEngineRpmSum += MeanWheelRpm * EffectiveRatio;
        }
        EngineRpmSum += Movement->GetEngineRotationSpeed();
        ++SampleCount;

        if (Elapsed < RunSeconds)
        {
            return false;
        }

        FPinkCabPartialClutchReactionRun Result;
        Result.InitialSpeedKmh = CurrentSpeedKmh();
        Result.SeedWheelDerivedEngineRpm =
            CurrentSeedWheelDerivedEngineRpm;
        Result.MeanEngineRpm =
            SampleCount > 0 ? EngineRpmSum / static_cast<float>(SampleCount) : 0.0f;
        Result.MeanDrivenWheelRpm =
            SampleCount > 0
                ? DrivenWheelRpmSumSamples / static_cast<float>(SampleCount)
                : 0.0f;
        Result.MeanWheelDerivedEngineRpm =
            SampleCount > 0
                ? WheelDerivedEngineRpmSum / static_cast<float>(SampleCount)
                : 0.0f;
        Runs.Add(Result);

        Test->AddInfo(FString::Printf(
            TEXT("P02_PHY009_REACTION speed_kmh=%.1f repeat=%d seed_wheel_derived_engine_rpm=%.3f engine_rpm=%.3f driven_wheel_rpm=%.3f measured_wheel_derived_engine_rpm=%.3f"),
            Result.InitialSpeedKmh,
            RepeatIndex + 1,
            Result.SeedWheelDerivedEngineRpm,
            Result.MeanEngineRpm,
            Result.MeanDrivenWheelRpm,
            Result.MeanWheelDerivedEngineRpm));

        ++RepeatIndex;
        if (RepeatIndex >= RepeatsPerCondition)
        {
            RepeatIndex = 0;
            ++ConditionIndex;
        }

        if (ConditionIndex < SpeedsKmh.Num())
        {
            BeginRun(*Pawn, *Movement, *Mesh);
            return false;
        }

        return EvaluateReaction();
    }

private:
    float CurrentSpeedKmh() const
    {
        return SpeedsKmh[FMath::Clamp(ConditionIndex, 0, SpeedsKmh.Num() - 1)];
    }

    void BeginRun(
        APinkCabChaosTatraPawn& Pawn,
        UChaosWheeledVehicleMovementComponent& Movement,
        USkeletalMeshComponent& Mesh)
    {
        FWheeledSnaphotData RunSnapshot = Baseline;
        const float SpeedMps = CurrentSpeedKmh() / 3.6f;
        const float SpeedCmPerSec = SpeedMps * 100.0f;

        // Build a physically consistent rolling state instead of injecting only
        // chassis velocity and waiting for Chaos to invent wheel speed. The
        // snapshot seeds every wheel from the authored runtime radius so the
        // low/high conditions begin with a real, deterministic shaft stimulus.
        float SeedDrivenWheelRpmSum = 0.0f;
        int32 SeedDrivenWheelCount = 0;
        for (int32 WheelIndex = 0;
             WheelIndex < RunSnapshot.WheelSnapshots.Num()
                 && WheelIndex < Movement.Wheels.Num();
             ++WheelIndex)
        {
            const UChaosVehicleWheel* Wheel = Movement.Wheels[WheelIndex];
            if (!Wheel)
            {
                continue;
            }
            const float RadiusM =
                FMath::Max(Wheel->WheelRadius * 0.01f, KINDA_SMALL_NUMBER);
            const float SeedAngularVelocityRadPerSec =
                SpeedMps / RadiusM;
            RunSnapshot.WheelSnapshots[WheelIndex].WheelAngularVelocity =
                SeedAngularVelocityRadPerSec;

            if (Wheel->bAffectedByEngine)
            {
                SeedDrivenWheelRpmSum +=
                    FMath::Abs(SeedAngularVelocityRadPerSec)
                    * (60.0f / (2.0f * PI));
                ++SeedDrivenWheelCount;
            }
        }

        const float SeedDrivenWheelRpm =
            SeedDrivenWheelCount > 0
                ? SeedDrivenWheelRpmSum
                    / static_cast<float>(SeedDrivenWheelCount)
                : 0.0f;
        const float EffectiveRatio = FMath::Abs(
            Movement.TransmissionSetup.GetGearRatio(1));
        CurrentSeedWheelDerivedEngineRpm =
            SeedDrivenWheelRpm * EffectiveRatio;

        Movement.SetSnapshot(RunSnapshot);
        Mesh.SetPhysicsLinearVelocity(
            Pawn.GetActorForwardVector() * SpeedCmPerSec);
        Mesh.SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
        Mesh.WakeAllRigidBodies();

        Controls = {};
        Controls.SetThrottle(0.0f);
        Controls.SetBrake(0.0f);
        Controls.SetHandbrake(0.0f);
        Controls.SetDriveline(1, 1, PartialCoupling);
        Controls.SetDrivetrainTorqueCapacity(1.0f);

        FPinkCabChaosVehicleDynamicsProvider Provider(&Movement);
        if (!FPinkCabChaosCockpitBridge::Apply(
                Cockpit, Movement, Controls, Provider))
        {
            Test->AddError(TEXT("P02 partial-clutch reaction initial actuation failed"));
        }

        EngineRpmSum = 0.0f;
        DrivenWheelRpmSumSamples = 0.0f;
        WheelDerivedEngineRpmSum = 0.0f;
        SampleCount = 0;
        RunStartSeconds = FPlatformTime::Seconds();
    }

    bool EvaluateReaction()
    {
        TArray<float> LowEngineRpm;
        TArray<float> HighEngineRpm;
        TArray<float> LowSeedWheelDerivedRpm;
        TArray<float> HighSeedWheelDerivedRpm;
        TArray<float> LowMeasuredWheelDerivedRpm;
        TArray<float> HighMeasuredWheelDerivedRpm;

        for (const FPinkCabPartialClutchReactionRun& Run : Runs)
        {
            const bool bLow = FMath::IsNearlyEqual(
                Run.InitialSpeedKmh, SpeedsKmh[0], KINDA_SMALL_NUMBER);
            (bLow ? LowEngineRpm : HighEngineRpm).Add(Run.MeanEngineRpm);
            (bLow ? LowSeedWheelDerivedRpm : HighSeedWheelDerivedRpm).Add(
                Run.SeedWheelDerivedEngineRpm);
            (bLow ? LowMeasuredWheelDerivedRpm : HighMeasuredWheelDerivedRpm).Add(
                Run.MeanWheelDerivedEngineRpm);
        }

        const float LowEngineMedian = Median(LowEngineRpm);
        const float HighEngineMedian = Median(HighEngineRpm);
        const float LowSeedWheelMedian = Median(LowSeedWheelDerivedRpm);
        const float HighSeedWheelMedian = Median(HighSeedWheelDerivedRpm);
        const float LowMeasuredWheelMedian = Median(LowMeasuredWheelDerivedRpm);
        const float HighMeasuredWheelMedian = Median(HighMeasuredWheelDerivedRpm);
        const float ShaftStimulusRpm =
            HighSeedWheelMedian - LowSeedWheelMedian;
        const float EngineReactionRpm = HighEngineMedian - LowEngineMedian;
        const float ReactionFraction = ShaftStimulusRpm > KINDA_SMALL_NUMBER
            ? EngineReactionRpm / ShaftStimulusRpm
            : 0.0f;

        Test->AddInfo(FString::Printf(
            TEXT("P02_PHY009_REACTION_MEDIAN low_engine_rpm=%.3f high_engine_rpm=%.3f low_seed_shaft_rpm=%.3f high_seed_shaft_rpm=%.3f low_measured_shaft_rpm=%.3f high_measured_shaft_rpm=%.3f shaft_stimulus_rpm=%.3f engine_reaction_rpm=%.3f reaction_fraction=%.6f"),
            LowEngineMedian,
            HighEngineMedian,
            LowSeedWheelMedian,
            HighSeedWheelMedian,
            LowMeasuredWheelMedian,
            HighMeasuredWheelMedian,
            ShaftStimulusRpm,
            EngineReactionRpm,
            ReactionFraction));

        Test->TestEqual(TEXT("P02 reaction has five low-speed repeats"),
            LowEngineRpm.Num(), RepeatsPerCondition);
        Test->TestEqual(TEXT("P02 reaction has five high-speed repeats"),
            HighEngineRpm.Num(), RepeatsPerCondition);
        Test->TestTrue(TEXT("P02 reaction fixture creates a meaningful shaft-speed stimulus"),
            ShaftStimulusRpm >= MinShaftStimulusRpm);

        // PHY-009 requires actual wheel->engine reaction at partial coupling.
        // With identical 3000-rpm engine state and zero throttle, a high-speed
        // driven axle must back-drive the engine measurably more than a low-speed
        // axle. 10% of the shaft stimulus is the predeclared diagnostic minimum;
        // this is a causality gate, not final clutch calibration.
        Test->TestTrue(TEXT("PHY-009 partial clutch transmits wheel-to-engine RPM reaction"),
            EngineReactionRpm >= MinEngineReactionRpm
                && ReactionFraction >= MinReactionFraction);
        return true;
    }

    FAutomationTestBase* Test = nullptr;
    FPinkCabCockpitState Cockpit;
    FPinkCabVehicleControlState Controls;
    FWheeledSnaphotData Baseline;
    TArray<FPinkCabPartialClutchReactionRun> Runs;

    const TArray<float> SpeedsKmh{10.0f, 50.0f};
    static constexpr int32 RepeatsPerCondition = 5;
    static constexpr float InitialEngineRpm = 3000.0f;
    static constexpr float PartialCoupling = 0.50f;
    static constexpr double SettleSeconds = 0.03;
    static constexpr double RunSeconds = 0.18;
    static constexpr float MinShaftStimulusRpm = 1500.0f;
    static constexpr float MinEngineReactionRpm = 300.0f;
    static constexpr float MinReactionFraction = 0.10f;

    bool bInitialized = false;
    int32 ConditionIndex = 0;
    int32 RepeatIndex = 0;
    int32 SampleCount = 0;
    float EngineRpmSum = 0.0f;
    float DrivenWheelRpmSumSamples = 0.0f;
    float WheelDerivedEngineRpmSum = 0.0f;
    float CurrentSeedWheelDerivedEngineRpm = 0.0f;
    double RunStartSeconds = 0.0;
};

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabPartialClutchWheelReactionRuntimeTest,
    "PinkCab.Vehicle.Physics.P02.PartialClutchWheelReaction",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabPartialClutchWheelReactionRuntimeTest::RunTest(const FString& Parameters)
{
    const bool bOpened = AutomationOpenMap(
        TEXT("/Game/Dev/Maps/L_PinkCab_ChaosWeave"),
        true);
    TestTrue(TEXT("P02 partial reaction runtime map opens"), bOpened);
    if (!bOpened)
    {
        return false;
    }

    ADD_LATENT_AUTOMATION_COMMAND(
        FPinkCabPartialClutchWheelReactionCommand(this));
    return true;
}

#endif
