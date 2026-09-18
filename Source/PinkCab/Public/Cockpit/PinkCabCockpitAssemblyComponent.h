#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Cockpit/PinkCabCockpitSlot.h"
#include "Cockpit/PinkCabCockpitVisualBinding.h"
#include "PinkCabCockpitAssemblyComponent.generated.h"

class UMaterialInterface;
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
    bool ApplyVisualBindings(TConstArrayView<FPinkCabCockpitVisualBinding> Bindings);
    void ResetVisualBindings();
    void SetGeneratedVisualMode(bool bShowFallbackShell, TConstArrayView<FPinkCabCockpitVisualBinding> ActiveBindings);

    FName ResolveGazeTarget(
        const FVector& WorldOrigin,
        const FVector& WorldForward,
        float MaxDistanceCm,
        int32 MaxCandidates) const;

private:
    void BuildPrimitiveShell();
    void IndexConfiguredSlots();
    void CaptureVisualBaseline();
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

    UPROPERTY(Transient)
    TArray<TObjectPtr<UStaticMeshComponent>> GeneratedPrimitives;

    TMap<uint8, FPinkCabCockpitSlotDefinition> SlotDefinitions;
    TMap<uint8, FTransform> BaselineTransforms;
    TMap<uint8, TObjectPtr<UStaticMesh>> BaselineMeshes;
    TMap<uint8, TObjectPtr<UMaterialInterface>> BaselineMaterials;
    TMap<uint8, bool> BaselineHiddenInGame;
    bool bBuilt = false;
    bool bVisualBaselineCaptured = false;
};
