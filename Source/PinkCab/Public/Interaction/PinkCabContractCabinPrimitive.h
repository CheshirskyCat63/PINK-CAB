#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interaction/PinkCabInteractionModel.h"
#include "PinkCabContractCabinPrimitive.generated.h"

class USceneComponent;
class UStaticMeshComponent;
class UPointLightComponent;

UCLASS()
class PINKCAB_API APinkCabContractCabinPrimitive : public AActor
{
    GENERATED_BODY()

public:
    APinkCabContractCabinPrimitive();

    FName GetInteractionTargetId() const { return InteractionTargetId; }
    bool IsDeveloperStateOn() const { return bDeveloperStateOn; }
    void ApplyInteractionEvent(const FPinkCabInteractionEvent& Event);

private:
    void UpdateVisualState();

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<USceneComponent> SceneRoot;

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UStaticMeshComponent> ToggleMesh;

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UPointLightComponent> StateLight;

    UPROPERTY(EditAnywhere)
    FName InteractionTargetId = FName(TEXT("ContractZeroHeadlampToggle"));

    bool bDeveloperStateOn = false;
};
