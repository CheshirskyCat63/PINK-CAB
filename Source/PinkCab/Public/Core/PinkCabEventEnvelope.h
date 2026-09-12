#pragma once

#include "CoreMinimal.h"
#include "Core/PinkCabSchemaVersion.h"
#include "Core/PinkCabStableId.h"

struct FPinkCabEventEnvelope
{
    static FPinkCabEventEnvelope Create(
        const FPinkCabStableId& InEventId,
        const FPinkCabStableId& InAggregateId,
        const FString& InEventType,
        const FPinkCabSchemaVersion& InSchemaVersion,
        uint64 InSequence,
        uint64 InDeterministicSeed)
    {
        FPinkCabEventEnvelope Envelope;
        Envelope.EventId = InEventId;
        Envelope.AggregateId = InAggregateId;
        Envelope.EventType = InEventType.TrimStartAndEnd();
        Envelope.SchemaVersion = InSchemaVersion;
        Envelope.Sequence = InSequence;
        Envelope.DeterministicSeed = InDeterministicSeed;
        return Envelope;
    }

    bool IsValid() const
    {
        return EventId.IsValid() && AggregateId.IsValid() && !EventType.IsEmpty()
            && SchemaVersion.IsValid() && Sequence > 0 && DeterministicSeed != 0;
    }
    uint64 GetSequence() const { return Sequence; }
    uint64 GetDeterministicSeed() const { return DeterministicSeed; }
    const FString& GetEventType() const { return EventType; }
    const FPinkCabStableId& GetEventId() const { return EventId; }
    const FPinkCabStableId& GetAggregateId() const { return AggregateId; }
    const FPinkCabSchemaVersion& GetSchemaVersion() const { return SchemaVersion; }

private:
    FPinkCabStableId EventId;
    FPinkCabStableId AggregateId;
    FString EventType;
    FPinkCabSchemaVersion SchemaVersion;
    uint64 Sequence = 0;
    uint64 DeterministicSeed = 0;
};
