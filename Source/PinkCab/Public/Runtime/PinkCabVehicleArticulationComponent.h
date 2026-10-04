#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Runtime/PinkCabVehicleArticulationLogic.h"
#include "PinkCabVehicleArticulationComponent.generated.h"

class UPinkCabVehicleVisualShellComponent;

UCLASS(ClassGroup=(PinkCab), meta=(BlueprintSpawnableComponent))
class PINKCAB_API UPinkCabVehicleArticulationComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UPinkCabVehicleArticulationComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(
        float DeltaTime,
        ELevelTick TickType,
        FActorComponentTickFunction* ThisTickFunction) override;

    void SetVisualShell(UPinkCabVehicleVisualShellComponent* InVisualShell);

    UFUNCTION(BlueprintCallable, Category="PinkCab|Vehicle|Presentation")
    bool SetPanelOpen(FName ArticulationId, bool bOpen);

    UFUNCTION(BlueprintCallable, Category="PinkCab|Vehicle|Presentation")
    bool TogglePanel(FName ArticulationId);

    UFUNCTION(BlueprintPure, Category="PinkCab|Vehicle|Presentation")
    float GetPanelFraction(FName ArticulationId) const;

    UFUNCTION(BlueprintCallable, Category="PinkCab|Vehicle|Presentation")
    void CloseAll();

private:
    UPROPERTY(Transient)
    TObjectPtr<UPinkCabVehicleVisualShellComponent> VisualShell;

    TMap<FName, FPinkCabVehicleArticulationState> States;

    void RefreshStateInventory();
};
