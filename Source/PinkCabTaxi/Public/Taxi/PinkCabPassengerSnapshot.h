#pragma once

#include "CoreMinimal.h"
#include "Taxi/PinkCabPassengerRecord.h"

struct FPinkCabPassengerSnapshot
{
    static constexpr int32 CurrentSchemaVersion = 1;

    int32 SchemaVersion = CurrentSchemaVersion;
    int32 MaxRecords = 256;
    int32 MaxPreferences = 8;
    int32 MaxRideMemories = 32;
    int32 MaxReplayJournalEntries = 512;
    int32 MaxNeuralMessages = 64;
    int32 MaxNeuralReplayJournalEntries = 256;
    TArray<FPinkCabPassengerRecord> Records;
};
