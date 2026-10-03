#include "Vehicle/PinkCabPhysicsFixturePawn.h"

#include "ChaosWheeledVehicleMovementComponent.h"
#include "ChaosVehicleWheel.h"
#include "Components/BoxComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "EngineUtils.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "Vehicle/PinkCabChaosPhysicalProfile.h"
#include "Vehicle/PinkCabChaosVehicleMovementComponent.h"
#include "Vehicle/PinkCabChaosWheelFront.h"
#include "Vehicle/PinkCabChaosWheelRear.h"

namespace
{
const FName FixtureFloorTag(TEXT("PinkCab.PhysicsFixture.Floor"));
}

APinkCabPhysicsFixturePawn::APinkCabPhysicsFixturePawn(
    const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer.SetDefaultSubobjectClass<
        UPinkCabChaosVehicleMovementComponent>(
            AWheeledVehiclePawn::VehicleMovementComponentName))
{
    PrimaryActorTick.bCanEverTick = false;
    AutoPossessPlayer = EAutoReceiveInput::Disabled;

    USkeletalMeshComponent* VehicleMesh = GetMesh();
    check(VehicleMesh);

    if (USkeletalMesh* MeshAsset = LoadObject<USkeletalMesh>(
            nullptr,
            TEXT("/Game/Vehicles/SportsCar/SKM_SportsCar.SKM_SportsCar")))
    {
        VehicleMesh->SetSkeletalMesh(MeshAsset);
    }
    if (UPhysicsAsset* PhysicsAsset = LoadObject<UPhysicsAsset>(
            nullptr,
            TEXT("/Game/Vehicles/SportsCar/PA_SportsCar.PA_SportsCar")))
    {
        VehicleMesh->SetPhysicsAsset(PhysicsAsset, false);
    }
    VehicleMesh->SetCollisionProfileName(TEXT("Vehicle"));
    VehicleMesh->SetSimulatePhysics(true);
    VehicleMesh->SetGenerateOverlapEvents(false);

    UChaosWheeledVehicleMovementComponent* Movement = GetChaosMovement();
    check(Movement);

    // The sterile automation pawn is intentionally not possessed. UE Chaos
    // defaults to a controller-gated input path for player pawns, so direct
    // authoritative test commands must explicitly opt into controllerless
    // input. This changes only fixture lifecycle, not vehicle physics.
    Movement->SetRequiresControllerForInputs(false);

    const FPinkCabChaosPhysicalProfile Profile =
        FPinkCabChaosPhysicalProfile::ForVariant(
            EPinkCabCalibrationVariant::Nominal);
    Profile.ApplyToMovement(*Movement);

    Movement->WheelSetups.SetNum(4);
    Movement->WheelSetups[0].WheelClass = UPinkCabChaosWheelFront::StaticClass();
    Movement->WheelSetups[0].BoneName = TEXT("Phys_Wheel_FL");
    Movement->WheelSetups[1].WheelClass = UPinkCabChaosWheelFront::StaticClass();
    Movement->WheelSetups[1].BoneName = TEXT("Phys_Wheel_FR");
    Movement->WheelSetups[2].WheelClass = UPinkCabChaosWheelRear::StaticClass();
    Movement->WheelSetups[2].BoneName = TEXT("Phys_Wheel_BL");
    Movement->WheelSetups[3].WheelClass = UPinkCabChaosWheelRear::StaticClass();
    Movement->WheelSetups[3].BoneName = TEXT("Phys_Wheel_BR");

    for (FChaosWheelSetup& Setup : Movement->WheelSetups)
    {
        Setup.AdditionalOffset = FVector::ZeroVector;
    }
}

UChaosWheeledVehicleMovementComponent*
APinkCabPhysicsFixturePawn::GetChaosMovement() const
{
    return Cast<UChaosWheeledVehicleMovementComponent>(
        GetVehicleMovementComponent());
}

void FPinkCabPhysicsFixtureRestGate::Reset()
{
    Observation = {};
    StartMechanicalStep = -1;
    LastMechanicalStep = -1;
    ElapsedMechanicalSeconds = 0.0;
    StableMechanicalSteps = 0;
}

bool FPinkCabPhysicsFixtureRestGate::Update(
    APinkCabPhysicsFixturePawn& Pawn)
{
    UChaosWheeledVehicleMovementComponent* Movement =
        Pawn.GetChaosMovement();
    UPinkCabChaosVehicleMovementComponent* PinkCabMovement =
        Cast<UPinkCabChaosVehicleMovementComponent>(Movement);
    USkeletalMeshComponent* Mesh = Pawn.GetMesh();
    if (!Movement || !PinkCabMovement || !Mesh)
    {
        StableMechanicalSteps = 0;
        return false;
    }

    const int64 MechanicalStep =
        PinkCabMovement->GetPinkCabMechanicalIntegrationStepCount();
    if (StartMechanicalStep < 0)
    {
        StartMechanicalStep = MechanicalStep;
        LastMechanicalStep = MechanicalStep;
        return false;
    }
    if (MechanicalStep == LastMechanicalStep)
    {
        return false;
    }

    // A timeout is a duration, not a fixed number of substeps. The same
    // settled-state predicates below still gate every measured case.
    ElapsedMechanicalSeconds += static_cast<double>(MechanicalStep - LastMechanicalStep)
        * static_cast<double>(PinkCabMovement->GetPinkCabLastMechanicalIntegrationDeltaSeconds());
    LastMechanicalStep = MechanicalStep;

    Observation = {};
    Observation.ElapsedMechanicalSeconds = ElapsedMechanicalSeconds;
    Observation.MechanicalStep = MechanicalStep;
    Observation.ElapsedMechanicalSteps =
        MechanicalStep - StartMechanicalStep;
    Observation.EngineRpm = Movement->GetEngineRotationSpeed();
    Observation.TargetIdleRpm = Movement->EngineSetup.EngineIdleRPM;
    Observation.BodyLinearSpeedCmPerSec =
        Mesh->GetPhysicsLinearVelocity().Size();
    Observation.BodyAngularSpeedDegPerSec =
        Mesh->GetPhysicsAngularVelocityInDegrees().Size();
    Observation.NativeCurrentGear = Movement->GetCurrentGear();
    Observation.NativeTargetGear = Movement->GetTargetGear();

    for (const UChaosVehicleWheel* Wheel : Movement->Wheels)
    {
        if (!Wheel || !Wheel->bAffectedByEngine)
        {
            continue;
        }
        Observation.MaxDrivenWheelRpm = FMath::Max(
            Observation.MaxDrivenWheelRpm,
            FMath::Abs(
                Wheel->GetWheelAngularVelocity()
                * (60.0f / (2.0f * PI))));
    }

    // A fresh vehicle is spawned above the floor. Engine RPM + wheel RPM alone
    // are therefore not a valid reset proof: those values are already quiet
    // while the chassis is still in free fall. Require enough completed
    // mechanical integrations for the fall/contact transient to occur, then
    // require five consecutive fully observed stable snapshots.
    static constexpr int64 MinimumMechanicalStepsBeforeRest = 20;
    static constexpr int32 RequiredStableMechanicalSteps = 5;
    static constexpr float EngineIdleToleranceRpm = 30.0f;
    static constexpr float DrivenWheelToleranceRpm = 2.0f;
    static constexpr float BodyLinearToleranceCmPerSec = 5.0f;
    static constexpr float BodyAngularToleranceDegPerSec = 2.0f;

    const bool bRestCandidate =
        Observation.ElapsedMechanicalSteps
            >= MinimumMechanicalStepsBeforeRest
        && FMath::Abs(
            Observation.EngineRpm - Observation.TargetIdleRpm)
            <= EngineIdleToleranceRpm
        && Observation.MaxDrivenWheelRpm <= DrivenWheelToleranceRpm
        && Observation.BodyLinearSpeedCmPerSec
            <= BodyLinearToleranceCmPerSec
        && Observation.BodyAngularSpeedDegPerSec
            <= BodyAngularToleranceDegPerSec
        && Observation.NativeCurrentGear == 0
        && Observation.NativeTargetGear == 0;

    if (bRestCandidate)
    {
        // With fixed async physics the game thread can legitimately observe
        // several completed physics steps at once. Require five consecutive
        // stable observations rather than pretending every physics step was
        // individually visible to the automation thread.
        ++StableMechanicalSteps;
    }
    else
    {
        StableMechanicalSteps = 0;
    }

    return StableMechanicalSteps >= RequiredStableMechanicalSteps;
}

APinkCabPhysicsFixturePawn* PinkCabPhysicsFixture::FindOrSpawnPawn(
    UWorld& World)
{
    for (TActorIterator<APinkCabPhysicsFixturePawn> It(&World); It; ++It)
    {
        if (!It->IsActorBeingDestroyed())
        {
            return *It;
        }
    }

    return PinkCabPhysicsFixture::SpawnFreshPawn(
        World,
        FVector(0.0f, 0.0f, 120.0f),
        FRotator::ZeroRotator);
}

APinkCabPhysicsFixturePawn* PinkCabPhysicsFixture::SpawnFreshPawn(
    UWorld& World,
    const FVector& Location,
    const FRotator& Rotation)
{
    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride =
        ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    APinkCabPhysicsFixturePawn* Pawn =
        World.SpawnActor<APinkCabPhysicsFixturePawn>(
            Location,
            Rotation,
            Params);
    if (Pawn)
    {
        PinkCabPhysicsFixture::KeepAwake(*Pawn);
    }
    return Pawn;
}

void PinkCabPhysicsFixture::KeepAwake(
    APinkCabPhysicsFixturePawn& Pawn)
{
    if (UChaosWheeledVehicleMovementComponent* Movement =
            Pawn.GetChaosMovement())
    {
        // Direct automation commands bypass the normal player-input loop that
        // wakes a sleeping Chaos vehicle. Keep the sterile fixture simulated
        // without altering forces, velocities, engine state or production code.
        Movement->SetSleeping(false);
    }
    if (USkeletalMeshComponent* Mesh = Pawn.GetMesh())
    {
        Mesh->WakeAllRigidBodies();
    }
}

void PinkCabPhysicsFixture::DestroyPawns(UWorld& World)
{
    TArray<APinkCabPhysicsFixturePawn*> Existing;
    for (TActorIterator<APinkCabPhysicsFixturePawn> It(&World); It; ++It)
    {
        if (!It->IsActorBeingDestroyed())
        {
            Existing.Add(*It);
        }
    }
    for (APinkCabPhysicsFixturePawn* Pawn : Existing)
    {
        Pawn->Destroy();
    }
}

AActor* PinkCabPhysicsFixture::FindOrSpawnFlatFloor(UWorld& World)
{
    for (TActorIterator<AActor> It(&World); It; ++It)
    {
        if (It->ActorHasTag(FixtureFloorTag))
        {
            return *It;
        }
    }

    AActor* Floor = World.SpawnActor<AActor>();
    if (!Floor)
    {
        return nullptr;
    }
    Floor->Tags.Add(FixtureFloorTag);

    UBoxComponent* Box = NewObject<UBoxComponent>(Floor);
    Floor->SetRootComponent(Box);
    Box->SetBoxExtent(FVector(5000.0f, 2500.0f, 50.0f));
    Box->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    Box->SetCollisionObjectType(ECC_WorldStatic);
    Box->SetCollisionResponseToAllChannels(ECR_Block);
    Box->SetGenerateOverlapEvents(false);
    Box->RegisterComponent();
    Floor->SetActorLocation(FVector(0.0f, 0.0f, -50.0f));
    Box->RecreatePhysicsState();
    return Floor;
}
