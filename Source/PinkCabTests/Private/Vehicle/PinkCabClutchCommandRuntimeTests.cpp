#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "EngineUtils.h"
#include "Runtime/PinkCabChaosTatraPawn.h"
#include "Vehicle/PinkCabChaosVehicleMovementComponent.h"

namespace
{
class FClutchWakeCommand final : public IAutomationLatentCommand
{
public:
    explicit FClutchWakeCommand(FAutomationTestBase* InTest)
        : Test(InTest), Started(FPlatformTime::Seconds()) {}
    bool Update() override
    {
        if (FPlatformTime::Seconds() - Started > 20.0)
        {
            Test->AddError(TEXT("clutch command lifecycle test timed out"));
            return true;
        }
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
            Controls.SetDriveline(1, 1, 0.0f);
            Controls.SetClutch(1.0f);
            Test->TestTrue(TEXT("initial complete command is accepted"), Pawn->GetPinkCabDynamicsProvider().ApplyControls(Controls));
            StageStart = World->GetTimeSeconds();
            return false;
        }
        const double Time = World->GetTimeSeconds() - StageStart;
        if (Stage == 0)
        {
            if (Time < 1.5) return false;
            // Deliberate test fixture only. Production must wake on the clutch
            // change, not depend on an unrelated mouse/brake/handbrake event.
            Movement->SetSleeping(true);
            Before = Movement->GetPinkCabDrivelineStepTelemetry();
            Controls.SetDriveline(1, 1, 0.5f);
            Controls.SetClutch(0.5f);
            Pawn->GetPinkCabDynamicsProvider().ApplyControls(Controls);
            Stage = 1;
            StageStart = World->GetTimeSeconds();
            return false;
        }
        if (Time < 0.3) return false;
        const auto After = Movement->GetPinkCabDrivelineStepTelemetry();
        Pawn->SetActorTickEnabled(true);
        Test->AddInfo(FString::Printf(TEXT("T6_CLUTCH_WAKE before_seq=%llu after_seq=%llu before_step=%llu after_step=%llu c=%.3f gear=%d torque_nm=%.5f"),
            Before.CommandSequence, After.CommandSequence, Before.PhysicsStep, After.PhysicsStep,
            After.Coupling, After.Gear, After.TransferredTorqueNm));
        Test->TestTrue(TEXT("clutch-only change reaches a new physical step"),
            After.PhysicsStep > Before.PhysicsStep && After.CommandSequence > Before.CommandSequence);
        Test->TestTrue(TEXT("asleep vehicle applies changed clutch without another control"),
            FMath::IsNearlyEqual(After.Coupling, 0.5f) && After.Gear == 1);
        return true;
    }
private:
    FAutomationTestBase* Test;
    double Started;
    double StageStart = -1.0;
    int32 Stage = 0;
    FPinkCabVehicleControlState Controls;
    FPinkCabDrivelineStepTelemetry Before;
};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPinkCabClutchCommandWakeTest,
    "PinkCab.Vehicle.Actuation.ClutchCommandWake",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPinkCabClutchCommandWakeTest::RunTest(const FString& Parameters)
{
    if (!AutomationOpenMap(TEXT("/Game/Dev/Maps/L_PinkCab_L1_EndlessStraight"), true)) return false;
    ADD_LATENT_AUTOMATION_COMMAND(FClutchWakeCommand(this));
    return true;
}
#endif
