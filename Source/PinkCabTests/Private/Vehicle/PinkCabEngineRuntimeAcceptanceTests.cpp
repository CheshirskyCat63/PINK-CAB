#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "EngineUtils.h"
#include "Engine/StaticMeshActor.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "HAL/PlatformTime.h"
#include "Runtime/PinkCabChaosTatraPawn.h"
#include "Vehicle/PinkCabChaosCockpitBridge.h"
#include "Vehicle/PinkCabChaosVehicleDynamicsProvider.h"
#include "Vehicle/PinkCabCockpitState.h"
#include "Vehicle/PinkCabVehicleControlState.h"

namespace
{
APinkCabChaosTatraPawn* FindTatra(UWorld& World)
{
    for (TActorIterator<APinkCabChaosTatraPawn> It(&World); It; ++It)
    {
        return *It;
    }
    return nullptr;
}

bool ApplyEngineState(
    APinkCabChaosTatraPawn& Pawn,
    FPinkCabCockpitState& Cockpit,
    FPinkCabVehicleControlState& Controls)
{
    UChaosWheeledVehicleMovementComponent* Movement = Pawn.GetChaosMovement();
    if (!Movement)
    {
        return false;
    }
    FPinkCabChaosVehicleDynamicsProvider Provider(Movement);
    return FPinkCabChaosCockpitBridge::Apply(
        Cockpit, *Movement, Controls, Provider);
}
}

class FPinkCabEngineOffSlopeCommand final : public IAutomationLatentCommand
{
public:
    explicit FPinkCabEngineOffSlopeCommand(FAutomationTestBase* InTest)
        : Test(InTest) {}

    virtual bool Update() override
    {
        UWorld* World = AutomationCommon::GetAnyGameWorld();
        if (!World)
        {
            return false;
        }

        APinkCabChaosTatraPawn* Pawn = FindTatra(*World);
        if (!Pawn)
        {
            return false;
        }
        UChaosWheeledVehicleMovementComponent* Movement =
            Pawn->GetChaosMovement();
        USkeletalMeshComponent* Mesh = Pawn->GetMesh();
        Test->TestNotNull(TEXT("slope fixture has movement"), Movement);
        Test->TestNotNull(TEXT("slope fixture has physics mesh"), Mesh);
        if (!Movement || !Mesh)
        {
            return true;
        }

        if (!bInitialized)
        {
            Pawn->SetActorTickEnabled(false);

            UStaticMesh* Cube = LoadObject<UStaticMesh>(
                nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
            Test->TestNotNull(TEXT("engine cube exists for isolated slope"), Cube);
            if (!Cube)
            {
                return true;
            }

            Ramp = World->SpawnActor<AStaticMeshActor>();
            Test->TestNotNull(TEXT("isolated slope actor spawns"), Ramp);
            if (!Ramp)
            {
                return true;
            }

            constexpr float RampPitchDeg = 10.0f;
            Ramp->GetStaticMeshComponent()->SetStaticMesh(Cube);
            Ramp->GetStaticMeshComponent()->SetMobility(EComponentMobility::Static);
            Ramp->GetStaticMeshComponent()->SetCollisionProfileName(TEXT("BlockAll"));
            Ramp->SetActorScale3D(FVector(14.0f, 5.0f, 0.10f));
            Ramp->SetActorRotation(FRotator(RampPitchDeg, 0.0f, 0.0f));
            Ramp->SetActorLocation(FVector(0.0f, 0.0f, 12000.0f));

            Pawn->SetActorLocationAndRotation(
                FVector(0.0f, 0.0f, 12320.0f),
                FRotator(RampPitchDeg, 0.0f, 0.0f),
                false,
                nullptr,
                ETeleportType::TeleportPhysics);
            Mesh->SetPhysicsLinearVelocity(FVector::ZeroVector);
            Mesh->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);

            Controls.SetThrottle(1.0f);
            Controls.SetBrake(0.0f);
            Controls.SetHandbrake(0.0f);
            Controls.SetDriveline(0, 0, 0.0f);
            Test->TestTrue(TEXT("off neutral slope state applies"),
                ApplyEngineState(*Pawn, Cockpit, Controls));
            Test->TestFalse(TEXT("off slope denies combustion"),
                Controls.IsCombustionAllowed());
            Test->TestEqual(TEXT("off slope final throttle is zero"),
                Controls.GetResolvedEngineThrottle01(), 0.0f);
            Test->TestEqual(TEXT("off slope external drive torque is zero"),
                Controls.ExternalRearDriveTorquePerWheelNm, 0.0f);
            Test->TestFalse(TEXT("off slope mechanical engine sim disabled"),
                Movement->bMechanicalSimEnabled);

            PhaseStartSeconds = FPlatformTime::Seconds();
            bInitialized = true;
            return false;
        }

        const double Elapsed = FPlatformTime::Seconds() - PhaseStartSeconds;
        if (!bSettled)
        {
            if (Elapsed < 0.60)
            {
                return false;
            }
            StartLocation = Pawn->GetActorLocation();
            StartVelocity = Mesh->GetPhysicsLinearVelocity();
            bSettled = true;
            PhaseStartSeconds = FPlatformTime::Seconds();
            return false;
        }

        if (FPlatformTime::Seconds() - PhaseStartSeconds < 1.20)
        {
            return false;
        }

        const FVector EndLocation = Pawn->GetActorLocation();
        const FVector EndVelocity = Mesh->GetPhysicsLinearVelocity();
        const float HorizontalTravelCm =
            FVector2D(EndLocation - StartLocation).Size();
        const float HorizontalSpeedCmPerSec =
            FVector2D(EndVelocity.X, EndVelocity.Y).Size();
        const float StartHorizontalSpeedCmPerSec =
            FVector2D(StartVelocity.X, StartVelocity.Y).Size();

        Test->TestTrue(TEXT("off neutral vehicle moves downhill under gravity"),
            HorizontalTravelCm > 5.0f);
        Test->TestTrue(TEXT("slope creates physical rolling speed"),
            HorizontalSpeedCmPerSec > StartHorizontalSpeedCmPerSec + 5.0f);
        Test->TestEqual(TEXT("slope coast remains zero engine throttle"),
            Movement->GetThrottleInput(), 0.0f);
        Test->TestEqual(TEXT("slope coast remains zero external drive torque"),
            Controls.ExternalRearDriveTorquePerWheelNm, 0.0f);

        if (Ramp)
        {
            Ramp->Destroy();
            Ramp = nullptr;
        }
        return true;
    }

private:
    FAutomationTestBase* Test = nullptr;
    AStaticMeshActor* Ramp = nullptr;
    FPinkCabCockpitState Cockpit;
    FPinkCabVehicleControlState Controls;
    bool bInitialized = false;
    bool bSettled = false;
    double PhaseStartSeconds = 0.0;
    FVector StartLocation = FVector::ZeroVector;
    FVector StartVelocity = FVector::ZeroVector;
};

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabEngineOffNeutralSlopeRuntimeTest,
    "PinkCab.Vehicle.Physics.EngineState.LiveOffNeutralSlope",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabEngineOffNeutralSlopeRuntimeTest::RunTest(const FString& Parameters)
{
    const bool bOpened = AutomationOpenMap(
        TEXT("/Game/Dev/Maps/L_PinkCab_ChaosWeave"), true);
    TestTrue(TEXT("slope runtime map opens"), bOpened);
    if (!bOpened)
    {
        return false;
    }
    ADD_LATENT_AUTOMATION_COMMAND(FPinkCabEngineOffSlopeCommand(this));
    return true;
}

class FPinkCabWarmIdleBlipCommand final : public IAutomationLatentCommand
{
public:
    explicit FPinkCabWarmIdleBlipCommand(FAutomationTestBase* InTest)
        : Test(InTest) {}

    virtual bool Update() override
    {
        UWorld* World = AutomationCommon::GetAnyGameWorld();
        if (!World)
        {
            return false;
        }

        APinkCabChaosTatraPawn* Pawn = FindTatra(*World);
        if (!Pawn)
        {
            return false;
        }
        UChaosWheeledVehicleMovementComponent* Movement =
            Pawn->GetChaosMovement();
        Test->TestNotNull(TEXT("idle fixture has movement"), Movement);
        if (!Movement)
        {
            return true;
        }

        if (!bInitialized)
        {
            Pawn->SetActorTickEnabled(false);
            Cockpit.StartEngine();
            Controls.SetBrake(0.0f);
            Controls.SetHandbrake(1.0f);
            Controls.SetDriveline(0, 0, 0.0f);
            Controls.SetThrottle(0.0f);
            Test->TestTrue(TEXT("warm idle engine state applies"),
                ApplyEngineState(*Pawn, Cockpit, Controls));
            Test->TestTrue(TEXT("warm idle enables mechanical sim"),
                Movement->bMechanicalSimEnabled);
            Phase = EPhase::SettlingIdle;
            PhaseStartSeconds = FPlatformTime::Seconds();
            bInitialized = true;
            return false;
        }

        const double Elapsed = FPlatformTime::Seconds() - PhaseStartSeconds;
        if (Phase == EPhase::SettlingIdle)
        {
            if (Elapsed < 1.50)
            {
                return false;
            }
            IdleBeforeBlip = Movement->GetEngineRotationSpeed();
            Test->TestTrue(TEXT("warm healthy neutral idle settles inside 900-950 rpm"),
                IdleBeforeBlip >= 900.0f && IdleBeforeBlip <= 950.0f);

            Controls.SetThrottle(0.55f);
            Test->TestTrue(TEXT("throttle blip applies through authoritative bridge"),
                ApplyEngineState(*Pawn, Cockpit, Controls));
            Phase = EPhase::Blipping;
            PhaseStartSeconds = FPlatformTime::Seconds();
            return false;
        }

        if (Phase == EPhase::Blipping)
        {
            if (Elapsed < 0.70)
            {
                return false;
            }
            const float BlipRpm = Movement->GetEngineRotationSpeed();
            Test->TestTrue(TEXT("neutral throttle blip raises rpm above idle"),
                BlipRpm > IdleBeforeBlip + 150.0f);

            Controls.SetThrottle(0.0f);
            Test->TestTrue(TEXT("blip release applies zero driver throttle"),
                ApplyEngineState(*Pawn, Cockpit, Controls));
            Phase = EPhase::ReturningIdle;
            PhaseStartSeconds = FPlatformTime::Seconds();
            return false;
        }

        if (Elapsed < 2.00)
        {
            return false;
        }

        const float ReturnedIdleRpm = Movement->GetEngineRotationSpeed();
        Test->TestTrue(TEXT("engine returns to warm carb idle band after blip"),
            ReturnedIdleRpm >= 900.0f && ReturnedIdleRpm <= 950.0f);
        Test->TestEqual(TEXT("released blip has zero final throttle"),
            Controls.GetResolvedEngineThrottle01(), 0.0f);
        return true;
    }

private:
    enum class EPhase : uint8
    {
        SettlingIdle,
        Blipping,
        ReturningIdle
    };

    FAutomationTestBase* Test = nullptr;
    FPinkCabCockpitState Cockpit;
    FPinkCabVehicleControlState Controls;
    bool bInitialized = false;
    EPhase Phase = EPhase::SettlingIdle;
    double PhaseStartSeconds = 0.0;
    float IdleBeforeBlip = 0.0f;
};

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabWarmIdleBlipRuntimeTest,
    "PinkCab.Vehicle.Physics.EngineState.LiveWarmIdleBlipReturn",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabWarmIdleBlipRuntimeTest::RunTest(const FString& Parameters)
{
    const bool bOpened = AutomationOpenMap(
        TEXT("/Game/Dev/Maps/L_PinkCab_ChaosWeave"), true);
    TestTrue(TEXT("idle runtime map opens"), bOpened);
    if (!bOpened)
    {
        return false;
    }
    ADD_LATENT_AUTOMATION_COMMAND(FPinkCabWarmIdleBlipCommand(this));
    return true;
}

#endif
