#include "Interaction/PinkCabPlayerInputAdapter.h"

#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerInput.h"
#include "Interaction/PinkCabPhysicalInputConvention.h"
#include "Interaction/PinkCabSemanticInputRouter.h"

namespace
{
bool IsSemanticActionHeld(
    const FPinkCabSemanticInputRouter& Router,
    const EPinkCabSemanticAction Action,
    TFunctionRef<bool(const FKey&)> IsKeyDown)
{
    const FKey Key = Router.GetKeyForAction(Action);
    return Key.IsValid() && IsKeyDown(Key);
}
}

FPinkCabPlayerInputSample FPinkCabPlayerInputAdapter::Capture(
    APlayerController& Controller,
    const FPinkCabSemanticInputRouter& Router) const
{
    float ProcessedMouseX = 0.0f;
    float ProcessedMouseY = 0.0f;
    Controller.GetInputMouseDelta(ProcessedMouseX, ProcessedMouseY);

    float RawMouseX = ProcessedMouseX;
    float RawMouseY = ProcessedMouseY;
    if (Controller.PlayerInput)
    {
        RawMouseX = Controller.PlayerInput->GetRawKeyValue(EKeys::MouseX);
        RawMouseY = Controller.PlayerInput->GetRawKeyValue(EKeys::MouseY);
    }

    const FKey WheelKey = Router.GetKeyForAction(EPinkCabSemanticAction::Wheel);
    const float AnalogWheelAxis = WheelKey.IsValid()
        ? Controller.GetInputAnalogKeyState(WheelKey)
        : 0.0f;
    const float WheelAxis = ResolveWheelAxis(
        AnalogWheelAxis,
        Controller.WasInputKeyJustPressed(EKeys::MouseScrollUp),
        Controller.WasInputKeyJustPressed(EKeys::MouseScrollDown));

    return ComposeSample(
        Router,
        [&Controller](const FKey& Key) { return Controller.IsInputKeyDown(Key); },
        Controller.WasInputKeyJustPressed(EKeys::Escape),
        ProcessedMouseX,
        ProcessedMouseY,
        RawMouseX,
        RawMouseY,
        WheelAxis);
}

FPinkCabPlayerInputSample FPinkCabPlayerInputAdapter::ComposeSample(
    const FPinkCabSemanticInputRouter& Router,
    TFunctionRef<bool(const FKey&)> IsKeyDown,
    const bool bSystemMenuToggleRequested,
    const float ProcessedMouseX,
    const float ProcessedMouseY,
    const float RawMouseX,
    const float RawMouseY,
    const float WheelAxis)
{
    FPinkCabPlayerInputSample Sample;
    Sample.bSystemMenuToggleRequested = bSystemMenuToggleRequested;
    Sample.bGazeHeld = IsSemanticActionHeld(Router, EPinkCabSemanticAction::GazeHold, IsKeyDown);
    Sample.bQuickRecall1Held = IsSemanticActionHeld(Router, EPinkCabSemanticAction::QuickRecall1, IsKeyDown);
    Sample.bQuickRecall2Held = IsSemanticActionHeld(Router, EPinkCabSemanticAction::QuickRecall2, IsKeyDown);
    Sample.bQuickRecall3Held = IsSemanticActionHeld(Router, EPinkCabSemanticAction::QuickRecall3, IsKeyDown);
    Sample.bQuickRecall4Held = IsSemanticActionHeld(Router, EPinkCabSemanticAction::QuickRecall4, IsKeyDown);
    Sample.bGripHeld = IsSemanticActionHeld(Router, EPinkCabSemanticAction::Grip, IsKeyDown);
    Sample.bMomentaryHeld = IsSemanticActionHeld(Router, EPinkCabSemanticAction::MomentaryPress, IsKeyDown);
    Sample.bClutchHeld = IsSemanticActionHeld(Router, EPinkCabSemanticAction::Clutch, IsKeyDown);
    Sample.bBrakeHeld = IsSemanticActionHeld(Router, EPinkCabSemanticAction::Brake, IsKeyDown);
    Sample.bThrottleHeld = IsSemanticActionHeld(Router, EPinkCabSemanticAction::Throttle, IsKeyDown);
    Sample.LookMouseX = FMath::IsFinite(ProcessedMouseX) ? ProcessedMouseX : 0.0f;
    Sample.LookMouseY = FMath::IsFinite(ProcessedMouseY) ? ProcessedMouseY : 0.0f;
    Sample.DeviceX = FPinkCabPhysicalInputConvention::ResolveActiveDeviceAxis(
        Sample.LookMouseX,
        RawMouseX);
    Sample.DeviceY = FPinkCabPhysicalInputConvention::ResolveActiveDeviceAxis(
        Sample.LookMouseY,
        RawMouseY);
    Sample.WheelSteps = WheelAxis > 0.0f ? 1 : (WheelAxis < 0.0f ? -1 : 0);
    return Sample;
}

float FPinkCabPlayerInputAdapter::ResolveWheelAxis(
    const float AnalogAxis,
    const bool bScrollUpPressed,
    const bool bScrollDownPressed)
{
    if (bScrollUpPressed != bScrollDownPressed)
    {
        return bScrollUpPressed ? 1.0f : -1.0f;
    }
    return FMath::IsFinite(AnalogAxis) ? AnalogAxis : 0.0f;
}
