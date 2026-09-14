#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Cockpit/PinkCabCockpitSlot.h"
#include "PinkCabCockpitAssemblyComponent.generated.h"

class UStaticMesh;
class UStaticMeshComponent;

UCLASS(ClassGroup=(PinkCab), meta=(BlueprintSpawnableComponent))
class PINKCAB_API UPinkCabCockpitAssemblyComponent : public USceneComponent
{
    GENERATED_BODY()

public:
    UPinkCabCockpitAssemblyComponent();
    virtual void BeginPlay() override;

    USceneComponent* GetSlotComponent(EPinkCabCockpitSlot Slot) const;
    const FPinkCabCockpitSlotDefinition* GetSlotDefinition(EPinkCabCockpitSlot Slot) const;
    void ConfigureSlotDefinition(const FPinkCabCockpitSlotDefinition& Definition);
    void RegisterExternalSlot(EPinkCabCockpitSlot Slot, USceneComponent* Component);
    int32 GetRegisteredSlotCount() const { return SlotComponents.Num(); }

    FName ResolveGazeTarget(
        const FVector& WorldOrigin,
        const FVector& WorldForward,
        float MaxDistanceCm,
        int32 MaxCandidates) const;

private:
    void BuildPrimitiveShell();
    void IndexConfiguredSlots();
    UStaticMeshComponent* AddPrimitive(
        FName Name,
        UStaticMesh* Mesh,
        const FVector& Location,
        const FRotator& Rotation,
        const FVector& Scale,
        EPinkCabCockpitSlot Slot,
        bool bRegisterSlot = true);

    UPROPERTY(EditAnywhere, Category="Cockpit|Slots")
    TArray<FPinkCabCockpitSlotDefinition> SlotConfiguration;

    UPROPERTY()
    TObjectPtr<UStaticMesh> CubeMesh;

    UPROPERTY()
    TObjectPtr<UStaticMesh> CylinderMesh;

    UPROPERTY(Transient)
    TMap<uint8, TObjectPtr<USceneComponent>> SlotComponents;

    TMap<uint8, FPinkCabCockpitSlotDefinition> SlotDefinitions;
    bool bBuilt = false;
};
