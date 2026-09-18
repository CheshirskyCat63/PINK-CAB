#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Vehicle/PinkCabVehicleVisualProfile.h"
#include "PinkCabVehicleVisualShellComponent.generated.h"

class UPrimitiveComponent;
class UStaticMeshComponent;

UCLASS(ClassGroup=(PinkCab), meta=(BlueprintSpawnableComponent))
class PINKCAB_API UPinkCabVehicleVisualShellComponent : public USceneComponent
{
    GENERATED_BODY()

public:
    UPinkCabVehicleVisualShellComponent();
    virtual void BeginPlay() override;

    const FPinkCabVehicleVisualProfile& GetProfile() const { return Profile; }
    FName GetProfileId() const { return Profile.ProfileId; }
    bool HasExteriorAsset() const { return Profile.HasExteriorAsset(); }
    bool HasCabinAsset() const { return Profile.HasCabinAsset(); }
    UPrimitiveComponent* GetExteriorPresentation() const;
    UPrimitiveComponent* GetCabinPresentation() const;
    int32 GetPresentationPartCount() const { return PresentationPartComponents.Num(); }
    bool ApplyProfile(const FPinkCabVehicleVisualProfile& InProfile);
    bool RebuildPresentation();

private:
    UPrimitiveComponent* BuildExterior();
    UPrimitiveComponent* BuildCabin();
    bool BuildPresentationParts();
    void DestroyPresentationParts();
    void DestroyPresentationComponent(TObjectPtr<UPrimitiveComponent>& Component);

    UPROPERTY(EditAnywhere, Category="Vehicle|Visual")
    FPinkCabVehicleVisualProfile Profile = FPinkCabVehicleVisualProfile::Fallback();

    UPROPERTY(Transient)
    TObjectPtr<UPrimitiveComponent> ExteriorPresentation;

    UPROPERTY(Transient)
    TObjectPtr<UPrimitiveComponent> CabinPresentation;

    UPROPERTY(Transient)
    TArray<TObjectPtr<UStaticMeshComponent>> PresentationPartComponents;
};
