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
    UStaticMesh* StaticMesh = nullptr;
    USkeletalMesh* SkeletalMesh = nullptr;
    FTransform Transform = Profile.CabinTransform;
    if (!Profile.CabinStaticMesh.IsNull()) StaticMesh = Profile.CabinStaticMesh.LoadSynchronous();
    else if (!Profile.CabinSkeletalMesh.IsNull()) SkeletalMesh = Profile.CabinSkeletalMesh.LoadSynchronous();
    else if (Profile.bUseExteriorAsCabinWhenCabinMissing)
    {
        Transform = Profile.ExteriorTransform;
        if (!Profile.ExteriorStaticMesh.IsNull()) StaticMesh = Profile.ExteriorStaticMesh.LoadSynchronous();
        else if (!Profile.ExteriorSkeletalMesh.IsNull()) SkeletalMesh = Profile.ExteriorSkeletalMesh.LoadSynchronous();
    }
    if (StaticMesh)
    {
        UStaticMeshComponent* Component = NewObject<UStaticMeshComponent>(Owner, TEXT("VehicleCabinPresentation"));
        Owner->AddInstanceComponent(Component); Component->SetupAttachment(this);
        Component->SetStaticMesh(StaticMesh); Component->SetRelativeTransform(Transform);
        Component->SetOnlyOwnerSee(true); ConfigurePresentation(*Component); Component->RegisterComponent();
        return Component;
    }
    if (SkeletalMesh)
    {
        USkeletalMeshComponent* Component = NewObject<USkeletalMeshComponent>(Owner, TEXT("VehicleCabinPresentation"));
        Owner->AddInstanceComponent(Component); Component->SetupAttachment(this);
        Component->SetSkeletalMesh(SkeletalMesh); Component->SetRelativeTransform(Transform);
        Component->SetOnlyOwnerSee(true); ConfigurePresentation(*Component); Component->RegisterComponent();
        return Component;
    }
    return nullptr;
}

bool UPinkCabVehicleVisualShellComponent::ApplyProfile(const FPinkCabVehicleVisualProfile& InProfile)
{
    if (!InProfile.IsValid()) return false;
    const FPinkCabVehicleVisualProfile Previous = Profile;
    Profile = InProfile;
    if (!IsRegistered() || RebuildPresentation()) return true;
    Profile = Previous;
    RebuildPresentation();
    return false;
}

bool UPinkCabVehicleVisualShellComponent::RebuildPresentation()
{
    DestroyPresentationComponent(ExteriorPresentation);
    DestroyPresentationComponent(CabinPresentation);
    if (!Profile.IsValid()) return false;

    ExteriorPresentation = BuildExterior();
    CabinPresentation = BuildCabin();
    const bool bExteriorReady = !Profile.HasExteriorAsset() || ExteriorPresentation != nullptr;
    const bool bCabinReady = !Profile.HasCabinAsset() || CabinPresentation != nullptr;
    return bExteriorReady && bCabinReady;
}
