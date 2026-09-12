#include "Vehicle/PinkCabChaosTatraPawn.h"

#include "Camera/CameraComponent.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputCoreTypes.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "UObject/ConstructorHelpers.h"
#include "Vehicle/PinkCabChaosWheelFront.h"
#include "Vehicle/PinkCabChaosWheelRear.h"
#include "Vehicle/PinkCabTatraProfile.h"

APinkCabChaosTatraPawn::APinkCabChaosTatraPawn()
{
    PrimaryActorTick.bCanEverTick = true;
    AutoPossessPlayer = EAutoReceiveInput::Player0;

    USkeletalMeshComponent* VehicleMesh = GetMesh();
    static ConstructorHelpers::FObjectFinder<USkeletalMesh> TemplateMesh(
        TEXT("/Game/SportsCar/SKM_SportsCar.SKM_SportsCar"));
    static ConstructorHelpers::FObjectFinder<UPhysicsAsset> TemplatePhysicsAsset(
        TEXT("/Game/SportsCar/PA_SportsCar.PA_SportsCar"));

    if (TemplateMesh.Succeeded())
    {
        VehicleMesh->SetSkeletalMesh(TemplateMesh.Object);
    }
    if (TemplatePhysicsAsset.Succeeded())
    {
        VehicleMesh->SetPhysicsAsset(TemplatePhysicsAsset.Object, false);
    }

    VehicleMesh->SetCollisionProfileName(TEXT("Vehicle"));
    VehicleMesh->SetSimulatePhysics(true);

    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(VehicleMesh);
    CameraBoom->TargetArmLength = 520.0f;
    CameraBoom->SocketOffset = FVector(0.0f, 0.0f, 150.0f);
    CameraBoom->bDoCollisionTest = false;
    CameraBoom->bInheritPitch = false;
    CameraBoom->bInheritRoll = false;

    ChaseCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ChaseCamera"));
    ChaseCamera->SetupAttachment(CameraBoom);

    UChaosWheeledVehicleMovementComponent* Movement = GetChaosMovement();
    check(Movement);
    DynamicsProvider = FPinkCabChaosVehicleDynamicsProvider(Movement);

    const FPinkCabTatraProfile Profile = FPinkCabTatraProfile::Canonical();
    Movement->Mass = Profile.GetReferenceCrewMassKg();
    Movement->DifferentialSetup.DifferentialType = EVehicleDifferential::RearWheelDrive;

    Movement->TorqueControl.Enabled = false;
    Movement->TargetRotationControl.Enabled = false;
    Movement->StabilizeControl.Enabled = false;

    Movement->WheelSetups.SetNum(4);
    Movement->WheelSetups[0].WheelClass = UPinkCabChaosWheelFront::StaticClass();
    Movement->WheelSetups[0].BoneName = TEXT("Phys_Wheel_FL");
    Movement->WheelSetups[1].WheelClass = UPinkCabChaosWheelFront::StaticClass();
    Movement->WheelSetups[1].BoneName = TEXT("Phys_Wheel_FR");
    Movement->WheelSetups[2].WheelClass = UPinkCabChaosWheelRear::StaticClass();
    Movement->WheelSetups[2].BoneName = TEXT("Phys_Wheel_BL");
    Movement->WheelSetups[3].WheelClass = UPinkCabChaosWheelRear::StaticClass();
    Movement->WheelSetups[3].BoneName = TEXT("Phys_Wheel_BR");

    // CALIBRATION seeds for the first stock-Chaos drive only.
    Movement->EngineSetup.MaxTorque = 240.0f;
    Movement->EngineSetup.MaxRPM = 6000.0f;
    Movement->EngineSetup.EngineIdleRPM = 750.0f;
    Movement->EngineSetup.EngineBrakeEffect = 0.15f;
    Movement->TransmissionSetup.bUseAutomaticGears = true;
    Movement->TransmissionSetup.bUseAutoReverse = true;
    Movement->TransmissionSetup.FinalRatio = 3.2f;
    Movement->TransmissionSetup.ForwardGearRatios = {3.8f, 2.2f, 1.5f, 1.1f, 0.85f};
    Movement->TransmissionSetup.ReverseGearRatios = {3.5f};
    Movement->SteeringSetup.SteeringType = ESteeringType::Ackermann;
    Movement->SteeringSetup.AngleRatio = 0.72f;
}

void APinkCabChaosTatraPawn::BeginPlay()
{
    Super::BeginPlay();
    DynamicsProvider = FPinkCabChaosVehicleDynamicsProvider(GetChaosMovement());
}

float APinkCabChaosTatraPawn::IntegrateMouseSteering(
    const float CurrentSteering,
    const float DeltaX,
    const bool bGazeHeld,
    const float Gain)
{
    return bGazeHeld ? CurrentSteering : FMath::Clamp(CurrentSteering + DeltaX * Gain, -1.0f, 1.0f);
}

void APinkCabChaosTatraPawn::ApplyMouseSteeringDelta(const float DeltaX, const bool bGazeHeld)
{
    SteeringCommand = IntegrateMouseSteering(SteeringCommand, DeltaX, bGazeHeld, MouseSteeringGain);
    ControlState.SetSteering(SteeringCommand);
}

void APinkCabChaosTatraPawn::Tick(const float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    APlayerController* PC = Cast<APlayerController>(GetController());
    if (!PC)
    {
        return;
    }

    float MouseX = 0.0f;
    float MouseY = 0.0f;
    PC->GetInputMouseDelta(MouseX, MouseY);
    const bool bGazeHeld = PC->IsInputKeyDown(EKeys::SpaceBar);
    ApplyMouseSteeringDelta(MouseX, bGazeHeld);

    if (bGazeHeld)
    {
        LookYaw = FMath::Clamp(LookYaw + MouseX * 0.45f, -110.0f, 110.0f);
        LookPitch = FMath::Clamp(LookPitch - MouseY * 0.35f, -45.0f, 35.0f);
    }
    else
    {
        LookYaw = FMath::FInterpTo(LookYaw, 0.0f, DeltaSeconds, 4.0f);
        LookPitch = FMath::FInterpTo(LookPitch, 0.0f, DeltaSeconds, 4.0f);
    }

    CameraBoom->SetRelativeRotation(FRotator(LookPitch, LookYaw, 0.0f));

    ControlState.SetThrottle(PC->IsInputKeyDown(EKeys::W) ? 1.0f : 0.0f);
    ControlState.SetBrake(PC->IsInputKeyDown(EKeys::S) ? 1.0f : 0.0f);
    ControlState.SetHandbrake(PC->IsInputKeyDown(EKeys::LeftShift) ? 1.0f : 0.0f);
    DynamicsProvider.ApplyControls(ControlState);

    if (PC->WasInputKeyJustPressed(EKeys::R))
    {
        const FVector ResetLocation = GetActorLocation() + FVector(0.0f, 0.0f, 80.0f);
        const FRotator ResetRotation(0.0f, GetActorRotation().Yaw, 0.0f);
        SetActorTransform(FTransform(ResetRotation, ResetLocation), false, nullptr, ETeleportType::TeleportPhysics);
        GetMesh()->SetPhysicsLinearVelocity(FVector::ZeroVector);
        GetMesh()->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
    }
}

UChaosWheeledVehicleMovementComponent* APinkCabChaosTatraPawn::GetChaosMovement() const
{
    return Cast<UChaosWheeledVehicleMovementComponent>(GetVehicleMovementComponent());
}
