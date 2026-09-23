#pragma once

#include "CoreMinimal.h"
#include "Economy/PinkCabTransaction.h"
#include "Vehicle/PinkCabVehicleHealthState.h"

struct FPinkCabRepairLine
{
    EPinkCabVehicleHealthChannel Channel =
        EPinkCabVehicleHealthChannel::Brake;
    float TargetHealth01 = 1.0f;
};

struct FPinkCabRepairRequest
{
    FPinkCabEconomyTransaction Transaction;
    TArray<FPinkCabRepairLine> Lines;
};

class PINKCAB_API FPinkCabRepairService
{
public:
    static bool BuildRequest(
        const FPinkCabVehicleHealthState& Health,
        const FPinkCabTransactionId& TransactionId,
        int64 AmountMinor,
        EPinkCabEconomyPurpose Purpose,
        const TArray<FPinkCabRepairLine>& Lines,
        FPinkCabRepairRequest& OutRequest);
    static bool ApplyCommittedRepair(
        FPinkCabVehicleHealthState& Health,
        const FPinkCabRepairRequest& Request);
};
