#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Vehicle/PinkCabDrivelineRuntimeTestUtils.h"

namespace
{
AActor* SpawnEightPercentRamp(UWorld& World)
{
    AActor* Ramp = World.SpawnActor<AActor>();
    if (!Ramp)
    {
        return nullptr;
    }

    constexpr float Grade = 0.08f;
    const float PitchDeg = FMath::RadiansToDegrees(FMath::Atan(Grade));
    UBoxComponent* Box = NewObject<UBoxComponent>(Ramp);
    Ramp->SetRootComponent(Box);
    Box->SetBoxExtent(FVector(1200.0f, 350.0f, 60.0f));
    Box->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    Box->SetCollisionObjectType(ECC_WorldStatic);
    Box->SetCollisionResponseToAllChannels(ECR_Block);
    Box->SetGenerateOverlapEvents(false);
    Box->RegisterComponent();
    Ramp->SetActorRotation(FRotator(PitchDeg, 0.0f, 0.0f));
    Ramp->SetActorLocation(FVector(0.0f, 0.0f, 12000.0f));
    Box->RecreatePhysicsState();
    return Ramp;
}
}

class FPinkCabD4FlatReverseCommand final : public IAutomationLatentCommand
{
public:
    explicit FPinkCabD4FlatReverseCommand(FAutomationTestBase* InTest)
        : Test(InTest) {}

    virtual bool Update() override
    {
        using namespace PinkCabDrivelineRuntimeTest;
        UWorld* World = AutomationCommon::GetAnyGameWorld();
        if (!World) return false;

        AActor* Floor = PinkCabPhysicsFixture::FindOrSpawnFlatFloor(*World);
        APinkCabPhysicsFixturePawn* Pawn =
            PinkCabPhysicsFixture::FindOrSpawnPawn(*World);
        if (!Floor || !Pawn)
        {
            Test->AddError(TEXT("D4 launch fixture failed to spawn"));
            return true;
        }
        PinkCabPhysicsFixture::KeepAwake(*Pawn);

        UChaosWheeledVehicleMovementComponent* Movement =
            Pawn->GetChaosMovement();
        UPinkCabChaosVehicleMovementComponent* PinkCabMovement =
            Cast<UPinkCabChaosVehicleMovementComponent>(Movement);
        USkeletalMeshComponent* Mesh = Pawn->GetMesh();
        if (!Movement || !PinkCabMovement || !Mesh)
        {
            Test->AddError(TEXT("D4 launch fixture is incomplete"));
            return true;
        }

        if (!bInitialized)
        {
            UGameplayStatics::SetGamePaused(World, false);
            Test->TestTrue(TEXT("D4 launch engine starts"), Cockpit.StartEngine());
            bInitialized = true;
            BeginRun(*World);
            return false;
        }

        if (!Apply(*Pawn, Cockpit, Controls))
        {
            Test->AddError(TEXT("D4 launch actuation failed"));
            return true;
        }

        if (bSettling)
        {
            if (!RestGate.Update(*Pawn)) return false;

            StartLocation = Mesh->GetComponentLocation();
            StartForward = Pawn->GetActorForwardVector();
            Controls = {};
            Controls.SetThrottle(0.55f);
            Controls.SetBrake(0.0f);
            Controls.SetHandbrake(0.0f);
            Controls.SetDriveline(CurrentGear(), CurrentGear(), 0.75f);
            Controls.SetDrivetrainTorqueCapacity(1.0f);
            LastMechanicalStep =
                PinkCabMovement->GetPinkCabMechanicalIntegrationStepCount();
            PhaseSimSeconds = 0.0;
            bSettling = false;
            return false;
        }

        if (!AdvanceMechanicalTime(
                *PinkCabMovement, LastMechanicalStep, PhaseSimSeconds))
        {
            return false;
        }

        const float SignedTravel = SignedForwardDistanceCm(
            StartLocation, Mesh->GetComponentLocation(), StartForward);
        const float Speed = HorizontalSpeedCmPerSec(*Mesh);
        const float RearTorque = MeanRearDriveTorqueNm(*Movement);
        const bool bReached = CurrentGear() > 0
            ? SignedTravel > 100.0f
            : SignedTravel < -100.0f;

        if (!bReached && PhaseSimSeconds < 6.0)
        {
            return false;
        }

        Test->AddInfo(FString::Printf(
            TEXT("P02_D4_LAUNCH gear=%d sim_s=%.3f travel_cm=%.3f speed_cm_s=%.3f rear_torque_nm=%.3f"),
            CurrentGear(), PhaseSimSeconds, SignedTravel, Speed, RearTorque));
        Test->TestTrue(
            CurrentGear() > 0
                ? TEXT("D4 first gear produces forward travel")
                : TEXT("D4 reverse produces backward travel"),
            bReached);
        Test->TestTrue(TEXT("D4 launch reaches measurable chassis speed"),
            Speed > 100.0f);
        Test->TestTrue(
            TEXT("D4 launch carries measurable rear-wheel drive torque"),
            FMath::Abs(RearTorque) > 1.0f);

        if (++GearIndex >= Gears.Num())
        {
            return true;
        }
        BeginRun(*World);
        return false;
    }

private:
    int32 CurrentGear() const { return Gears[GearIndex]; }

    void BeginRun(UWorld& World)
    {
        PinkCabPhysicsFixture::DestroyPawns(World);
        Controls = {};
        Controls.SetBrake(1.0f);
        Controls.SetDriveline(0, 0, 0.0f);
        Controls.SetDrivetrainTorqueCapacity(1.0f);
        RestGate.Reset();
        bSettling = true;
        LastMechanicalStep = -1;
        PhaseSimSeconds = 0.0;
    }

    FAutomationTestBase* Test = nullptr;
    FPinkCabCockpitState Cockpit;
    FPinkCabVehicleControlState Controls;
    FPinkCabPhysicsFixtureRestGate RestGate;
    const TArray<int32> Gears{1, -1};
    int32 GearIndex = 0;
    int64 LastMechanicalStep = -1;
    double PhaseSimSeconds = 0.0;
    bool bInitialized = false;
    bool bSettling = false;
    FVector StartLocation = FVector::ZeroVector;
    FVector StartForward = FVector::ForwardVector;
};

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabD4FlatReverseRuntimeTest,
    "PinkCab.Vehicle.Physics.P02.D4.FlatAndReverseLaunch",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabD4FlatReverseRuntimeTest::RunTest(const FString&)
{
    const bool bOpened = AutomationOpenMap(PinkCabPhysicsFixture::MapPath, true);
    TestTrue(TEXT("D4 launch map opens"), bOpened);
    if (!bOpened) return false;
    ADD_LATENT_AUTOMATION_COMMAND(FPinkCabD4FlatReverseCommand(this));
    return true;
}

class FPinkCabD4InclineCommand final : public IAutomationLatentCommand
{
public:
    explicit FPinkCabD4InclineCommand(FAutomationTestBase* InTest)
        : Test(InTest) {}

    virtual bool Update() override
    {
        using namespace PinkCabDrivelineRuntimeTest;
        UWorld* World = AutomationCommon::GetAnyGameWorld();
        if (!World) return false;

        if (!Ramp)
        {
            Ramp = SpawnEightPercentRamp(*World);
            Test->TestNotNull(TEXT("D4 8% incline spawns"), Ramp);
            if (!Ramp) return true;
        }

        APinkCabPhysicsFixturePawn* Pawn = nullptr;
        if (!bInitialized)
        {
            PinkCabPhysicsFixture::DestroyPawns(*World);
            constexpr float Grade = 0.08f;
            const float PitchDeg =
                FMath::RadiansToDegrees(FMath::Atan(Grade));
            Pawn = PinkCabPhysicsFixture::SpawnFreshPawn(
                *World,
                FVector(0.0f, 0.0f, 12280.0f),
                FRotator(PitchDeg, 0.0f, 0.0f));
        }
        else
        {
            Pawn = PinkCabPhysicsFixture::FindOrSpawnPawn(*World);
        }
        if (!Pawn) return false;
        PinkCabPhysicsFixture::KeepAwake(*Pawn);

        UChaosWheeledVehicleMovementComponent* Movement =
            Pawn->GetChaosMovement();
        UPinkCabChaosVehicleMovementComponent* PinkCabMovement =
            Cast<UPinkCabChaosVehicleMovementComponent>(Movement);
        USkeletalMeshComponent* Mesh = Pawn->GetMesh();
        if (!Movement || !PinkCabMovement || !Mesh)
        {
            Test->AddError(TEXT("D4 incline fixture is incomplete"));
            return true;
        }

        if (!bInitialized)
        {
            UGameplayStatics::SetGamePaused(World, false);
            Test->TestTrue(TEXT("D4 incline engine starts"), Cockpit.StartEngine());
            Controls = {};
            Controls.SetBrake(1.0f);
            Controls.SetDriveline(0, 0, 0.0f);
            Controls.SetDrivetrainTorqueCapacity(1.0f);
            RestGate.Reset();
            bInitialized = true;
            return false;
        }

        if (!Apply(*Pawn, Cockpit, Controls))
        {
            Test->AddError(TEXT("D4 incline actuation failed"));
            return true;
        }

        if (!bLaunching)
        {
            if (!RestGate.Update(*Pawn)) return false;
            StartLocation = Mesh->GetComponentLocation();
            StartForward = Pawn->GetActorForwardVector();
            Controls = {};
            Controls.SetThrottle(0.70f);
            Controls.SetDriveline(1, 1, 0.75f);
            Controls.SetDrivetrainTorqueCapacity(1.0f);
            LastMechanicalStep =
                PinkCabMovement->GetPinkCabMechanicalIntegrationStepCount();
            PhaseSimSeconds = 0.0;
            bLaunching = true;
            return false;
        }

        if (!AdvanceMechanicalTime(
                *PinkCabMovement, LastMechanicalStep, PhaseSimSeconds))
        {
            return false;
        }

        const float SignedTravel = SignedForwardDistanceCm(
            StartLocation, Mesh->GetComponentLocation(), StartForward);
        if (SignedTravel < 75.0f && PhaseSimSeconds < 6.0)
        {
            return false;
        }

        const float RearTorque = MeanRearDriveTorqueNm(*Movement);
        Test->AddInfo(FString::Printf(
            TEXT("P02_D4_INCLINE grade=0.08 sim_s=%.3f travel_cm=%.3f speed_cm_s=%.3f rear_torque_nm=%.3f"),
            PhaseSimSeconds, SignedTravel, HorizontalSpeedCmPerSec(*Mesh), RearTorque));
        Test->TestTrue(TEXT("D4 drivetrain moves uphill on 8% grade"),
            SignedTravel > 75.0f);
        Test->TestTrue(TEXT("D4 incline retains positive rear torque"),
            RearTorque > 1.0f);
        Ramp->Destroy();
        Ramp = nullptr;
        return true;
    }

private:
    FAutomationTestBase* Test = nullptr;
    AActor* Ramp = nullptr;
    FPinkCabCockpitState Cockpit;
    FPinkCabVehicleControlState Controls;
    FPinkCabPhysicsFixtureRestGate RestGate;
    bool bInitialized = false;
    bool bLaunching = false;
    int64 LastMechanicalStep = -1;
    double PhaseSimSeconds = 0.0;
    FVector StartLocation = FVector::ZeroVector;
    FVector StartForward = FVector::ForwardVector;
};

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabD4InclineRuntimeTest,
    "PinkCab.Vehicle.Physics.P02.D4.InclineLoad",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabD4InclineRuntimeTest::RunTest(const FString&)
{
    const bool bOpened = AutomationOpenMap(PinkCabPhysicsFixture::MapPath, true);
    TestTrue(TEXT("D4 incline map opens"), bOpened);
    if (!bOpened) return false;
    ADD_LATENT_AUTOMATION_COMMAND(FPinkCabD4InclineCommand(this));
    return true;
}

#endif
