#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "EngineUtils.h"
#include "ChaosVehicleWheel.h"
#include "Components/SkeletalMeshComponent.h"
#include "Runtime/PinkCabChaosTatraPawn.h"
#include "Vehicle/PinkCabChaosVehicleMovementComponent.h"
#include "Vehicle/PinkCabVehicleControlState.h"

namespace
{
class FAnalogHandbrakeCommand final : public IAutomationLatentCommand
{
public:
    explicit FAnalogHandbrakeCommand(FAutomationTestBase* InTest)
        : Test(InTest), StartWall(FPlatformTime::Seconds()) {}

    bool Update() override
    {
        if (FPlatformTime::Seconds() - StartWall > 30.0)
        {
            Test->AddError(TEXT("analog handbrake runtime timed out"));
            return true;
        }
        UWorld* World = AutomationCommon::GetAnyGameWorld();
        if (!World) return false;
        APinkCabChaosTatraPawn* Pawn = nullptr;
        for (TActorIterator<APinkCabChaosTatraPawn> It(World); It; ++It) { Pawn = *It; break; }
        if (!Pawn || !Pawn->GetChaosMovement() || !Pawn->GetMesh()) return false;
        auto* Movement = Cast<UPinkCabChaosVehicleMovementComponent>(Pawn->GetChaosMovement());
        if (!Movement || Movement->Wheels.Num() != 4) return false;
        if (StageStart < 0.0)
        {
            Pawn->SetSystemMenuOpen(false);
            Pawn->SetActorTickEnabled(false); // Isolate the production actuation seam, not a second control writer.
            Controls.SetHandbrake(Commands[Stage]);
            Controls.SetBrake(0.0f);
            Controls.SetThrottle(0.0f);
            Controls.SetDriveline(0, 0, 0.0f);
            if (!Test->TestTrue(TEXT("provider applies physical handbrake command"),
                Pawn->GetPinkCabDynamicsProvider().ApplyControls(Controls))) return true;
            StageStart = World->GetTimeSeconds();
            return false;
        }
        if (World->GetTimeSeconds() - StageStart < 0.8) return false;
        for (int32 Index = 0; Index < 4; ++Index)
        {
            const UChaosVehicleWheel* Wheel = Movement->Wheels[Index];
            if (!Test->TestNotNull(TEXT("physical wheel exists"), Wheel)) continue;
            const auto& State = Movement->GetWheelState(Index);
            const float Expected = Wheel->bAffectedByHandbrake
                ? Commands[Stage] * Wheel->MaxHandBrakeTorque : 0.0f;
            Test->TestTrue(*FString::Printf(TEXT("handbrake %.2f wheel%d actual torque matches analog magnitude"),
                Commands[Stage], Index), FMath::IsNearlyEqual(State.BrakeTorque, Expected, 0.5f));
            Test->AddInfo(FString::Printf(TEXT("T6_HANDBRAKE command=%.3f wheel=%d actual_nm=%.3f expected_nm=%.3f"),
                Commands[Stage], Index, State.BrakeTorque, Expected));
        }
        if (++Stage == UE_ARRAY_COUNT(Commands))
        {
            Pawn->SetActorTickEnabled(true);
            return true;
        }
        StageStart = -1.0;
        return false;
    }
private:
    FAutomationTestBase* Test;
    double StartWall;
    double StageStart = -1.0;
    int32 Stage = 0;
    FPinkCabVehicleControlState Controls;
    const float Commands[5] = {0.0f, 0.25f, 0.5f, 1.0f, 0.0f};
};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPinkCabAnalogHandbrakeRuntimeTest,
    "PinkCab.Vehicle.Actuation.AnalogHandbrakeTorque",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPinkCabAnalogHandbrakeRuntimeTest::RunTest(const FString& Parameters)
{
    if (!TestTrue(TEXT("handbrake fixture opens production Tatra map"),
        AutomationOpenMap(TEXT("/Game/Dev/Maps/L_PinkCab_L1_EndlessStraight"), true))) return false;
    ADD_LATENT_AUTOMATION_COMMAND(FAnalogHandbrakeCommand(this));
    return true;
}
#endif
