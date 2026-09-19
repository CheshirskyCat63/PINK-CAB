#pragma once

#include "Taxi/PinkCabPassengerRecordTypes.h"

class PINKCABTAXI_API FPinkCabPassengerRegistry
{
public:
    explicit FPinkCabPassengerRegistry(
        int32 InMaxRecords = 256,
        int32 InMaxPreferences = 8,
        int32 InMaxRideMemories = 32,
        int32 InMaxReplayJournalEntries = 512,
        int32 InMaxNeuralMessages = 64,
        int32 InMaxNeuralReplayJournalEntries = 256);

    int32 Num() const;
    bool TryCreate(
        const FPinkCabStableId& IdentityId,
        const FPinkCabPassengerTemplate& Template,
        const FString& ContextKey,
        const TArray<FName>& PreferenceTags,
        FPinkCabPassengerRecord*& OutRecord);
    FPinkCabPassengerRecord* Find(const FPinkCabStableId& IdentityId);
    const FPinkCabPassengerRecord* Find(
        const FPinkCabStableId& IdentityId) const;
    uint64 GetReconstructionSignature() const;

private:
    int32 MaxRecords = 256;
    int32 MaxPreferences = 8;
    int32 MaxRideMemories = 32;
    int32 MaxReplayJournalEntries = 512;
    int32 MaxNeuralMessages = 64;
    int32 MaxNeuralReplayJournalEntries = 256;
    TMap<FString, FPinkCabPassengerRecord> Records;

    friend class FPinkCabPassengerSnapshotCodec;
    friend class FPinkCabGameSnapshotArchive;
};
