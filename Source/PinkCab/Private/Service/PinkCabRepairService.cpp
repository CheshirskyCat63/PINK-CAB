#include "Service/PinkCabRepairService.h"

bool FPinkCabRepairService::BuildRequest(
    const FPinkCabVehicleHealthState& Health,
    const FPinkCabTransactionId& TransactionId,
    int64 AmountMinor,
    EPinkCabEconomyPurpose Purpose,
    const TArray<FPinkCabRepairLine>& Lines,
    FPinkCabRepairRequest& OutRequest)
{
    if (!TransactionId.IsValid()
        || AmountMinor <= 0
        || Lines.IsEmpty())
    {
        return false;
    }

    for (const FPinkCabRepairLine& Line : Lines)
    {
        const int32 Index = static_cast<int32>(Line.Channel);
        if (Index < 0
            || Index >= static_cast<int32>(
                EPinkCabVehicleHealthChannel::CosmeticBody))
        {
            return false;
        }

        const float Target =
            FMath::Clamp(Line.TargetHealth01, 0.0f, 1.0f);
        if (Target
            <= Health.GetHealth(Line.Channel)
                + KINDA_SMALL_NUMBER)
        {
            return false;
        }
    }

    OutRequest.Transaction =
        FPinkCabEconomyTransaction::Debit(
            TransactionId,
            EPinkCabTransactionType::Repair,
            AmountMinor,
            Purpose);
    OutRequest.Lines = Lines;
    return true;
}

bool FPinkCabRepairService::ApplyCommittedRepair(
    FPinkCabVehicleHealthState& Health,
    const FPinkCabRepairRequest& Request)
{
    bool bChanged = false;
    for (const FPinkCabRepairLine& Line : Request.Lines)
    {
        bChanged |= Health.RestoreFunctionalHealthTo(
            Line.Channel,
            Line.TargetHealth01);
    }
    return bChanged;
}
