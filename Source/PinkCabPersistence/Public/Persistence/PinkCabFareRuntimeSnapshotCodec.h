#pragma once

#include "Persistence/PinkCabFareRuntimeSnapshotTypes.h"

class PINKCABPERSISTENCE_API FPinkCabFareRuntimeSnapshotCodec
{
public:
    static bool Capture(
        const FPinkCabFareLoopCoordinator& Coordinator,
        FPinkCabFareRuntimeSnapshot& OutSnapshot);
    static bool Restore(
        const FPinkCabFareRuntimeSnapshot& Snapshot,
        FPinkCabFareLoopCoordinator& OutCoordinator);

private:
    static bool IsValidId(const FString& Value);
    static bool ValidatePricing(const FPinkCabFarePricingTerms& Terms);
    static bool ValidateManifestHeader(
        const FPinkCabFarePassengerManifestSnapshot& Manifest);
    static bool ValidateManifestRecord(
        const FPinkCabFarePassengerRecordSnapshot& Record,
        TSet<FString>& PassengerIds,
        TSet<uint8>& Seats,
        float& TotalMassKg);
    static bool ValidateManifest(
        const FPinkCabFarePassengerManifestSnapshot& Manifest);
    static bool ValidateSessionBasics(
        const FPinkCabFareSessionSnapshot& Session);
    static bool ValidateSessionRecovery(
        const FPinkCabFareSessionSnapshot& Session);
    static bool ValidateSession(
        const FPinkCabFareSessionSnapshot& Session);
    static bool ValidateTaximeter(
        const FPinkCabTaximeterSnapshot& Taximeter);

    static bool ValidateOfferedState(
        const FPinkCabFareRuntimeSnapshot& Snapshot);
    static bool ValidateBoardedState(
        const FPinkCabFareRuntimeSnapshot& Snapshot);
    static bool ValidateActiveState(
        const FPinkCabFareRuntimeSnapshot& Snapshot);
    static bool ValidateAwaitingPaymentState(
        const FPinkCabFareRuntimeSnapshot& Snapshot);
    static bool ValidatePaidState(
        const FPinkCabFareRuntimeSnapshot& Snapshot);
    static bool ValidateEvadedState(
        const FPinkCabFareRuntimeSnapshot& Snapshot);
    static bool ValidateCompleteState(
        const FPinkCabFareRuntimeSnapshot& Snapshot);
    static bool ValidateFailedState(
        const FPinkCabFareRuntimeSnapshot& Snapshot);
    static bool ValidateStateMachine(
        const FPinkCabFareRuntimeSnapshot& Snapshot);
    static bool ValidateIdentityCoherence(
        const FPinkCabFareRuntimeSnapshot& Snapshot);
    static bool Validate(
        const FPinkCabFareRuntimeSnapshot& Snapshot);
};
