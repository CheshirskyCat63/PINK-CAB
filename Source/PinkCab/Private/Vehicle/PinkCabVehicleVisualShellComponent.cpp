#include "Vehicle/PinkCabVehicleVisualShellComponent.h"

#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/Actor.h"

namespace
{
void ConfigurePresentation(UPrimitiveComponent& Component)
{
    Component.SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Component.SetGenerateOverlapEvents(false);
}
}

UPinkCabVehicleVisualShellComponent::UPinkCabVehicleVisualShellComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UPinkCabVehicleVisualShellComponent::BeginPlay()
{
    Super::BeginPlay();
    RebuildPresentation();
}

void UPinkCabVehicleVisualShellComponent::DestroyPresentationComponent(
    TObjectPtr<UPrimitiveComponent>& Component)
{
    if (Component)
    {
        Component->DestroyComponent();
        Component = nullptr;
    }
}

UPrimitiveComponent* UPinkCabVehicleVisualShellComponent::BuildExterior()
{
    AActor* Owner = GetOwner();
    if (!Owner) return nullptr;

    if (!Profile.ExteriorStaticMesh.IsNull())
    {
        UStaticMesh* Mesh = Profile.ExteriorStaticMesh.LoadSynchronous();
        if (!Mesh) return nullptr;
        UStaticMeshComponent* Component = NewObject<UStaticMeshComponent>(Owner, TEXT("VehicleExteriorPresentation"));
        Owner->AddInstanceComponent(Component);
        Component->SetupAttachment(this);
        Component->SetStaticMesh(Mesh);
        Component->SetRelativeTransform(Profile.ExteriorTransform);
        Component->SetOwnerNoSee(true);
        ConfigurePresentation(*Component);
        Component->RegisterComponent();
        return Component;
    }

    if (!Profile.ExteriorSkeletalMesh.IsNull())
    {
        USkeletalMesh* Mesh = Profile.ExteriorSkeletalMesh.LoadSynchronous();
        if (!Mesh) return nullptr;
        USkeletalMeshComponent* Component = NewObject<USkeletalMeshComponent>(Owner, TEXT("VehicleExteriorPresentation"));
        Owner->AddInstanceComponent(Component);
        Component->SetupAttachment(this);
        Component->SetSkeletalMesh(Mesh);
        Component->SetRelativeTransform(Profile.ExteriorTransform);
        Component->SetOwnerNoSee(true);
        ConfigurePresentation(*Component);
        Component->RegisterComponent();
        return Component;
    }
    return nullptr;
}

UPrimitiveComponent* UPinkCabVehicleVisualShellComponent::BuildCabin()
{
    AActor* Owner = GetOwner();
    if (!Owner) return nullptr;

    if (!Profile.CabinStaticMesh.IsNull())
    {
        UStaticMesh* Mesh = Profile.CabinStaticMesh.LoadSynchronous();
        if (!Mesh) return nullptr;
        UStaticMeshComponent* Component = NewObject<UStaticMeshComponent>(Owner, TEXT("VehicleCabinPresentation"));
        Owner->AddInstanceComponent(Component);
        Component->SetupAttachment(this);
        Component->SetStaticMesh(Mesh);
        Component->SetRelativeTransform(Profile.CabinTransform);
        Component->SetOnlyOwnerSee(true);
        ConfigurePresentation(*Component);
        Component->RegisterComponent();
        return Component;
    }

    if (!Profile.CabinSkeletalMesh.IsNull())
    {
        USkeletalMesh* Mesh = Profile.CabinSkeletalMesh.LoadSynchronous();
        if (!Mesh) return nullptr;
        USkeletalMeshComponent* Component = NewObject<USkeletalMeshComponent>(Owner, TEXT("VehicleCabinPresentation"));
        Owner->AddInstanceComponent(Component);
        Component->SetupAttachment(this);
        Component->SetSkeletalMesh(Mesh);
        Component->SetRelativeTransform(Profile.CabinTransform);
        Component->SetOnlyOwnerSee(true);
        ConfigurePresentation(*Component);
        Component->RegisterComponent();
        return Component;
    }
    return nullptr;
}

bool UPinkCabVehicleVisualShellComponent::ApplyProfile(const FPinkCabVehicleVisualProfile& InProfile)
{
    if (!InProfile.IsValid())
    {
        return false;
    }
    Profile = InProfile;
    return !IsRegistered() || RebuildPresentation();
}

bool UPinkCabVehicleVisualShellComponent::RebuildPresentation()
{
    DestroyPresentationComponent(ExteriorPresentation);
    DestroyPresentationComponent(CabinPresentation);
    if (!Profile.IsValid()) return false;

    ExteriorPresentation = BuildExterior();
    CabinPresentation = BuildCabin();
    return true;
}
