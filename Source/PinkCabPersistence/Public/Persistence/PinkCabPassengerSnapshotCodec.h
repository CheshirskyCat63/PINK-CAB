#pragma once

#include "Persistence/PinkCabPersistedLogicalState.h"
#include "Taxi/PinkCabPassengerRegistry.h"
#include "Taxi/PinkCabPassengerSnapshot.h"

class PINKCABPERSISTENCE_API FPinkCabPassengerSnapshotCodec
{
public:
    static bool Capture(
        const FPinkCabPassengerRegistry& Registry,
        FPinkCabPassengerSnapshot& OutSnapshot);
    static bool Restore(
        const FPinkCabPassengerSnapshot& Snapshot,
        FPinkCabPassengerRegistry& OutRegistry);
    static bool ToPersistedIdentity(
        const FPinkCabPassengerRecord& Record,
        FPinkCabPersistedPassengerIdentity& OutPersisted);

private:
    static bool ValidateStableIdStrings(const TArray<FString>& Ids, int32 MaxEntries);
    static bool ValidateRelationship(const FPinkCabPassengerRelationship& Relationship);
    static bool ValidateCanonicalAppearance(const FPinkCabPassengerRecord& Record);
    static bool ValidatePreferences(
        const FPinkCabPassengerRecord& Record,
        const FPinkCabPassengerSnapshot& Snapshot);
    static bool ValidateCounters(
        const FPinkCabPassengerRecord& Record,
        const FPinkCabPassengerSnapshot& Snapshot);
    static bool ValidateRideMemoryState(
        const FPinkCabPassengerRecord& Record,
        const FPinkCabPassengerSnapshot& Snapshot);
    static bool ValidateNeuralState(
        const FPinkCabPassengerRecord& Record,
        const FPinkCabPassengerSnapshot& Snapshot);
    static bool ValidateRecord(
        const FPinkCabPassengerRecord& Record,
        const FPinkCabPassengerSnapshot& Snapshot);
    static bool Validate(const FPinkCabPassengerSnapshot& Snapshot);
};
