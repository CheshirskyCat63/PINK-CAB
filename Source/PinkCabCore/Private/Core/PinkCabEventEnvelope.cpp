#include "Core/PinkCabEventEnvelope.h"

FPinkCabEventEnvelope FPinkCabEventEnvelope::Create(
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

bool FPinkCabEventEnvelope::IsValid() const
{
    return EventId.IsValid()
        && AggregateId.IsValid()
        && !EventType.IsEmpty()
        && SchemaVersion.IsValid()
        && Sequence > 0
        && DeterministicSeed != 0;
}
uint64 FPinkCabEventEnvelope::GetSequence() const
{
    return Sequence;
}

uint64 FPinkCabEventEnvelope::GetDeterministicSeed() const
{
    return DeterministicSeed;
}

const FString& FPinkCabEventEnvelope::GetEventType() const
{
    return EventType;
}

const FPinkCabStableId& FPinkCabEventEnvelope::GetEventId() const
{
    return EventId;
}

const FPinkCabStableId& FPinkCabEventEnvelope::GetAggregateId() const
{
    return AggregateId;
}

const FPinkCabSchemaVersion& FPinkCabEventEnvelope::GetSchemaVersion() const
{
    return SchemaVersion;
}
