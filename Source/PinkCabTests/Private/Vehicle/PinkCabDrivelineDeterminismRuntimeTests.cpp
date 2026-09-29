#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "HAL/IConsoleManager.h"
#include "Kismet/GameplayStatics.h"
#include "Vehicle/PinkCabPhysicsFixturePawn.h"
#include "Vehicle/PinkCabChaosCockpitBridge.h"
#include "Vehicle/PinkCabChaosVehicleDynamicsProvider.h"
#include "Vehicle/PinkCabChaosVehicleMovementComponent.h"
#include "Vehicle/PinkCabCockpitState.h"
#include "Vehicle/PinkCabVehicleControlState.h"

namespace
{
struct FPinkCabD5Run
{
    int32 GameFpsCap = 0;
    int32 Repeat = 0;
    float MeanGameDeltaMs = 0.0f;
    float MechanicalDeltaMs = 0.0f;
    float RearTorqueNm = 0.0f;
    float EngineRpm = 0.0f;
    float EngineResponseRpmPerSec = 0.0f;
    float ResetEngineRpm = 0.0f;
    int32 SampleSteps = 0;
};

float MedianD5(TArray<float> Values)
{
    Values.Sort();
    if (Values.IsEmpty()) return 0.0f;
    const int32 Middle = Values.Num() / 2;
    return Values.Num() % 2 == 0
        ? 0.5f * (Values[Middle - 1] + Values[Middle])
        : Values[Middle];
}

float RelativeD5(const float A, const float B, const float Floor)
{
    return FMath::Abs(A - B)
        / FMath::Max3(FMath::Abs(A), FMath::Abs(B), Floor);
}
}

class FPinkCabD5CadenceCommand final : public IAutomationLatentCommand
{
public:
    explicit FPinkCabD5CadenceCommand(FAutomationTestBase* InTest)
        : Test(InTest) {}

    virtual ~FPinkCabD5CadenceCommand() override
    {
        RestoreMaxFps();
    }

    virtual bool Update() override
    {
        UWorld* World = AutomationCommon::GetAnyGameWorld();
        if (!World) return false;

        AActor* Floor = PinkCabPhysicsFixture::FindOrSpawnFlatFloor(*World);
        APinkCabPhysicsFixturePawn* Pawn =
            PinkCabPhysicsFixture::FindOrSpawnPawn(*World);
        if (!Floor || !Pawn)
        {
            Test->AddError(TEXT("D5 cadence fixture failed to spawn"));
            return true;
        }
        PinkCabPhysicsFixture::KeepAwake(*Pawn);

        UChaosWheeledVehicleMovementComponent* Movement =
            Pawn->GetChaosMovement();
        UPinkCabChaosVehicleMovementComponent* PinkCabMovement =
            Cast<UPinkCabChaosVehicleMovementComponent>(Movement);
        if (!Movement || !PinkCabMovement)
        {
            Test->AddError(TEXT("D5 cadence fixture has no PinkCab movement"));
            return true;
        }

        if (!bInitialized)
        {
            UGameplayStatics::SetGamePaused(World, false);
            MaxFpsCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("t.MaxFPS"));
            Test->TestNotNull(TEXT("D5 t.MaxFPS exists"), MaxFpsCVar);
            if (!MaxFpsCVar) return true;
            OriginalMaxFps = MaxFpsCVar->GetFloat();
            Test->TestTrue(TEXT("D5 cadence engine starts"), Cockpit.StartEngine());
            bInitialized = true;
            BeginFreshRun(*World);
            return false;
        }

        RecordGameDelta();

        FPinkCabChaosVehicleDynamicsProvider Provider(Movement);
        if (!FPinkCabChaosCockpitBridge::Apply(Cockpit, *Movement, Controls, Provider))
        {
            Test->AddError(TEXT("D5 cadence authoritative actuation failed"));
            return true;
        }

        if (bResetting)
        {
            ++ResetPollCount;
            if (!RestGate.Update(*Pawn))
            {
                if (RestGate.GetObservation().ElapsedMechanicalSteps
                        < ResetTimeoutMechanicalSteps
                    && ResetPollCount < ResetPollLimit)
                {
                    return false;
                }
                Test->AddError(TEXT("D5 cadence physical reset did not settle"));
                return true;
            }
            if (GameDeltaSamples < MinimumGameDeltaSamples)
            {
                return false;
            }

            CurrentMeanGameDeltaMs = static_cast<float>(
                GameDeltaSum / static_cast<double>(GameDeltaSamples) * 1000.0);
            CurrentResetEngineRpm =
                RestGate.GetObservation().EngineRpm;
            Controls = {};
            Controls.SetThrottle(0.50f);
            Controls.SetBrake(0.0f);
            Controls.SetHandbrake(0.0f);
            Controls.SetDriveline(1, 1, 0.75f);
            Controls.SetDrivetrainTorqueCapacity(1.0f);

            FPinkCabChaosVehicleDynamicsProvider ConditionProvider(Movement);
            if (!FPinkCabChaosCockpitBridge::Apply(
                    Cockpit, *Movement, Controls, ConditionProvider))
            {
                Test->AddError(TEXT("D5 measured condition failed"));
                return true;
            }
            if (!PinkCabMovement->BeginPinkCabMechanicalEvidenceWindow(
                    0.0f, SingleStepSampleSeconds))
            {
                Test->AddError(TEXT("D5 evidence window failed"));
                return true;
            }
            bResetting = false;
            return false;
        }

        FPinkCabMechanicalEvidenceSnapshot Evidence;
        if (!PinkCabMovement->ReadPinkCabMechanicalEvidenceWindow(Evidence))
        {
            Test->AddError(TEXT("D5 evidence read failed"));
            return true;
        }
        if (!Evidence.bComplete) return false;

        FPinkCabD5Run Run;
        Run.GameFpsCap = CurrentCap();
        Run.Repeat = RepeatIndex;
        Run.MeanGameDeltaMs = CurrentMeanGameDeltaMs;
        Run.MechanicalDeltaMs = Evidence.MeanDeltaSeconds * 1000.0f;
        Run.RearTorqueNm = Evidence.MeanDrivenWheelTorqueNm;
        Run.EngineRpm = Evidence.MeanEngineRpm;
        Run.ResetEngineRpm = CurrentResetEngineRpm;
        Run.EngineResponseRpmPerSec =
            Evidence.MeanDeltaSeconds > KINDA_SMALL_NUMBER
                ? (Evidence.MeanEngineRpm - CurrentResetEngineRpm)
                    / Evidence.MeanDeltaSeconds
                : 0.0f;
        Run.SampleSteps = Evidence.CompletedSampleSteps;
        Runs.Add(Run);

        Test->AddInfo(FString::Printf(
            TEXT("P02_D5_CADENCE fps_cap=%d repeat=%d game_dt_ms=%.6f mechanical_dt_ms=%.6f rear_torque_nm=%.3f engine_rpm=%.3f reset_engine_rpm=%.3f engine_response_rpm_s=%.3f steps=%d"),
            Run.GameFpsCap, Run.Repeat + 1, Run.MeanGameDeltaMs,
            Run.MechanicalDeltaMs, Run.RearTorqueNm, Run.EngineRpm,
            Run.ResetEngineRpm, Run.EngineResponseRpmPerSec,
            Run.SampleSteps));

        if (++RepeatIndex >= RepeatsPerCap)
        {
            RepeatIndex = 0;
            ++CapIndex;
        }
        if (CapIndex >= Caps.Num())
        {
            return Evaluate();
        }

        BeginFreshRun(*World);
        return false;
    }

private:
    int32 CurrentCap() const { return Caps[CapIndex]; }

    void RecordGameDelta()
    {
        const float Delta = FApp::GetDeltaTime();
        ++GameFramesObserved;
        if (GameFramesObserved <= GameDeltaWarmupFrames
            || !FMath::IsFinite(Delta)
            || Delta <= KINDA_SMALL_NUMBER)
        {
            return;
        }
        GameDeltaSum += static_cast<double>(Delta);
        ++GameDeltaSamples;
    }

    void BeginFreshRun(UWorld& World)
    {
        if (MaxFpsCVar)
        {
            MaxFpsCVar->Set(static_cast<float>(CurrentCap()), ECVF_SetByCode);
        }
        PinkCabPhysicsFixture::DestroyPawns(World);
        Controls = {};
        Controls.SetBrake(1.0f);
        Controls.SetDriveline(0, 0, 0.0f);
        Controls.SetDrivetrainTorqueCapacity(1.0f);
        RestGate.Reset();
        ResetPollCount = 0;
        GameFramesObserved = 0;
        GameDeltaSamples = 0;
        GameDeltaSum = 0.0;
        CurrentMeanGameDeltaMs = 0.0f;
        CurrentResetEngineRpm = 0.0f;
        bResetting = true;
    }

    void RestoreMaxFps()
    {
        if (MaxFpsCVar && !bRestored)
        {
            MaxFpsCVar->Set(OriginalMaxFps, ECVF_SetByCode);
            bRestored = true;
        }
    }

    TArray<float> Values(const int32 Cap, const int32 Field) const
    {
        TArray<float> Result;
        for (const FPinkCabD5Run& Run : Runs)
        {
            if (Run.GameFpsCap != Cap) continue;
            if (Field == 0) Result.Add(Run.MeanGameDeltaMs);
            else if (Field == 1) Result.Add(Run.MechanicalDeltaMs);
            else if (Field == 2) Result.Add(Run.RearTorqueNm);
            else Result.Add(Run.EngineResponseRpmPerSec);
        }
        return Result;
    }

    bool Evaluate()
    {
        RestoreMaxFps();
        Test->TestEqual(TEXT("D5 executes 15 fresh cadence runs"),
            Runs.Num(), Caps.Num() * RepeatsPerCap);
        for (const FPinkCabD5Run& Run : Runs)
        {
            Test->TestTrue(TEXT("D5 records positive game delta"),
                Run.MeanGameDeltaMs > 0.0f && FMath::IsFinite(Run.MeanGameDeltaMs));
            Test->TestTrue(TEXT("D5 records positive mechanical delta"),
                Run.MechanicalDeltaMs > 0.0f && FMath::IsFinite(Run.MechanicalDeltaMs));
            Test->TestEqual(TEXT("D5 evidence is exactly one mechanical step"),
                Run.SampleSteps, 1);
            Test->TestTrue(TEXT("D5 records finite normalized engine response"),
                FMath::IsFinite(Run.EngineResponseRpmPerSec));
            Test->TestTrue(TEXT("D5 produces measurable driveline torque"),
                FMath::Abs(Run.RearTorqueNm) > 25.0f);
        }

        const float Game30 = MedianD5(Values(30, 0));
        const float Game60 = MedianD5(Values(60, 0));
        const float Game120 = MedianD5(Values(120, 0));
        Test->TestTrue(TEXT("D5 observed game cadence changes 30 to 60"),
            Game30 > Game60);
        Test->TestTrue(TEXT("D5 observed game cadence changes 60 to 120"),
            Game60 > Game120);

        const float Torque30 = MedianD5(Values(30, 2));
        const float Rpm30 = MedianD5(Values(30, 3));
        for (const int32 Cap : {60, 120})
        {
            Test->TestTrue(TEXT("D5 torque is cadence invariant"),
                RelativeD5(Torque30, MedianD5(Values(Cap, 2)), 25.0f)
                    <= ResponseRelativeTolerance);
            Test->TestTrue(TEXT("D5 engine response is cadence invariant"),
                RelativeD5(Rpm30, MedianD5(Values(Cap, 3)), 250.0f)
                    <= ResponseRelativeTolerance);
        }
        return true;
    }

    FAutomationTestBase* Test = nullptr;
    FPinkCabCockpitState Cockpit;
    FPinkCabVehicleControlState Controls;
    FPinkCabPhysicsFixtureRestGate RestGate;
    IConsoleVariable* MaxFpsCVar = nullptr;
    const TArray<int32> Caps{30, 60, 120};
    TArray<FPinkCabD5Run> Runs;

    static constexpr int32 RepeatsPerCap = 5;
    static constexpr int32 GameDeltaWarmupFrames = 5;
    static constexpr int32 MinimumGameDeltaSamples = 10;
    static constexpr float SingleStepSampleSeconds = 1.0e-4f;
    static constexpr float ResponseRelativeTolerance = 0.05f;
    static constexpr int64 ResetTimeoutMechanicalSteps = 240;
    static constexpr int32 ResetPollLimit = 2400;

    int32 CapIndex = 0;
    int32 RepeatIndex = 0;
    int32 ResetPollCount = 0;
    int32 GameFramesObserved = 0;
    int32 GameDeltaSamples = 0;
    bool bInitialized = false;
    bool bResetting = false;
    bool bRestored = false;
    float OriginalMaxFps = 0.0f;
    float CurrentMeanGameDeltaMs = 0.0f;
    float CurrentResetEngineRpm = 0.0f;
    double GameDeltaSum = 0.0;
};

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabD5CadenceRuntimeTest,
    "PinkCab.Vehicle.Physics.P02.D5.RenderCadenceInvariance",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabD5CadenceRuntimeTest::RunTest(const FString&)
{
    const bool bOpened = AutomationOpenMap(PinkCabPhysicsFixture::MapPath, true);
    TestTrue(TEXT("D5 cadence map opens"), bOpened);
    if (!bOpened) return false;
    ADD_LATENT_AUTOMATION_COMMAND(FPinkCabD5CadenceCommand(this));
    return true;
}

#endif
