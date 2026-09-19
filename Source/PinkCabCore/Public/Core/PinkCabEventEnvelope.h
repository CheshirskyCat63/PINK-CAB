#pragma once

#include "CoreMinimal.h"
#include "Core/PinkCabSchemaVersion.h"
#include "Core/PinkCabStableId.h"

struct PINKCABCORE_API FPinkCabEventEnvelope
{
    static FPinkCabEventEnvelope Create(
        const FPinkCabStableId& InEventId,
        const FPinkCabStableId& InAggregateId,
        const FString& InEventType,
        const FPinkCabSchemaVersion& InSchemaVersion,
        uint64 InSequence,
        uint64 InDeterministicSeed);

    bool IsValid() const;
    uint64 GetSequence() const;
    uint64 GetDeterministicSeed() const;
    const FString& GetEventType() const;
    const FPinkCabStableId& GetEventId() const;
    const FPinkCabStableId& GetAggregateId() const;
    const FPinkCabSchemaVersion& GetSchemaVersion() const;

private:
    FPinkCabStableId EventId;
    FPinkCabStableId AggregateId;
    FString EventType;
    FPinkCabSchemaVersion SchemaVersion;
    uint64 Sequence = 0;
    uint64 DeterministicSeed = 0;
};
