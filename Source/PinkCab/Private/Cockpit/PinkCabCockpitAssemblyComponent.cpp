#include "Cockpit/PinkCabCockpitAssemblyComponent.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/Actor.h"
#include "UObject/ConstructorHelpers.h"

UPinkCabCockpitAssemblyComponent::UPinkCabCockpitAssemblyComponent()
{
    PrimaryComponentTick.bCanEverTick = false;

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(TEXT("/Engine/BasicShapes/Cube.Cube"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderFinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
    CubeMesh = CubeFinder.Succeeded() ? CubeFinder.Object : nullptr;
    CylinderMesh = CylinderFinder.Succeeded() ? CylinderFinder.Object : nullptr;
}

void UPinkCabCockpitAssemblyComponent::BeginPlay()
{
    Super::BeginPlay();
    BuildPrimitiveShell();
}

USceneComponent* UPinkCabCockpitAssemblyComponent::GetSlotComponent(const EPinkCabCockpitSlot Slot) const
{
    if (const TObjectPtr<USceneComponent>* Found = SlotComponents.Find(static_cast<uint8>(Slot)))
    {
        return Found->Get();
    }
    return nullptr;
}

void UPinkCabCockpitAssemblyComponent::RegisterExternalSlot(const EPinkCabCockpitSlot Slot, USceneComponent* Component)
{
    if (Component)
    {
        SlotComponents.Add(static_cast<uint8>(Slot), Component);
    }
}

UStaticMeshComponent* UPinkCabCockpitAssemblyComponent::AddPrimitive(
    const FName Name,
    UStaticMesh* Mesh,
    const FVector& Location,
    const FRotator& Rotation,
    const FVector& Scale,
    const EPinkCabCockpitSlot Slot,
    const bool bRegisterSlot)
{
    AActor* Owner = GetOwner();
    if (!Owner || !Mesh)
    {
        return nullptr;
    }

    UStaticMeshComponent* Component = NewObject<UStaticMeshComponent>(Owner, Name);
    Owner->AddInstanceComponent(Component);
    Component->SetupAttachment(this);
    Component->SetStaticMesh(Mesh);
    Component->SetRelativeLocation(Location);
    Component->SetRelativeRotation(Rotation);
    Component->SetRelativeScale3D(Scale);
    Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Component->SetGenerateOverlapEvents(false);
    Component->SetCastShadow(false);
    Component->RegisterComponent();

    if (bRegisterSlot)
    {
        SlotComponents.Add(static_cast<uint8>(Slot), Component);
    }
    return Component;
}

void UPinkCabCockpitAssemblyComponent::BuildPrimitiveShell()
{
    if (bBuilt)
    {
        return;
    }
    bBuilt = true;

    // Shell: intentionally open around the windshield so the primitive cabin never blocks the road view.
    AddPrimitive(TEXT("CabinFloor"), CubeMesh, FVector(-40.0f, 0.0f, 45.0f), FRotator::ZeroRotator,
        FVector(1.8f, 1.4f, 0.06f), EPinkCabCockpitSlot::Dashboard, false);
    AddPrimitive(TEXT("CabinDashboardShell"), CubeMesh, FVector(70.0f, 0.0f, 88.0f), FRotator::ZeroRotator,
        FVector(0.32f, 1.15f, 0.20f), EPinkCabCockpitSlot::Dashboard, false);
    AddPrimitive(TEXT("CabinRoofHeader"), CubeMesh, FVector(35.0f, 0.0f, 176.0f), FRotator::ZeroRotator,
        FVector(0.20f, 1.12f, 0.08f), EPinkCabCockpitSlot::Dashboard, false);
    AddPrimitive(TEXT("CabinAPillarLeft"), CubeMesh, FVector(50.0f, -92.0f, 135.0f), FRotator(0.0f, 0.0f, -12.0f),
        FVector(0.09f, 0.09f, 0.55f), EPinkCabCockpitSlot::Dashboard, false);
    AddPrimitive(TEXT("CabinAPillarRight"), CubeMesh, FVector(50.0f, 92.0f, 135.0f), FRotator(0.0f, 0.0f, 12.0f),
        FVector(0.09f, 0.09f, 0.55f), EPinkCabCockpitSlot::Dashboard, false);

    AddPrimitive(TEXT("CockpitDashboard"), CubeMesh, FVector(45.0f, -35.0f, 103.0f), FRotator::ZeroRotator,
        FVector(0.38f, 0.62f, 0.12f), EPinkCabCockpitSlot::Dashboard);
    AddPrimitive(TEXT("CockpitSteeringWheel"), CylinderMesh, FVector(14.0f, -39.0f, 113.0f), FRotator(0.0f, 90.0f, 0.0f),
        FVector(0.34f, 0.34f, 0.06f), EPinkCabCockpitSlot::SteeringWheel);
    AddPrimitive(TEXT("CockpitClutchPedal"), CubeMesh, FVector(24.0f, -61.0f, 58.0f), FRotator(18.0f, 0.0f, 0.0f),
        FVector(0.16f, 0.08f, 0.05f), EPinkCabCockpitSlot::ClutchPedal);
    AddPrimitive(TEXT("CockpitBrakePedal"), CubeMesh, FVector(27.0f, -43.0f, 58.0f), FRotator(18.0f, 0.0f, 0.0f),
        FVector(0.16f, 0.08f, 0.05f), EPinkCabCockpitSlot::BrakePedal);
    AddPrimitive(TEXT("CockpitThrottlePedal"), CubeMesh, FVector(30.0f, -25.0f, 58.0f), FRotator(18.0f, 0.0f, 0.0f),
        FVector(0.18f, 0.07f, 0.04f), EPinkCabCockpitSlot::ThrottlePedal);
    AddPrimitive(TEXT("CockpitGearbox"), CylinderMesh, FVector(-12.0f, 7.0f, 76.0f), FRotator::ZeroRotator,
        FVector(0.08f, 0.08f, 0.32f), EPinkCabCockpitSlot::Gearbox);
    AddPrimitive(TEXT("CockpitHandbrake"), CubeMesh, FVector(-35.0f, 18.0f, 69.0f), FRotator(0.0f, -18.0f, 0.0f),
        FVector(0.34f, 0.05f, 0.05f), EPinkCabCockpitSlot::Handbrake);
    AddPrimitive(TEXT("CockpitTaximeter"), CubeMesh, FVector(54.0f, 22.0f, 120.0f), FRotator::ZeroRotator,
        FVector(0.23f, 0.28f, 0.15f), EPinkCabCockpitSlot::Taximeter);
    AddPrimitive(TEXT("CockpitNavigation"), CubeMesh, FVector(51.0f, -3.0f, 112.0f), FRotator::ZeroRotator,
        FVector(0.10f, 0.23f, 0.17f), EPinkCabCockpitSlot::Navigation);
    AddPrimitive(TEXT("CockpitRadio"), CubeMesh, FVector(46.0f, 18.0f, 96.0f), FRotator::ZeroRotator,
        FVector(0.11f, 0.24f, 0.12f), EPinkCabCockpitSlot::Radio);
    AddPrimitive(TEXT("CockpitPassengerDoor"), CubeMesh, FVector(-5.0f, 82.0f, 91.0f), FRotator::ZeroRotator,
        FVector(0.16f, 0.06f, 0.08f), EPinkCabCockpitSlot::PassengerDoor);
    AddPrimitive(TEXT("CockpitIgnition"), CylinderMesh, FVector(38.0f, -18.0f, 96.0f), FRotator(90.0f, 0.0f, 0.0f),
        FVector(0.05f, 0.05f, 0.08f), EPinkCabCockpitSlot::Ignition);
    AddPrimitive(TEXT("CockpitTurnSignals"), CubeMesh, FVector(18.0f, -57.0f, 111.0f), FRotator::ZeroRotator,
        FVector(0.20f, 0.03f, 0.03f), EPinkCabCockpitSlot::TurnSignals);
    AddPrimitive(TEXT("CockpitHorn"), CubeMesh, FVector(11.0f, -39.0f, 113.0f), FRotator::ZeroRotator,
        FVector(0.09f, 0.09f, 0.04f), EPinkCabCockpitSlot::Horn);
    AddPrimitive(TEXT("CockpitLights"), CubeMesh, FVector(45.0f, -67.0f, 102.0f), FRotator::ZeroRotator,
        FVector(0.06f, 0.06f, 0.06f), EPinkCabCockpitSlot::Lights);
    AddPrimitive(TEXT("CockpitWipers"), CubeMesh, FVector(48.0f, -58.0f, 98.0f), FRotator::ZeroRotator,
        FVector(0.06f, 0.06f, 0.06f), EPinkCabCockpitSlot::Wipers);
    AddPrimitive(TEXT("CockpitWasher"), CubeMesh, FVector(48.0f, -48.0f, 98.0f), FRotator::ZeroRotator,
        FVector(0.06f, 0.06f, 0.06f), EPinkCabCockpitSlot::Washer);
    AddPrimitive(TEXT("CockpitWarnings"), CubeMesh, FVector(57.0f, -37.0f, 118.0f), FRotator::ZeroRotator,
        FVector(0.08f, 0.22f, 0.08f), EPinkCabCockpitSlot::Warnings);
    AddPrimitive(TEXT("CockpitRearViewMirror"), CubeMesh, FVector(10.0f, 0.0f, 162.0f), FRotator::ZeroRotator,
        FVector(0.05f, 0.30f, 0.11f), EPinkCabCockpitSlot::RearViewMirror);
    AddPrimitive(TEXT("CockpitLeftMirror"), CubeMesh, FVector(28.0f, -105.0f, 125.0f), FRotator::ZeroRotator,
        FVector(0.05f, 0.16f, 0.12f), EPinkCabCockpitSlot::LeftMirror);
    AddPrimitive(TEXT("CockpitRightMirror"), CubeMesh, FVector(28.0f, 105.0f, 125.0f), FRotator::ZeroRotator,
        FVector(0.05f, 0.16f, 0.12f), EPinkCabCockpitSlot::RightMirror);
}
