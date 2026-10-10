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

// Local-only extension indexed by the exact native packet prepared on the GT.
// Keys are opaque identities, never dereferenced. A native packet cannot be
// reallocated at the same address while Chaos still owns it. Repeated physics
// substeps read the same immutable snapshot; there is no latest-value fallback.
class FPinkCabChaosCommandChannel
{
public:
    static constexpr int32 MaxQueuedFrames = 256;
    bool Publish(FPinkCabChaosCommandFrame Frame, const FChaosVehicleAsyncInput* Packet)
    {
        FScopeLock Lock(&Mutex);
        if (!Packet) return false;
        for (auto It = Commands.CreateIterator(); It; ++It)
            if (It.Value().Sequence < LastConsumedSequence) It.RemoveCurrent();
        // Never evict an unconsumed packet to insert a newer one. Exhaustion is
        // an explicit error at the caller, not silently substituted controls.
        if (!Commands.Contains(Packet) && Commands.Num() >= MaxQueuedFrames) return false;
        Frame.Sequence = ++Sequence;
        Commands.Add(Packet, MoveTemp(Frame));
        return true;
    }
    FPinkCabChaosCommandFrame ReadCommand(const FChaosVehicleAsyncInput* Packet) const
    {
        FScopeLock Lock(&Mutex);
        if (const auto* Found = Commands.Find(Packet))
        {
            LastConsumedSequence = FMath::Max(LastConsumedSequence, Found->Sequence);
            return *Found;
        }
        return {}; // Missing or retired packet: no propulsion, never latest input.
    }
    void Reset()
    {
        FScopeLock Lock(&Mutex);
        // Reset belongs to the next native input, not older queued commands.
        // Physical recreation gets a new channel; outstanding input keeps its
        // old channel alive with the old simulation until its native retirement.
        Feedback = {};
    }
    int32 GetRetainedFrameCount() const
    {
        FScopeLock Lock(&Mutex);
        return Commands.Num();
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
    mutable uint64 LastConsumedSequence = 0;
    TMap<const FChaosVehicleAsyncInput*, FPinkCabChaosCommandFrame> Commands;
    FPinkCabDrivelineStepTelemetry Feedback;
};
