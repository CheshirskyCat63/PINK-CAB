#include "Vehicle/PinkCabPhysicsFixturePawn.h"

#include "ChaosWheeledVehicleMovementComponent.h"
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

    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride =
        ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    APinkCabPhysicsFixturePawn* Pawn =
        World.SpawnActor<APinkCabPhysicsFixturePawn>(
            FVector(0.0f, 0.0f, 120.0f),
            FRotator::ZeroRotator,
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
