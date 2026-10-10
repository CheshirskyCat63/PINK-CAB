#pragma once
#include "CoreMinimal.h"
#include "Misc/ScopeLock.h"
#include "ChaosVehicleManagerAsyncCallback.h"
#include "Vehicle/PinkCabVehicleControlState.h"
#include "Vehicle/PinkCabClutchDrivelineConfig.h"
#include "Vehicle/PinkCabEngineRpmEnvelope.h"
#include "Vehicle/PinkCabDrivelineStepTelemetry.h"

struct FPinkCabChaosCommandFrame
{
    FNetworkVehicleInputs Native;
    FPinkCabVehicleControlState Controls;
    FPinkCabClutchDrivelineConfig ClutchConfig;
    FPinkCabEngineRpmEnvelope RpmEnvelope;
    uint64 Sequence = 0;
    bool bValid = false;
};

// One bounded complete local command, never one independently sampled pedal.
// All traffic is value-only. A copy is held for the entire physics step.
class FPinkCabChaosCommandChannel
{
public:
    void Publish(FPinkCabChaosCommandFrame Frame)
    {
        FScopeLock Lock(&Mutex);
        Frame.Sequence = ++Sequence;
        Command = MoveTemp(Frame);
    }
    FPinkCabChaosCommandFrame ReadCommand() const
    {
        FScopeLock Lock(&Mutex);
        return Command;
    }
    void Reset()
    {
        FScopeLock Lock(&Mutex);
        Command = {};
        Command.Sequence = ++Sequence;
        Feedback = {};
    }
    void PublishFeedback(const FPinkCabDrivelineStepTelemetry& Value)
    {
        FScopeLock Lock(&Mutex);
        Feedback = Value;
    }
    FPinkCabDrivelineStepTelemetry ReadFeedback() const
    {
        FScopeLock Lock(&Mutex);
        return Feedback;
    }
private:
    mutable FCriticalSection Mutex;
    uint64 Sequence = 0;
    FPinkCabChaosCommandFrame Command;
    FPinkCabDrivelineStepTelemetry Feedback;
};
