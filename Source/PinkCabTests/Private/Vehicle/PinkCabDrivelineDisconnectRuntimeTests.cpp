#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "EngineUtils.h"
#include "ChaosVehicleWheel.h"
#include "Runtime/PinkCabChaosTatraPawn.h"
#include "Vehicle/PinkCabChaosVehicleMovementComponent.h"
#include "Vehicle/PinkCabVehicleControlState.h"

namespace
{
struct FDisconnectCase { int32 Gear; float Coupling; bool Running; float Throttle; float Brake; float Handbrake; };
class FDisconnectCommand final : public IAutomationLatentCommand
{
public:
    explicit FDisconnectCommand(FAutomationTestBase* InTest, bool bInPartial = false) : Test(InTest), Started(FPlatformTime::Seconds()), bPartial(bInPartial) {}
    bool Update() override
    {
        if (FPlatformTime::Seconds() - Started > 45.0) { Test->AddError(TEXT("disconnect test timed out")); return true; }
        UWorld* World = AutomationCommon::GetAnyGameWorld();
        if (!World) return false;
        APinkCabChaosTatraPawn* Pawn = nullptr;
        for (TActorIterator<APinkCabChaosTatraPawn> It(World); It; ++It) { Pawn = *It; break; }
        if (!Pawn) return false;
        auto* Movement = Cast<UPinkCabChaosVehicleMovementComponent>(Pawn->GetChaosMovement());
        if (!Movement || Movement->Wheels.Num() != 4) return false;
        // A stationary brake/drive counter-torque fixture must actually simulate:
        // sleeping bodies expose their last cached wheel output, not current
        // resolved torque. This test-only wake never changes velocity or forces.
        // Natural sleep/release remains covered by AnalogHandbrakeTorque, which
        // does not force wake. No sleep override is added to production here.
        Movement->SetSleeping(false);
        const FDisconnectCase Case = bPartial ? FDisconnectCase{1, 0.5f, true, 0.5f, 1.0f, 0.0f} : Cases[Stage];
        if (StageStart < 0.0)
        {
            Pawn->SetSystemMenuOpen(false);
            Pawn->SetActorTickEnabled(false);
            FPinkCabVehicleControlState Controls;
            Controls.SetDriveline(Case.Gear, Case.Gear, Case.Coupling);
            Controls.SetClutch(1.0f - Case.Coupling);
            Controls.SetThrottle(Case.Throttle);
            Controls.SetBrake(Case.Brake);
            Controls.SetHandbrake(Case.Handbrake);
            Controls.SetResolvedEngineActuation(Case.Running, Case.Throttle, Case.Throttle,
                Movement->EngineSetup.MaxTorque, Case.Running ? Case.Throttle * Movement->EngineSetup.MaxTorque : 0.0f);
            Test->TestTrue(TEXT("production adapter accepts disconnect case"), Pawn->GetPinkCabDynamicsProvider().ApplyControls(Controls));
            StageStart = World->GetTimeSeconds();
            return false;
        }
        if (World->GetTimeSeconds() - StageStart < 0.8) return false;
        Test->TestTrue(TEXT("native mechanical simulation stays enabled"), Movement->bMechanicalSimEnabled);
        for (int32 Index = 0; Index < 4; ++Index)
        {
            const auto& State = Movement->GetWheelState(Index);
            const UChaosVehicleWheel* Wheel = Movement->Wheels[Index];
            const float ExpectedBrake = Case.Brake * Wheel->MaxBrakeTorque
                + (Wheel->bAffectedByHandbrake ? Case.Handbrake * Wheel->MaxHandBrakeTorque : 0.0f);
            Test->AddInfo(FString::Printf(TEXT("T6_DISCONNECT case=%d gear=%d coupling=%.3f running=%d throttle=%.2f wheel=%d drive=%.3f brake=%.3f expected_brake=%.3f rpm=%.1f"),
                Stage, Case.Gear, Case.Coupling, Case.Running, Case.Throttle, Index,
                State.DriveTorque, State.BrakeTorque, ExpectedBrake, Movement->GetEngineRotationSpeed()));
            if (Case.Gear != 0 && Case.Coupling > 0.0f && Case.Running && Wheel->bAffectedByEngine)
            {
                Test->TestTrue(TEXT("connected clutch transmits physical torque in selected direction"),
                    State.DriveTorque * Case.Gear > 1.0f);
            }
            else
            {
                Test->TestTrue(TEXT("open connection transmits no engine drive torque"), FMath::IsNearlyZero(State.DriveTorque, 0.5f));
            }
            Test->TestTrue(TEXT("open connection preserves only commanded wheel braking"), FMath::IsNearlyEqual(State.BrakeTorque, ExpectedBrake, 0.5f));
        }
        if (bPartial || ++Stage == UE_ARRAY_COUNT(Cases)) { Pawn->SetActorTickEnabled(true); return true; }
        StageStart = -1.0;
        return false;
    }
private:
    FAutomationTestBase* Test;
    double Started;
    double StageStart = -1.0;
    int32 Stage = 0;
    bool bPartial = false;
    const FDisconnectCase Cases[11] = {
        {0,1.0f,false,0,0,0}, {0,1.0f,true,0,0,0}, {0,1.0f,true,0.5f,0,0},
        {1,0.0f,true,0.5f,0,0}, {-1,0.0f,true,0,0,0}, {1,0.0f,false,0,0,0},
        {1,0.0f,true,0,0.25f,0}, {0,1.0f,true,0,0,0.5f},
        {1,1.0f,true,0.5f,1.0f,0}, {-1,1.0f,true,0.5f,1.0f,0}, {0,1.0f,true,0,0,0}};
};
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPinkCabDrivelineDisconnectRuntimeTest,
    "PinkCab.Vehicle.Actuation.DrivelineDisconnect",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPinkCabDrivelineDisconnectRuntimeTest::RunTest(const FString& Parameters)
{
    if (!TestTrue(TEXT("disconnect fixture opens"), AutomationOpenMap(TEXT("/Game/Dev/Maps/L_PinkCab_L1_EndlessStraight"), true))) return false;
    ADD_LATENT_AUTOMATION_COMMAND(FDisconnectCommand(this));
    return true;
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPinkCabPartialClutchPhysicalRuntimeTest,
    "PinkCab.Vehicle.Actuation.PartialClutchTransfer",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPinkCabPartialClutchPhysicalRuntimeTest::RunTest(const FString& Parameters)
{
    if (!TestTrue(TEXT("partial clutch physical fixture opens"), AutomationOpenMap(TEXT("/Game/Dev/Maps/L_PinkCab_L1_EndlessStraight"), true))) return false;
    ADD_LATENT_AUTOMATION_COMMAND(FDisconnectCommand(this, true));
    return true;
}
#endif
