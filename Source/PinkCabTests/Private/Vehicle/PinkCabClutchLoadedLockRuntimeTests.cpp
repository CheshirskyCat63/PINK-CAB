#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "EngineUtils.h"
#include "Runtime/PinkCabChaosTatraPawn.h"
#include "Vehicle/PinkCabChaosVehicleMovementComponent.h"

namespace
{
class FClutchLoadedLockCommand final : public IAutomationLatentCommand
{
public:
    explicit FClutchLoadedLockCommand(FAutomationTestBase* InTest, int32 InGear = 1) : Test(InTest), Started(FPlatformTime::Seconds()), Gear(InGear) {}
    bool Update() override
    {
        if (FPlatformTime::Seconds() - Started > 25.0) { Test->AddError(TEXT("loaded clutch synchronization timed out")); return true; }
        UWorld* World = AutomationCommon::GetAnyGameWorld();
        if (!World) return false;
        APinkCabChaosTatraPawn* Pawn = nullptr;
        for (TActorIterator<APinkCabChaosTatraPawn> It(World); It; ++It) { Pawn = *It; break; }
        if (!Pawn) return false;
        auto* Movement = Cast<UPinkCabChaosVehicleMovementComponent>(Pawn->GetChaosMovement());
        if (!Movement || Movement->Wheels.Num() != 4) return false;
        if (StageStart < 0.0)
        {
            Pawn->SetSystemMenuOpen(false);
            Pawn->SetActorTickEnabled(false);
            FPinkCabVehicleControlState Controls;
            Controls.SetDriveline(Gear, Gear, 1.0f);
            Controls.SetClutch(0.0f);
            Controls.SetThrottle(0.4f);
            Controls.SetResolvedEngineActuation(true, 0.4f, 0.4f,
                Movement->EngineSetup.MaxTorque, Movement->EngineSetup.MaxTorque * 0.4f);
            Pawn->GetPinkCabDynamicsProvider().ApplyControls(Controls);
            StageStart = World->GetTimeSeconds();
            return false;
        }
        const double Time = World->GetTimeSeconds() - StageStart;
        const auto Step = Movement->GetPinkCabDrivelineStepTelemetry();
        if (Time >= 3.0 && Step.PhysicsStep != LastStep)
        {
            LastStep = Step.PhysicsStep;
            const double WheelDerivedOmega = Step.EffectiveGearRatio
                * (Step.NativeWheelOmegaRad[2] + Step.NativeWheelOmegaRad[3]) * 0.5;
            const double SlipRpm = FMath::Abs(Step.EngineOmegaAfter - WheelDerivedOmega) * 60.0 / (2.0 * PI);
            MaxSlipRpm = FMath::Max(MaxSlipRpm, SlipRpm);
            SumSlipRpm += SlipRpm;
            if (Samples % 20 == 0)
                Test->AddInfo(FString::Printf(TEXT("T6_HOLD_SAMPLE step=%llu c=%.2f e=%.5f pred=%.5f actual=%.5f torque=%.5f residual=%.8f converged=%d"),
                    Step.PhysicsStep, Step.Coupling, Step.EngineOmegaAfter, Step.PredictedShaftOmega,
                    WheelDerivedOmega, Step.TransferredTorqueNm, Step.NativeCouplingResidualNm, Step.bNativeResponseConverged));
            ++Samples;
        }
        if (Time < 6.0) return false;
        Pawn->SetActorTickEnabled(true);
        const double Tolerance = Movement->GetPinkCabClutchConfig().LockedSlipRpm;
        Test->AddInfo(FString::Printf(TEXT("T6_LOADED_LOCK samples=%d speed_cm_s=%.3f mean_slip_rpm=%.3f max_slip_rpm=%.3f profile_locked_slip_rpm=%.3f"),
            Samples, Movement->GetForwardSpeed(), Samples ? SumSlipRpm / Samples : 0.0, MaxSlipRpm, Tolerance));
        Test->AddInfo(FString::Printf(TEXT("T6_HOLD_BOUNDARY engine_before=%.6f engine_after=%.6f shaft_before=%.6f shaft_after=%.6f torque_nm=%.6f capacity_nm=%.6f rear_nm=%.6f"),
            Step.EngineOmegaBefore, Step.EngineOmegaAfter, Step.ShaftOmega,
            Step.EffectiveGearRatio * (Step.NativeWheelOmegaRad[2] + Step.NativeWheelOmegaRad[3]) * 0.5,
            Step.TransferredTorqueNm, Step.CapacityNm, Step.NativeWheelDriveNm[2]));
        Test->TestTrue(TEXT("loaded synchronization is measured during native forward travel"), Samples >= 20 && Movement->GetForwardSpeed() * Gear > 100.0f);
        Test->TestTrue(TEXT("full healthy clutch synchronizes the actual driven wheels under sustained normal load"),
            MaxSlipRpm <= Tolerance);
        return true;
    }
private:
    FAutomationTestBase* Test;
    double Started;
    int32 Gear = 1;
    double StageStart = -1.0;
    uint64 LastStep = 0;
    int32 Samples = 0;
    double MaxSlipRpm = 0.0;
    double SumSlipRpm = 0.0;
};
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPinkCabClutchLoadedLockRuntimeTest,
    "PinkCab.Vehicle.Actuation.ClutchLoadedLock",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPinkCabClutchLoadedLockRuntimeTest::RunTest(const FString& Parameters)
{
    if (!AutomationOpenMap(TEXT("/Game/Dev/Maps/L_PinkCab_L1_EndlessStraight"), true)) return false;
    ADD_LATENT_AUTOMATION_COMMAND(FClutchLoadedLockCommand(this));
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPinkCabClutchLoadedLockReverseRuntimeTest,
    "PinkCab.Vehicle.Actuation.ClutchLoadedLockReverse",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPinkCabClutchLoadedLockReverseRuntimeTest::RunTest(const FString& Parameters)
{
    if (!AutomationOpenMap(TEXT("/Game/Dev/Maps/L_PinkCab_L1_EndlessStraight"), true)) return false;
    ADD_LATENT_AUTOMATION_COMMAND(FClutchLoadedLockCommand(this, -1));
    return true;
}
#endif
