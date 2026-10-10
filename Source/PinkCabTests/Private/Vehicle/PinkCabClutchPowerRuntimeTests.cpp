#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "EngineUtils.h"
#include "Runtime/PinkCabChaosTatraPawn.h"
#include "Vehicle/PinkCabChaosVehicleMovementComponent.h"

namespace
{
class FClutchBackDriveCommand final : public IAutomationLatentCommand
{
public:
    explicit FClutchBackDriveCommand(FAutomationTestBase* InTest) : Test(InTest), Started(FPlatformTime::Seconds()) {}
    bool Update() override
    {
        if (FPlatformTime::Seconds() - Started > 35.0) { Test->AddError(TEXT("back-drive fixture timed out")); return true; }
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
            const float Coupling = Stage == 1 ? 0.0f : (Stage == 2 ? 0.5f : 1.0f);
            const float Throttle = Stage == 0 ? 0.8f : 0.0f;
            Controls.SetDriveline(1, 1, Coupling);
            Controls.SetClutch(1.0f - Coupling);
            Controls.SetThrottle(Throttle);
            Controls.SetResolvedEngineActuation(Stage < 2, Throttle, Throttle,
                Movement->EngineSetup.MaxTorque, Movement->EngineSetup.MaxTorque * Throttle);
            Pawn->GetPinkCabDynamicsProvider().ApplyControls(Controls);
            StageStart = World->GetTimeSeconds();
            return false;
        }
        const double Time = World->GetTimeSeconds() - StageStart;
        const auto Step = Movement->GetPinkCabDrivelineStepTelemetry();
        if (Stage == 2 && Time > 0.1 && Step.PhysicsStep != LastStep)
        {
            LastStep = Step.PhysicsStep;
            MaxEnergyGain = FMath::Max(MaxEnergyGain, Step.ConnectionEnergyDeltaJ);
            MinimumGearLoss = FMath::Min(MinimumGearLoss, Step.GearLossJ);
            if (Step.TransferredTorqueNm < -0.01) ++BackDriveSamples;
            if (Step.ConnectionEnergyDeltaJ > 0.01) ++EnergyGainSamples;
            PeakEngineOmega = FMath::Max(PeakEngineOmega, Step.EngineOmegaAfter);
        }
        if (Time < (Stage == 0 ? 6.0 : (Stage == 1 ? 2.5 : 3.0))) return false;
        Test->AddInfo(FString::Printf(TEXT("T6_BACKDRIVE stage=%d speed_cm_s=%.3f engine_rad_s=%.4f shaft_rad_s=%.4f torque_nm=%.5f"),
            Stage, Movement->GetForwardSpeed(), Step.EngineOmegaAfter, Step.ShaftOmega, Step.TransferredTorqueNm));
        if (Stage == 0) Test->TestTrue(TEXT("backdrive starts from actual native propulsion, no injected velocity"), Movement->GetForwardSpeed() > 250.0f);
        if (++Stage < 3) { StageStart = -1.0; return false; }
        Pawn->SetActorTickEnabled(true);
        Test->AddInfo(FString::Printf(TEXT("T6_BACKDRIVE samples=%d gain_samples=%d max_energy_gain_j=%.8f minimum_gear_loss_j=%.8f peak_engine_rad_s=%.4f"),
            BackDriveSamples, EnergyGainSamples, MaxEnergyGain, MinimumGearLoss, PeakEngineOmega));
        Test->TestTrue(TEXT("real road wheels drive the non-combusting engine through partial clutch"), BackDriveSamples >= 15);
        Test->TestTrue(TEXT("gear efficiency cannot create energy during wheel-to-engine flow"), EnergyGainSamples == 0 && MinimumGearLoss >= -0.01);
        return true;
    }
private:
    FAutomationTestBase* Test;
    double Started;
    double StageStart = -1.0;
    int32 Stage = 0;
    uint64 LastStep = 0;
    int32 BackDriveSamples = 0;
    int32 EnergyGainSamples = 0;
    double MaxEnergyGain = 0.0;
    double MinimumGearLoss = 0.0;
    double PeakEngineOmega = 0.0;
};
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPinkCabClutchBackDrivePowerTest,
    "PinkCab.Vehicle.Actuation.ClutchBackDrivePower",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPinkCabClutchBackDrivePowerTest::RunTest(const FString& Parameters)
{
    if (!AutomationOpenMap(TEXT("/Game/Dev/Maps/L_PinkCab_L1_EndlessStraight"), true)) return false;
    ADD_LATENT_AUTOMATION_COMMAND(FClutchBackDriveCommand(this));
    return true;
}
#endif
