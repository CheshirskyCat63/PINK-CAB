#include "Interaction/PinkCabContractCabinPrimitive.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "UObject/ConstructorHelpers.h"

APinkCabContractCabinPrimitive::APinkCabContractCabinPrimitive()
{
    PrimaryActorTick.bCanEverTick = false;

    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    RootComponent = SceneRoot;

    ToggleMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Toggle"));
    ToggleMesh->SetupAttachment(SceneRoot);
    ToggleMesh->SetRelativeScale3D(FVector(0.35f, 0.12f, 0.08f));

    static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
    if (Cube.Succeeded())
    {
        ToggleMesh->SetStaticMesh(Cube.Object);
    }

    StateLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("StateLight"));
    StateLight->SetupAttachment(SceneRoot);
    StateLight->SetRelativeLocation(FVector(0.0f, 0.0f, 35.0f));
    StateLight->SetIntensity(500.0f);
    StateLight->SetVisibility(false);
}

void APinkCabContractCabinPrimitive::ApplyInteractionEvent(const FPinkCabInteractionEvent& Event)
{
    if (Event.TargetId != InteractionTargetId || Event.Gesture != EPinkCabInteractionGesture::WheelIncrement || Event.SignedValue == 0)
    {
        return;
    }

    bDeveloperStateOn = Event.SignedValue > 0;
    UpdateVisualState();
}

void APinkCabContractCabinPrimitive::UpdateVisualState()
{
    if (StateLight)
    {
        StateLight->SetVisibility(bDeveloperStateOn);
    }

    if (ToggleMesh)
    {
        ToggleMesh->SetRelativeRotation(FRotator(bDeveloperStateOn ? -25.0f : 25.0f, 0.0f, 0.0f));
    }
}
