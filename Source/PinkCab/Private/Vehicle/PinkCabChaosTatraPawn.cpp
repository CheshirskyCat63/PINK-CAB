#include "Vehicle/PinkCabChaosTatraPawn.h"

#include "Camera/CameraComponent.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Vehicle/PinkCabChaosWheelFront.h"
#include "Vehicle/PinkCabChaosWheelRear.h"
#include "Vehicle/PinkCabTatraProfile.h"

APinkCabChaosTatraPawn::APinkCabChaosTatraPawn()
{
    USkeletalMeshComponent* VehicleMesh = GetMesh();
    VehicleMesh->SetSimulatePhysics(true);
    VehicleMesh->SetCollisionProfileName(TEXT("Vehicle"));

    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(VehicleMesh);
    CameraBoom->TargetArmLength = 520.0f;
    CameraBoom->SocketOffset = FVector(0.0f, 0.0f, 150.0f);
    CameraBoom->bInheritPitch = false;
    CameraBoom->bInheritRoll = false;

    ChaseCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ChaseCamera"));
    ChaseCamera->SetupAttachment(CameraBoom);

    UChaosWheeledVehicleMovementComponent* Movement = GetChaosMovement();
    check(Movement);
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

UChaosWheeledVehicleMovementComponent* APinkCabChaosTatraPawn::GetChaosMovement() const
{
    return Cast<UChaosWheeledVehicleMovementComponent>(GetVehicleMovementComponent());
}
