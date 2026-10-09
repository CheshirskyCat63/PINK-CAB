#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "EngineUtils.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "Runtime/PinkCabChaosTatraPawn.h"
#include "Vehicle/PinkCabVehicleControlState.h"

namespace
{
class FDisconnectedCoastCommand final : public IAutomationLatentCommand
{
public:
    explicit FDisconnectedCoastCommand(FAutomationTestBase* InTest)
        : Test(InTest), Started(FPlatformTime::Seconds()) {}
    bool Update() override
    {
        if (FPlatformTime::Seconds() - Started > 35.0) { Test->AddError(TEXT("coast test timed out")); return true; }
        UWorld* World = AutomationCommon::GetAnyGameWorld();
        if (!World) return false;
        APinkCabChaosTatraPawn* Pawn = nullptr;
        for (TActorIterator<APinkCabChaosTatraPawn> It(World); It; ++It) { Pawn = *It; break; }
        if (!Pawn || !Pawn->GetChaosMovement()) return false;
        auto* Movement = Pawn->GetChaosMovement();
        if (Movement->Wheels.Num() != 4) return false;
        if (StageStart < 0.0)
        {
            Pawn->SetSystemMenuOpen(false);
            Pawn->SetActorTickEnabled(false);
            FPinkCabVehicleControlState Controls;
            Controls.SetBrake(0.0f);
            Controls.SetHandbrake(0.0f);
            Controls.SetSteering(0.0f);
            Controls.SetClutch(Stage == 1 ? 1.0f : 0.0f);
            Controls.SetDriveline(Stage == 2 ? 0 : 1, Stage == 2 ? 0 : 1, Stage == 1 ? 0.0f : 1.0f);
            const float Throttle = Stage == 0 ? 0.5f : 0.0f;
            Controls.SetThrottle(Throttle);
            Controls.SetResolvedEngineActuation(true, Throttle, Throttle,
                Movement->EngineSetup.MaxTorque, Throttle * Movement->EngineSetup.MaxTorque);
            if (!Test->TestTrue(TEXT("coast uses production controls"), Pawn->GetPinkCabDynamicsProvider().ApplyControls(Controls))) return true;
            StageStart = World->GetTimeSeconds();
            StartPosition = Pawn->GetActorLocation();
            Forward = Pawn->GetActorForwardVector();
            Samples = 0;
            MaxDrive = MaxBrake = 0.0f;
            bAllContacts = true;
            return false;
        }
        const double Elapsed = World->GetTimeSeconds() - StageStart;
        if (Stage > 0 && Elapsed >= 0.2)
        {
            ++Samples;
            for (int32 Index = 0; Index < 4; ++Index)
            {
                const auto& Wheel = Movement->GetWheelState(Index);
                MaxDrive = FMath::Max(MaxDrive, FMath::Abs(Wheel.DriveTorque));
                MaxBrake = FMath::Max(MaxBrake, FMath::Abs(Wheel.BrakeTorque));
                bAllContacts &= Wheel.bInContact;
            }
        }
        if (Elapsed < (Stage == 0 ? 2.0 : 1.0)) return false;
        const float Travel = FVector::DotProduct(Pawn->GetActorLocation() - StartPosition, Forward);
        Test->AddInfo(FString::Printf(TEXT("T6_COAST stage=%d time=%.3f travel_cm=%.3f speed_cm_s=%.3f engine_rpm=%.1f max_drive_nm=%.3f max_brake_nm=%.3f samples=%d"),
            Stage, Elapsed, Travel, Movement->GetForwardSpeed(), Movement->GetEngineRotationSpeed(), MaxDrive, MaxBrake, Samples));
        Test->TestTrue(TEXT("car moves through the physical road rather than a synthetic velocity"), Travel > 50.0f && Movement->GetForwardSpeed() > 50.0f);
        if (Stage > 0)
        {
            Test->TestTrue(TEXT("coasting keeps four native road contacts"), Samples >= 10 && bAllContacts);
            Test->TestTrue(TEXT("disconnected moving car receives no engine propulsion"), MaxDrive <= 0.5f);
            Test->TestTrue(TEXT("disconnected moving car receives no engine braking"), MaxBrake <= 0.5f);
            Test->TestTrue(TEXT("native mechanical simulation remains enabled while coasting"), Movement->bMechanicalSimEnabled);
        }
        if (++Stage == 3) { Pawn->SetActorTickEnabled(true); return true; }
        StageStart = -1.0;
        return false;
    }
private:
    FAutomationTestBase* Test;
    double Started;
    double StageStart = -1.0;
    FVector StartPosition = FVector::ZeroVector;
    FVector Forward = FVector::ForwardVector;
    int32 Stage = 0;
    int32 Samples = 0;
    float MaxDrive = 0.0f;
    float MaxBrake = 0.0f;
    bool bAllContacts = true;
};
}
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPinkCabDisconnectedCoastRuntimeTest,
    "PinkCab.Vehicle.Actuation.DisconnectedCoast",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPinkCabDisconnectedCoastRuntimeTest::RunTest(const FString& Parameters)
{
    if (!AutomationOpenMap(TEXT("/Game/Dev/Maps/L_PinkCab_L1_EndlessStraight"), true)) return false;
    ADD_LATENT_AUTOMATION_COMMAND(FDisconnectedCoastCommand(this));
    return true;
}
#endif
