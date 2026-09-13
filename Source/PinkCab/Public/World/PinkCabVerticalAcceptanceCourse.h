#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "World/PinkCabVerticalContactRegistry.h"
#include "PinkCabVerticalAcceptanceCourse.generated.h"

class USceneComponent;
class UStaticMeshComponent;
class UDirectionalLightComponent;

UCLASS(NotBlueprintable)
class PINKCAB_API APinkCabVerticalAcceptanceCourse : public AActor
{
    GENERATED_BODY()

public:
    APinkCabVerticalAcceptanceCourse();

    static FPinkCabCityIdentity GetPresentationCityIdentity();
    static FPinkCabLaneId GetPresentationLaneId(int32 Index);
    static FString GetPresentationContactId(int32 Index);

    FString GetPresentedContactId(int32 Index) const;
    bool HasPrimitiveForContact(const FString& ContactId) const;
    int32 GetL1PrimitiveCount() const { return L1Primitives.Num(); }
    int32 GetL2PrimitiveCount() const { return L2Primitives.Num(); }
    bool IsPresentationOnly() const { return true; }
    FString GetAuthorityMarker() const;

private:
    UPROPERTY(VisibleAnywhere, Category="PINK CAB|Vertical Acceptance")
    TObjectPtr<USceneComponent> SceneRoot;

    UPROPERTY(VisibleAnywhere, Category="PINK CAB|Vertical Acceptance")
    TArray<TObjectPtr<UStaticMeshComponent>> L1Primitives;

    UPROPERTY(VisibleAnywhere, Category="PINK CAB|Vertical Acceptance")
    TArray<TObjectPtr<UStaticMeshComponent>> L2Primitives;

    UPROPERTY(VisibleAnywhere, Category="PINK CAB|Vertical Acceptance")
    TObjectPtr<UDirectionalLightComponent> KeyLight;
};
