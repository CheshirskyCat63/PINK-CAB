#pragma once

#include "CoreMinimal.h"
#include "Service/PinkCabServiceContext.h"
#include "Service/PinkCabServiceInventory.h"
#include "Service/PinkCabVehicleBuild.h"
#include "Service/PinkCabServiceOperationRuntime.h"
#include "Service/PinkCabFuelTank.h"
#include "Service/PinkCabMovingFuelSession.h"

struct FPinkCabOwnedPartSnapshot
{
    FString PartId;
    int32 Quantity = 0;
};

struct FPinkCabInstalledPartSnapshot
{
    FName SlotId;
    FString PartId;
};

struct FPinkCabInventorySnapshot
{
    int32 MaxItems = 0;
    int32 MaxReplayJournalEntries = 0;
    TArray<FPinkCabOwnedPartSnapshot> Parts;
    TArray<FString> AppliedOperationIds;
};
struct FPinkCabBuildSnapshot
{
    int32 SchemaVersion = 1;
    int32 MaxReplayJournalEntries = 0;
    TArray<FPinkCabInstalledPartSnapshot> InstalledParts;
    TArray<FString> AppliedInstallOperationIds;
};

struct FPinkCabServiceOperationSnapshot
{
    int32 MaxReplayJournalEntries = 0;
    TArray<FString> CompletedOperationIds;
};

struct FPinkCabFuelTankSnapshot
{
    float CapacityLiters = 0.0f;
    float CurrentLiters = 0.0f;
    int32 MaxReplayJournalEntries = 0;
    TArray<FString> CreditedTransactionIds;
};

struct FPinkCabMovingFuelSnapshot
{
    FPinkCabMovingFuelPolicyInputs Policy;
    EPinkCabMovingFuelState State = EPinkCabMovingFuelState::Disconnected;
    EPinkCabMovingFuelAbortReason AbortReason = EPinkCabMovingFuelAbortReason::None;
    FString ServiceLaneId;
};
struct FPinkCabServiceSnapshot
{
    static constexpr int32 CurrentSchemaVersion = 1;

    int32 SchemaVersion = CurrentSchemaVersion;
    FPinkCabServiceContext Context;
    FPinkCabInventorySnapshot Inventory;
    FPinkCabBuildSnapshot Build;
    FPinkCabServiceOperationSnapshot Operations;
    FPinkCabFuelTankSnapshot FuelTank;
    FPinkCabMovingFuelSnapshot MovingFuel;
};
