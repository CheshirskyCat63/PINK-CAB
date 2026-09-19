#pragma once

#include "CoreMinimal.h"
#include "Core/PinkCabStateKernel.h"
#include "Persistence/PinkCabWorkdaySessionState.h"
#include "World/PinkCabCityDeltaState.h"

struct FPinkCabCityDeltaSnapshotRecord
{
    FString DeltaId;
    EPinkCabCityDeltaKind Kind = EPinkCabCityDeltaKind::LaneClosure;
    FString SubjectStableId;
    FString OperationKey;
};

struct FPinkCabCityDeltaSnapshot
{
    static constexpr int32 CurrentSchemaVersion = 1;
    int32 SchemaVersion = CurrentSchemaVersion;
    int32 MaxDeltas = 0;
    TArray<FPinkCabCityDeltaSnapshotRecord> Records;
};

struct FPinkCabStateKernelSnapshot
{
    static constexpr int32 CurrentSchemaVersion = 1;
    int32 SchemaVersion = CurrentSchemaVersion;
    uint64 RootSeed = 0;
    uint64 Sequence = 0;
};

struct FPinkCabWorkdaySessionSnapshot
{
    static constexpr int32 CurrentSchemaVersion = 1;
    int32 SchemaVersion = CurrentSchemaVersion;
    FString WorkdayId;
    int32 Ordinal = 0;
    double ElapsedGameSeconds = 0.0;
    int32 MaxReplayJournalEntries = 0;
    bool bSummaryCommitted = false;
    int32 HouseholdTransactionCount = 0;
    bool bTerminalRecovery = false;
    TArray<FString> AppliedHouseholdOperationIds;
};

class PINKCABPERSISTENCE_API FPinkCabWorldSessionSnapshotCodec
{
public:
    static bool CaptureCityDeltas(
        const FPinkCabCityDeltaState& Source,
        FPinkCabCityDeltaSnapshot& OutSnapshot);
    static bool RestoreCityDeltas(
        const FPinkCabCityDeltaSnapshot& Snapshot,
        FPinkCabCityDeltaState& OutState);
    static bool CaptureKernel(
        const FPinkCabStateKernel& Source,
        FPinkCabStateKernelSnapshot& OutSnapshot);
    static bool RestoreKernel(
        const FPinkCabStateKernelSnapshot& Snapshot,
        FPinkCabStateKernel& OutKernel);
    static bool CaptureWorkday(
        const FPinkCabWorkdaySessionState& Source,
        FPinkCabWorkdaySessionSnapshot& OutSnapshot);
    static bool RestoreWorkday(
        const FPinkCabWorkdaySessionSnapshot& Snapshot,
        FPinkCabWorkdaySessionState& OutState);

private:
    static bool ValidateCityDeltas(
        const FPinkCabCityDeltaSnapshot& Snapshot);
    static bool ValidateWorkday(
        const FPinkCabWorkdaySessionSnapshot& Snapshot);
};
