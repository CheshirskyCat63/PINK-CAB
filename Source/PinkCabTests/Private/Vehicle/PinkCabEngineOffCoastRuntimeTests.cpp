#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "EngineUtils.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "HAL/PlatformTime.h"
#include "Kismet/GameplayStatics.h"
#include "Runtime/PinkCabChaosTatraPawn.h"
#include "Vehicle/PinkCabChaosCockpitBridge.h"
#include "Vehicle/PinkCabChaosVehicleDynamicsProvider.h"
#include "Vehicle/PinkCabCockpitState.h"
#include "Vehicle/PinkCabVehicleControlState.h"

class FPinkCabEngineOffCoastCommand final : public IAutomationLatentCommand
{
public:
    explicit FPinkCabEngineOffCoastCommand(FAutomationTestBase* InTest)
        : Test(InTest) {}

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

        UChaosWheeledVehicleMovementComponent* Movement =
            Pawn->GetChaosMovement();
        USkeletalMeshComponent* Mesh = Pawn->GetMesh();
        Test->TestNotNull(TEXT("live Chaos movement exists"), Movement);
        Test->TestNotNull(TEXT("live vehicle mesh exists"), Mesh);
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

            Test->TestTrue(TEXT("fixture starts local engine"), Cockpit.StartEngine());
            Controls.SetThrottle(0.75f);
            Controls.SetBrake(0.0f);
            Controls.SetHandbrake(0.0f);
            Controls.SetDriveline(1, 1, 0.50f);

            FPinkCabChaosVehicleDynamicsProvider Provider(Movement);
            Test->TestTrue(TEXT("running launch controls apply"),
                FPinkCabChaosCockpitBridge::Apply(
                    Cockpit, *Movement, Controls, Provider));
            Test->TestTrue(TEXT("launch has combustion permission"),
                Controls.IsCombustionAllowed());
            Test->TestTrue(TEXT("partial-clutch launch requests positive torque"),
                Controls.ExternalRearDriveTorquePerWheelNm > 0.0f);

            LaunchStartLocation = Mesh->GetComponentLocation();
            PhaseStartSeconds = FPlatformTime::Seconds();
            bInitialized = true;
            return false;
        }

        const double Elapsed = FPlatformTime::Seconds() - PhaseStartSeconds;
        if (!bKeyedOff)
        {
            FPinkCabChaosVehicleDynamicsProvider Provider(Movement);
            if (!FPinkCabChaosCockpitBridge::Apply(
                    Cockpit, *Movement, Controls, Provider))
            {
                Test->AddError(TEXT("running launch actuation refresh failed"));
                return true;
            }

            const float SpeedCmPerSec =
                Mesh->GetPhysicsLinearVelocity().Size2D();
            const float LaunchTravelCm =
                FVector::Dist2D(Mesh->GetComponentLocation(), LaunchStartLocation);
            if (SpeedCmPerSec < 300.0f || LaunchTravelCm < 100.0f)
            {
                if (Elapsed < 6.0)
                {
                    return false;
                }
                Test->AddError(FString::Printf(
                    TEXT("live drivetrain failed to establish coast precondition speed=%.3f travel=%.3f"),
                    SpeedCmPerSec,
                    LaunchTravelCm));
                return true;
            }

            Cockpit.StopEngine();
            FPinkCabChaosVehicleDynamicsProvider Provider(Movement);
            Test->TestTrue(TEXT("key-off controls apply"),
                FPinkCabChaosCockpitBridge::Apply(
                    Cockpit, *Movement, Controls, Provider));
            Test->TestFalse(TEXT("key-off disables mechanical engine sim"),
                Movement->bMechanicalSimEnabled);
            Test->TestFalse(TEXT("key-off denies combustion"),
                Controls.IsCombustionAllowed());
            Test->TestEqual(TEXT("key-off sends zero Chaos throttle"),
                Movement->GetThrottleInput(), 0.0f);
            Test->TestEqual(TEXT("key-off clears external drive torque"),
                Controls.ExternalRearDriveTorquePerWheelNm, 0.0f);

            KeyOffLocation = Mesh->GetComponentLocation();
            KeyOffSpeedCmPerSec = SpeedCmPerSec;
            PhaseStartSeconds = FPlatformTime::Seconds();
            bKeyedOff = true;
            return false;
        }

        FPinkCabChaosVehicleDynamicsProvider OffProvider(Movement);
        if (!FPinkCabChaosCockpitBridge::Apply(
                Cockpit, *Movement, Controls, OffProvider))
        {
            Test->AddError(TEXT("engine-off actuation refresh failed"));
            return true;
        }

        if (Elapsed < 0.75)
        {
            return false;
        }

        const float CoastSpeedCmPerSec =
            Mesh->GetPhysicsLinearVelocity().Size2D();
        const float CoastTravelCm =
            FVector::Dist2D(Mesh->GetComponentLocation(), KeyOffLocation);
        Test->AddInfo(FString::Printf(
            TEXT("P01_FLAT_COAST keyoff_speed_cm_s=%.3f coast_speed_cm_s=%.3f post_keyoff_travel_cm=%.3f"),
            KeyOffSpeedCmPerSec,
            CoastSpeedCmPerSec,
            CoastTravelCm));
        Test->TestTrue(TEXT("engine-off body physically advances after key-off"),
            CoastTravelCm > 20.0f);
        Test->TestTrue(TEXT("engine-off body remains physically free to coast"),
            CoastSpeedCmPerSec > 10.0f);
        Test->TestEqual(TEXT("engine-off coast still has zero Chaos throttle"),
            Movement->GetThrottleInput(), 0.0f);
        Test->TestEqual(TEXT("engine-off coast has zero external drive torque"),
            Controls.ExternalRearDriveTorquePerWheelNm, 0.0f);
        return true;
    }

private:
    FAutomationTestBase* Test = nullptr;
    FPinkCabCockpitState Cockpit;
    FPinkCabVehicleControlState Controls;
    bool bInitialized = false;
    bool bKeyedOff = false;
    double PhaseStartSeconds = 0.0;
    float KeyOffSpeedCmPerSec = 0.0f;
    FVector LaunchStartLocation = FVector::ZeroVector;
    FVector KeyOffLocation = FVector::ZeroVector;
};

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabVehicleEngineOffCoastRuntimeTest,
    "PinkCab.Vehicle.Physics.EngineState.LiveEngineOffCoast",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabVehicleEngineOffCoastRuntimeTest::RunTest(const FString& Parameters)
{
    const bool bOpened = AutomationOpenMap(
        TEXT("/Game/Dev/Maps/L_PinkCab_ChaosWeave"),
        true);
    TestTrue(TEXT("vehicle runtime map opens"), bOpened);
    if (!bOpened)
    {
        return false;
    }

    ADD_LATENT_AUTOMATION_COMMAND(
        FPinkCabEngineOffCoastCommand(this));
    return true;
}

#endif
