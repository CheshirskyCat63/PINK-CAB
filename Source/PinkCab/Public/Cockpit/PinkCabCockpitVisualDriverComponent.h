#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PinkCabCockpitVisualDriverComponent.generated.h"

class UPinkCabCockpitAssemblyComponent;
struct FPinkCabCockpitPresentationState;
UCLASS(ClassGroup=(PinkCab), meta=(BlueprintSpawnableComponent))
class PINKCAB_API UPinkCabCockpitVisualDriverComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UPinkCabCockpitVisualDriverComponent();

    static float SteeringAngleDegrees(float Steering);
    static float PedalTravelDegrees(float Value);
    static float HandbrakeAngleDegrees(bool bEngaged);
    static FVector GearLeverOffset(int32 Gear);

    void Apply(UPinkCabCockpitAssemblyComponent& Assembly,
        const FPinkCabCockpitPresentationState& State);

private:
    TMap<uint8, FTransform> BaseTransforms;
    void CacheBaseTransforms(UPinkCabCockpitAssemblyComponent& Assembly);
};

