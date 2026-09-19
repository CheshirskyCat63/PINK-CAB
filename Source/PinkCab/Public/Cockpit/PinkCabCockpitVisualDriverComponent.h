#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PinkCabCockpitVisualDriverComponent.generated.h"

class UPinkCabCockpitAssemblyComponent;
class USceneComponent;
struct FPinkCabCockpitPresentationState;
enum class EPinkCabCockpitSlot : uint8;
UCLASS(ClassGroup=(PinkCab), meta=(BlueprintSpawnableComponent))
class PINKCAB_API UPinkCabCockpitVisualDriverComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UPinkCabCockpitVisualDriverComponent();

    static float SteeringAngleDegrees(float Steering);
    static float PedalTravelDegrees(float Value);
    static float HandbrakeAngleDegrees(float Amount);
    static float TemperatureNeedleAngleDegrees(float Temperature01);
    static float FuelNeedleAngleDegrees(float Fuel01);
    static float SpeedometerNeedleAngleDegrees(float SpeedKmh);
    static float TachometerNeedleAngleDegrees(float EngineRpm);
    static FVector PivotCompensatedLocation(const FTransform& BaseTransform, const FVector& MeshLocalCenter, const FRotator& RotationOffset);
    static FVector GearLeverOffset(int32 Gear);
    static FVector2D GearCursorForGear(int32 Gear);
    static FVector GearLeverOffsetFromCursor(FVector2D Cursor);

    void Apply(UPinkCabCockpitAssemblyComponent& Assembly,
        const FPinkCabCockpitPresentationState& State);
    void SetSteeringVisualComponent(USceneComponent* Component);
    USceneComponent* GetSteeringVisualComponent() const { return SteeringVisualComponent.Get(); }
    void InvalidateBaseTransforms()
    {
        BaseTransforms.Reset();
        bSteeringVisualBaseValid = false;
    }

private:
    TMap<uint8, FTransform> BaseTransforms;
    UPROPERTY(Transient)
    TObjectPtr<USceneComponent> SteeringVisualComponent;
    FTransform SteeringVisualBaseTransform = FTransform::Identity;
    bool bSteeringVisualBaseValid = false;

    void CacheBaseTransforms(UPinkCabCockpitAssemblyComponent& Assembly);
    const FTransform* GetBaseTransform(EPinkCabCockpitSlot Slot) const;
    void ApplyRotationOffset(
        UPinkCabCockpitAssemblyComponent& Assembly,
        EPinkCabCockpitSlot Slot,
        const FRotator& Offset) const;
    void ApplyLocationOffset(
        UPinkCabCockpitAssemblyComponent& Assembly,
        EPinkCabCockpitSlot Slot,
        const FVector& Offset) const;
    void ApplySteeringState(
        UPinkCabCockpitAssemblyComponent& Assembly,
        const FPinkCabCockpitPresentationState& State);
    void ApplyControlMotion(
        UPinkCabCockpitAssemblyComponent& Assembly,
        const FPinkCabCockpitPresentationState& State) const;
    void ApplyAvailabilityState(
        UPinkCabCockpitAssemblyComponent& Assembly,
        const FPinkCabCockpitPresentationState& State) const;
};

