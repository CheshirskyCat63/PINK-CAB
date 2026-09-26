#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "EngineUtils.h"
#include "HAL/PlatformTime.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Runtime/PinkCabChaosTatraPawn.h"
#include "Vehicle/PinkCabVehicleControlState.h"
#include "World/PinkCabL1EndlessRoadStreamer.h"
#include "World/PinkCabL1RoadChunkActor.h"

namespace PinkCabL1EndlessRoadSeamTests
{
struct FState
{
    TWeakObjectPtr<APinkCabChaosTatraPawn> Pawn;
    TWeakObjectPtr<APinkCabL1EndlessRoadStreamer> Streamer;
    double StartedSeconds = -1.0;
    bool bPrepared = false;
};

bool HasUniqueActiveIds(const APinkCabL1EndlessRoadStreamer& Streamer)
{
    TSet<FString> Seen;
    for (const FPinkCabChunkId& Id : Streamer.GetActiveChunkIds())
    {
        const FString Key = Id.Serialize();
        if (!Id.IsValid() || Seen.Contains(Key))
        {
            return false;
        }
        Seen.Add(Key);
    }
    return Seen.Num() == Streamer.GetActiveChunkCount();
}

int32 CountWheelContacts(const UChaosWheeledVehicleMovementComponent& Movement)
{
    int32 Contacts = 0;
    for (int32 WheelIndex = 0; WheelIndex < Movement.GetNumWheels(); ++WheelIndex)
    {
        Contacts += Movement.GetWheelState(WheelIndex).bInContact ? 1 : 0;
    }
    return Contacts;
}

bool HasCollisionOnChunk(const APinkCabL1RoadChunkActor* Chunk)
{
    return Chunk
        && Chunk->IsVisualReady()
        && Chunk->GetRoadMeshComponent()
        && Chunk->GetRoadMeshComponent()->GetCollisionEnabled()
            != ECollisionEnabled::NoCollision;
}
}

class FPinkCabL1EndlessRoadSeamDriveCommand final : public IAutomationLatentCommand
{
public:
    FPinkCabL1EndlessRoadSeamDriveCommand(
        FAutomationTestBase* InTest,
        TSharedRef<PinkCabL1EndlessRoadSeamTests::FState> InState)
        : Test(InTest), State(InState) {}

    virtual bool Update() override
    {
        UWorld* World = AutomationCommon::GetAnyGameWorld();
        if (!World)
        {
            return false;
        }

        if (!State->Pawn.IsValid() || !State->Streamer.IsValid())
        {
            for (TActorIterator<APinkCabChaosTatraPawn> It(World); It; ++It)
            {
                State->Pawn = *It;
                break;
            }
            for (TActorIterator<APinkCabL1EndlessRoadStreamer> It(World); It; ++It)
            {
                State->Streamer = *It;
                break;
            }
        }

        APinkCabChaosTatraPawn* Pawn = State->Pawn.Get();
        APinkCabL1EndlessRoadStreamer* Streamer = State->Streamer.Get();
        if (!Pawn || !Streamer)
        {
            return false;
        }

        UChaosWheeledVehicleMovementComponent* Movement = Pawn->GetChaosMovement();
        if (!Movement)
        {
            Test->AddError(TEXT("endless-road Tatra has no Chaos movement"));
            return true;
        }

        if (!State->bPrepared)
        {
            Pawn->SetSystemMenuOpen(false);
            // Match the proven Chaos runtime smoke harness: disable only the
            // gameplay Pawn tick so live keyboard sampling cannot overwrite
            // the direct test controls. Chaos movement components keep ticking.
            Pawn->SetActorTickEnabled(false);

            const FVector SeamStart(99950.0, 1300.0, 180.0);
            Pawn->SetActorLocation(
                SeamStart,
                false,
                nullptr,
                ETeleportType::TeleportPhysics);
            Pawn->SetActorRotation(FRotator::ZeroRotator);

            Streamer->SetTrackedActor(Pawn);
            Test->TestTrue(TEXT("pre-seam positive window materializes"),
                Streamer->RefreshForState(
                    SeamStart,
                    FVector(100.0, 0.0, 0.0)));
            Test->TestEqual(TEXT("pre-seam current chunk is zero"),
                Streamer->GetCurrentChunkIndex(), 0);
            Test->TestEqual(TEXT("pre-seam pool is seven"),
                Streamer->GetPoolSize(), 7);
            Test->TestEqual(TEXT("pre-seam active set is seven"),
                Streamer->GetActiveChunkCount(), 7);
            Test->TestTrue(TEXT("pre-seam active IDs are unique"),
                PinkCabL1EndlessRoadSeamTests::HasUniqueActiveIds(*Streamer));
            Test->TestTrue(TEXT("chunk zero collision active before seam"),
                PinkCabL1EndlessRoadSeamTests::HasCollisionOnChunk(
                    Streamer->FindActiveChunkActor(0)));
            Test->TestTrue(TEXT("chunk one collision active before seam"),
                PinkCabL1EndlessRoadSeamTests::HasCollisionOnChunk(
                    Streamer->FindActiveChunkActor(1)));

            // This test owns only road seam continuity. Keep it independent
            // from engine/clutch/stall rules by crossing on chassis inertia.
            // Explicitly decouple the drivetrain: a raw velocity fixture must
            // not inherit the live pawn's pre-existing 1st-gear mechanical sim.
            Movement->EnableMechanicalSim(false);
            Movement->SetTargetGear(0, true);

            FPinkCabVehicleControlState FreeRoll;
            FreeRoll.SetThrottle(0.0f);
            FreeRoll.SetSteering(0.0f);
            FreeRoll.SetBrake(0.0f);
            FreeRoll.SetHandbrake(0.0f);
            FreeRoll.SetDriveline(0, 0, 0.0f);
            Pawn->GetPinkCabDynamicsProvider().ApplyControls(FreeRoll);

            USkeletalMeshComponent* Mesh = Pawn->GetMesh();
            Test->TestNotNull(TEXT("seam fixture has physical vehicle mesh"), Mesh);
            if (!Mesh)
            {
                return true;
            }
            Mesh->SetPhysicsLinearVelocity(FVector(900.0f, 0.0f, 0.0f));
            Mesh->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
            Mesh->WakeAllRigidBodies();

            State->StartedSeconds = FPlatformTime::Seconds();
            State->bPrepared = true;
            return false;
        }

        const double Elapsed =
            FPlatformTime::Seconds() - State->StartedSeconds;
        if (Elapsed > 12.0)
        {
            Test->AddError(FString::Printf(
                TEXT("Tatra did not complete inertial seam crossing in 12s: X=%.1f speed=%.1f gear=%d contacts=%d"),
                Pawn->GetActorLocation().X,
                Movement->GetForwardSpeed(),
                Movement->GetCurrentGear(),
                PinkCabL1EndlessRoadSeamTests::CountWheelContacts(*Movement)));
            return true;
        }

        const int32 Contacts =
            PinkCabL1EndlessRoadSeamTests::CountWheelContacts(*Movement);

        if (Contacts < 2)
        {
            return false;
        }

        const double X = Pawn->GetActorLocation().X;
        if (X <= 100100.0 || Streamer->GetCurrentChunkIndex() != 1)
        {
            return false;
        }

        const int32 ContactsAfter =
            PinkCabL1EndlessRoadSeamTests::CountWheelContacts(*Movement);

        Test->AddInfo(FString::Printf(
            TEXT("CD869_SEAM_CROSSING X=%.1f speed=%.1f gear=%d contacts=%d active=%d pool=%d chunk=%d"),
            X,
            Movement->GetForwardSpeed(),
            Movement->GetCurrentGear(),
            ContactsAfter,
            Streamer->GetActiveChunkCount(),
            Streamer->GetPoolSize(),
            Streamer->GetCurrentChunkIndex()));

        Test->TestTrue(TEXT("Tatra center crossed 1000m seam"), X > 100100.0);
        Test->TestTrue(TEXT("at least two Chaos wheels remain in contact after seam"),
            ContactsAfter >= 2);
        Test->TestEqual(TEXT("streamer advanced exactly to logical chunk one"),
            Streamer->GetCurrentChunkIndex(), 1);
        Test->TestEqual(TEXT("streamer still has seven active chunks"),
            Streamer->GetActiveChunkCount(), 7);
        Test->TestEqual(TEXT("physical pool remains seven"),
            Streamer->GetPoolSize(), 7);
        Test->TestTrue(TEXT("post-seam active IDs remain unique"),
            PinkCabL1EndlessRoadSeamTests::HasUniqueActiveIds(*Streamer));
        Test->TestTrue(TEXT("chunk zero collision remains active behind car"),
            PinkCabL1EndlessRoadSeamTests::HasCollisionOnChunk(
                Streamer->FindActiveChunkActor(0)));
        Test->TestTrue(TEXT("chunk one collision remains active under car"),
            PinkCabL1EndlessRoadSeamTests::HasCollisionOnChunk(
                Streamer->FindActiveChunkActor(1)));

        FPinkCabVehicleControlState Stop;
        Stop.SetBrake(1.0f);
        Pawn->GetPinkCabDynamicsProvider().ApplyControls(Stop);
        return true;
    }

private:
    FAutomationTestBase* Test = nullptr;
    TSharedRef<PinkCabL1EndlessRoadSeamTests::FState> State;
};

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPinkCabL1EndlessRoadChaosSeamCrossingTest,
    "PinkCab.World.L1EndlessRoad.Physics.SeamCrossing",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPinkCabL1EndlessRoadChaosSeamCrossingTest::RunTest(
    const FString& Parameters)
{
    const bool bOpened = AutomationOpenMap(
        TEXT("/Game/Dev/Maps/L_PinkCab_L1_EndlessStraight"),
        true);
    TestTrue(TEXT("endless-road candidate map opens for PIE"), bOpened);
    if (!bOpened)
    {
        return false;
    }

    const TSharedRef<PinkCabL1EndlessRoadSeamTests::FState> State =
        MakeShared<PinkCabL1EndlessRoadSeamTests::FState>();
    ADD_LATENT_AUTOMATION_COMMAND(
        FPinkCabL1EndlessRoadSeamDriveCommand(this, State));
    return true;
}

#endif
