#include "Cockpit/PinkCabCockpitAssemblyComponent.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/Actor.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
void ApplyDefaultInteractionMetadata(FPinkCabCockpitSlotDefinition& Definition)
{
    const FPinkCabInteractionControlSpec Spec = PinkCabInteractionSpecForTargetId(Definition.StableId);
    Definition.bSupportsGrip = Spec.bSupportsGrip;
    Definition.bSupportsMomentary = Spec.bSupportsMomentary;
    Definition.bSupportsWheel = Spec.bSupportsWheel;
}
}


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
    IndexConfiguredSlots();
    BuildPrimitiveShell();
    CaptureVisualBaseline();
}


void UPinkCabCockpitAssemblyComponent::CaptureVisualBaseline()
{
    if (bVisualBaselineCaptured) return;
    BaselineTransforms.Reset();
    BaselineMeshes.Reset();
    BaselineMaterials.Reset();
    BaselineHiddenInGame.Reset();
    for (const TPair<uint8, TObjectPtr<USceneComponent>>& Pair : SlotComponents)
    {
        USceneComponent* Component = Pair.Value.Get();
        if (!Component) continue;
        BaselineTransforms.Add(Pair.Key, Component->GetRelativeTransform());
        if (UStaticMeshComponent* StaticMesh = Cast<UStaticMeshComponent>(Component))
        {
            BaselineMeshes.Add(Pair.Key, StaticMesh->GetStaticMesh());
            BaselineMaterials.Add(Pair.Key, StaticMesh->GetMaterial(0));
            BaselineHiddenInGame.Add(Pair.Key, StaticMesh->bHiddenInGame);
        }
    }
    bVisualBaselineCaptured = true;
}

void UPinkCabCockpitAssemblyComponent::IndexConfiguredSlots()
{
    SlotDefinitions.Reset();
    for (FPinkCabCockpitSlotDefinition& Definition : SlotConfiguration)
    {
        if (Definition.StableId.IsNone())
        {
            Definition.StableId = PinkCabCockpitSlotId(Definition.Slot);
        }
        SlotDefinitions.Add(static_cast<uint8>(Definition.Slot), Definition);
    }
}

USceneComponent* UPinkCabCockpitAssemblyComponent::GetSlotComponent(const EPinkCabCockpitSlot Slot) const
{
    if (const TObjectPtr<USceneComponent>* Found = SlotComponents.Find(static_cast<uint8>(Slot)))
    {
        return Found->Get();
    }
    return nullptr;
}

const FPinkCabCockpitSlotDefinition* UPinkCabCockpitAssemblyComponent::GetSlotDefinition(const EPinkCabCockpitSlot Slot) const
{
    return SlotDefinitions.Find(static_cast<uint8>(Slot));
}

void UPinkCabCockpitAssemblyComponent::ConfigureSlotDefinition(const FPinkCabCockpitSlotDefinition& InDefinition)
{
    FPinkCabCockpitSlotDefinition Definition = InDefinition;
    if (Definition.StableId.IsNone())
    {
        Definition.StableId = PinkCabCockpitSlotId(Definition.Slot);
    }
    const uint8 Key = static_cast<uint8>(Definition.Slot);
    SlotDefinitions.Add(Key, Definition);
    const int32 Existing = SlotConfiguration.IndexOfByPredicate([Key](const FPinkCabCockpitSlotDefinition& Item)
    {
        return static_cast<uint8>(Item.Slot) == Key;
    });
    if (Existing == INDEX_NONE) SlotConfiguration.Add(Definition);
    else SlotConfiguration[Existing] = Definition;

    if (USceneComponent* Component = GetSlotComponent(Definition.Slot))
    {
        Component->SetRelativeTransform(Definition.LocalTransform);
    }
}

void UPinkCabCockpitAssemblyComponent::RegisterExternalSlot(const EPinkCabCockpitSlot Slot, USceneComponent* Component)
{
    if (!Component) return;
    const uint8 Key = static_cast<uint8>(Slot);
    SlotComponents.Add(Key, Component);
    if (!SlotDefinitions.Contains(Key))
    {
        FPinkCabCockpitSlotDefinition Definition(Slot, PinkCabCockpitSlotId(Slot));
        Definition.LocalTransform = Component->GetRelativeTransform();
        ApplyDefaultInteractionMetadata(Definition);
        SlotDefinitions.Add(Key, Definition);
    }
}


void UPinkCabCockpitAssemblyComponent::ResetVisualBindings()
{
    if (!bVisualBaselineCaptured) CaptureVisualBaseline();
    for (const TPair<uint8, FTransform>& Pair : BaselineTransforms)
    {
        if (TObjectPtr<USceneComponent>* Found = SlotComponents.Find(Pair.Key))
        {
            if (USceneComponent* Component = Found->Get()) Component->SetRelativeTransform(Pair.Value);
        }
    }
    for (const TPair<uint8, TObjectPtr<UStaticMesh>>& Pair : BaselineMeshes)
    {
        if (TObjectPtr<USceneComponent>* Found = SlotComponents.Find(Pair.Key))
        {
            if (UStaticMeshComponent* StaticMesh = Cast<UStaticMeshComponent>(Found->Get()))
            {
                StaticMesh->SetStaticMesh(Pair.Value.Get());
                if (TObjectPtr<UMaterialInterface>* Material = BaselineMaterials.Find(Pair.Key))
                    StaticMesh->SetMaterial(0, Material->Get());
                StaticMesh->SetHiddenInGame(BaselineHiddenInGame.FindRef(Pair.Key));
            }
        }
    }
}

bool UPinkCabCockpitAssemblyComponent::ApplyVisualBindings(
    TConstArrayView<FPinkCabCockpitVisualBinding> Bindings)
{
    if (!FPinkCabCockpitVisualBinding::ValidateUnique(Bindings)) return false;
    if (!bVisualBaselineCaptured) CaptureVisualBaseline();
    for (const FPinkCabCockpitVisualBinding& Binding : Bindings)
    {
        USceneComponent* Component = GetSlotComponent(Binding.Slot);
        if (!Component) return false;
        if ((!Binding.MeshOverride.IsNull() || !Binding.MaterialOverride.IsNull())
            && !Cast<UStaticMeshComponent>(Component)) return false;
        if (!Binding.MeshOverride.IsNull() && !Binding.MeshOverride.LoadSynchronous()) return false;
        if (!Binding.MaterialOverride.IsNull() && !Binding.MaterialOverride.LoadSynchronous()) return false;
    }
    ResetVisualBindings();
    for (const FPinkCabCockpitVisualBinding& Binding : Bindings)
    {
        USceneComponent* Component = GetSlotComponent(Binding.Slot);
        Component->SetRelativeTransform(Binding.LocalTransform);
        if (UStaticMeshComponent* StaticMesh = Cast<UStaticMeshComponent>(Component))
        {
            if (!Binding.MeshOverride.IsNull()) StaticMesh->SetStaticMesh(Binding.MeshOverride.Get());
            if (!Binding.MaterialOverride.IsNull()) StaticMesh->SetMaterial(0, Binding.MaterialOverride.Get());
            StaticMesh->SetHiddenInGame(!Binding.bShowAnchorMesh);
        }
    }
    return true;
}

FName UPinkCabCockpitAssemblyComponent::ResolveGazeTarget(
    const FVector& WorldOrigin,
    const FVector& WorldForward,
    const float MaxDistanceCm,
    const int32 MaxCandidates) const
{
    if (MaxCandidates <= 0 || MaxDistanceCm <= 0.0f) return NAME_None;
    const FVector Forward = WorldForward.GetSafeNormal();
    if (Forward.IsNearlyZero()) return NAME_None;

    TArray<FPinkCabInteractionCandidate> Candidates;
    Candidates.Reserve(FMath::Min(MaxCandidates, 22));
    for (uint8 Raw = 0; Raw <= static_cast<uint8>(EPinkCabCockpitSlot::RightMirror) && Candidates.Num() < MaxCandidates; ++Raw)
    {
        const EPinkCabCockpitSlot Slot = static_cast<EPinkCabCockpitSlot>(Raw);
        const USceneComponent* Component = GetSlotComponent(Slot);
        const FPinkCabCockpitSlotDefinition* Definition = GetSlotDefinition(Slot);
        if (!Component || !Definition) continue;
        if (!Definition->bSupportsGrip && !Definition->bSupportsMomentary && !Definition->bSupportsWheel) continue;

        const FVector ToTarget = Component->GetComponentLocation() - WorldOrigin;
        const float DistanceCm = ToTarget.Size();
        if (DistanceCm <= KINDA_SMALL_NUMBER || DistanceCm > MaxDistanceCm) continue;
        const float Facing = FVector::DotProduct(Forward, ToTarget / DistanceCm);
        if (Facing <= 0.0f) continue;
        Candidates.Emplace(Definition->StableId, DistanceCm, 1.0f - Facing, true);
    }
    return FPinkCabInteractionTargetSelector::SelectCurrentTarget(Candidates, Candidates.Num(), MaxDistanceCm);
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
    if (!Owner || !Mesh) return nullptr;

    FPinkCabCockpitSlotDefinition Effective(Slot, PinkCabCockpitSlotId(Slot));
    Effective.LocalTransform = FTransform(Rotation, Location, Scale);
    if (bRegisterSlot)
    {
        if (const FPinkCabCockpitSlotDefinition* Configured = GetSlotDefinition(Slot))
        {
            Effective = *Configured;
        }
        else
        {
            ApplyDefaultInteractionMetadata(Effective);
            SlotDefinitions.Add(static_cast<uint8>(Slot), Effective);
        }
    }

    UStaticMesh* EffectiveMesh = Mesh;
    if (bRegisterSlot && Effective.MeshOverride.IsValid()) EffectiveMesh = Effective.MeshOverride.Get();
    UStaticMeshComponent* Component = NewObject<UStaticMeshComponent>(Owner, Name);
    Owner->AddInstanceComponent(Component);
    Component->SetupAttachment(this);
    Component->SetStaticMesh(EffectiveMesh);
    Component->SetRelativeTransform(Effective.LocalTransform);
    if (bRegisterSlot && Effective.MaterialOverride.IsValid()) Component->SetMaterial(0, Effective.MaterialOverride.Get());
    Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Component->SetGenerateOverlapEvents(false);
    Component->SetCastShadow(false);
    Component->RegisterComponent();

    if (bRegisterSlot) SlotComponents.Add(static_cast<uint8>(Slot), Component);
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
