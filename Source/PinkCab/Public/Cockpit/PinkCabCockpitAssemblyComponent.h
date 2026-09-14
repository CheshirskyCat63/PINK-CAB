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
    void RegisterExternalSlot(EPinkCabCockpitSlot Slot, USceneComponent* Component);
    int32 GetRegisteredSlotCount() const { return SlotComponents.Num(); }

private:
    void BuildPrimitiveShell();
    UStaticMeshComponent* AddPrimitive(
        FName Name,
        UStaticMesh* Mesh,
        const FVector& Location,
        const FRotator& Rotation,
        const FVector& Scale,
        EPinkCabCockpitSlot Slot,
        bool bRegisterSlot = true);

    UPROPERTY()
    TObjectPtr<UStaticMesh> CubeMesh;

    UPROPERTY()
    TObjectPtr<UStaticMesh> CylinderMesh;

    UPROPERTY(Transient)
    TMap<uint8, TObjectPtr<USceneComponent>> SlotComponents;

    bool bBuilt = false;
};
