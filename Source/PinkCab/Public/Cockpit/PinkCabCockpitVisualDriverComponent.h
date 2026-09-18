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
    static float HandbrakeAngleDegrees(float Amount);
    static float TemperatureNeedleAngleDegrees(float Temperature01);
    static float FuelNeedleAngleDegrees(float Fuel01);
    static float SpeedometerNeedleAngleDegrees(float SpeedKmh);
    static float TachometerNeedleAngleDegrees(float EngineRpm);
    static FVector PivotCompensatedLocation(const FTransform& BaseTransform, const FVector& MeshLocalCenter, const FRotator& RotationOffset);
    static FVector GearLeverOffset(int32 Gear);
    static FVector2D GearCursorForGear(int32 Gear);
    static FVector2D IntegrateGearCursor(FVector2D Current, float MouseDeltaX, float MouseDeltaY, float Gain = 0.025f);
    static FVector GearLeverOffsetFromCursor(FVector2D Cursor);
    static int32 GearForCursor(FVector2D Cursor);

    void Apply(UPinkCabCockpitAssemblyComponent& Assembly,
        const FPinkCabCockpitPresentationState& State);
    void InvalidateBaseTransforms() { BaseTransforms.Reset(); }

private:
    TMap<uint8, FTransform> BaseTransforms;
    void CacheBaseTransforms(UPinkCabCockpitAssemblyComponent& Assembly);
};

