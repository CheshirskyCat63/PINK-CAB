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

UPrimitiveComponent* UPinkCabVehicleVisualShellComponent::GetExteriorPresentation() const
{
    if (ExteriorPresentation) return ExteriorPresentation.Get();
    return PresentationPartComponents.Num() > 0 ? PresentationPartComponents[0].Get() : nullptr;
}

UPrimitiveComponent* UPinkCabVehicleVisualShellComponent::GetCabinPresentation() const
{
    if (CabinPresentation) return CabinPresentation.Get();
    return PresentationPartComponents.Num() > 0 ? PresentationPartComponents[0].Get() : nullptr;
}

UStaticMeshComponent* UPinkCabVehicleVisualShellComponent::GetPresentationPartComponent(const FName PartId) const
{
    const int32 Index = Profile.PresentationParts.IndexOfByPredicate([PartId](const FPinkCabVehiclePresentationPart& Part)
    {
        return Part.PartId == PartId;
    });
    return PresentationPartComponents.IsValidIndex(Index)
        ? PresentationPartComponents[Index].Get()
        : nullptr;
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

void UPinkCabVehicleVisualShellComponent::DestroyPresentationParts()
{
    for (UStaticMeshComponent* Component : PresentationPartComponents)
    {
        if (Component) Component->DestroyComponent();
    }
    PresentationPartComponents.Reset();
}

bool UPinkCabVehicleVisualShellComponent::BuildPresentationParts()
{
    AActor* Owner = GetOwner();
    if (!Owner) return Profile.PresentationParts.IsEmpty();

    for (const FPinkCabVehiclePresentationPart& Part : Profile.PresentationParts)
    {
        UStaticMesh* Mesh = Part.Mesh.LoadSynchronous();
        if (!Mesh) return false;
        UStaticMeshComponent* Component = NewObject<UStaticMeshComponent>(Owner);
        Owner->AddInstanceComponent(Component);
        Component->SetupAttachment(this);
        Component->SetMobility(EComponentMobility::Movable);
        Component->SetStaticMesh(Mesh);
        Component->SetRelativeTransform(Part.LocalTransform);
        Component->SetOwnerNoSee(Part.bOwnerNoSee);
        Component->SetOnlyOwnerSee(Part.bOnlyOwnerSee);
        ConfigurePresentation(*Component);
        Component->RegisterComponent();
        PresentationPartComponents.Add(Component);
    }
    return true;
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
    DestroyPresentationParts();
    if (!Profile.IsValid()) return false;

    ExteriorPresentation = BuildExterior();
    CabinPresentation = BuildCabin();
    const bool bPartsReady = BuildPresentationParts();
    const bool bExteriorReady = !Profile.HasExteriorAsset() || ExteriorPresentation != nullptr;
    const bool bCabinReady = !Profile.HasCabinAsset() || CabinPresentation != nullptr;
    return bExteriorReady && bCabinReady && bPartsReady;
}
