#pragma once

#include "CoreMinimal.h"
#include "InputCoreTypes.h"
#include "Interaction/PinkCabPlayerInputSample.h"

class APlayerController;
struct FPinkCabSemanticInputRouter;

class PINKCAB_API FPinkCabPlayerInputAdapter
{
public:
    FPinkCabPlayerInputSample Capture(
        APlayerController& Controller,
        const FPinkCabSemanticInputRouter& Router) const;

    static FPinkCabPlayerInputSample ComposeSample(
        const FPinkCabSemanticInputRouter& Router,
        TFunctionRef<bool(const FKey&)> IsKeyDown,
        bool bSystemMenuToggleRequested,
        float ProcessedMouseX,
        float ProcessedMouseY,
        float RawMouseX,
        float RawMouseY,
        float WheelAxis);
};
