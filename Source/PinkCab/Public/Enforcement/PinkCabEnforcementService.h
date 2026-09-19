#pragma once

#include "CoreMinimal.h"
#include "Economy/PinkCabEconomyLedger.h"
#include "Enforcement/PinkCabEnforcementLedger.h"
#include "Enforcement/PinkCabRoadRuleProfile.h"
#include "World/PinkCabCityIdentity.h"

enum class EPinkCabEnforcementSettlementResult : uint8
{
    Committed,
    Duplicate,
    InvalidEvent,
    MissingRule,
    EconomyRejected
};

struct FPinkCabEnforcementSettlement
{
    FPinkCabTransactionId TransactionId;
    EPinkCabTransactionType TransactionType = EPinkCabTransactionType::Fine;
    int64 FineAmountMinor = 0;
    int32 ReputationDelta = 0;
};

class PINKCAB_API FPinkCabEnforcementService
{
public:
    static bool Evaluate(
        const FPinkCabRoadRuleProfile& Profile,
        const FPinkCabRoadObservation& Observation,
        TArray<FPinkCabEnforcementEvent>& OutEvents);

    static EPinkCabEnforcementSettlementResult Settle(
        const FPinkCabRoadRuleProfile& Profile,
        const FPinkCabEnforcementEvent& Event,
        FPinkCabEconomyLedger& Economy,
        FPinkCabEnforcementLedger& Reputation,
        FPinkCabEnforcementSettlement& OutSettlement);

    static FPinkCabTransactionId MakeFineTransactionId(
        const FPinkCabStableId& EventId);

private:
    static bool IsViolation(
        const FPinkCabRoadRuleDefinition& Rule,
        const FPinkCabRoadObservation& Observation);
    static float ResolveSeverity(
        const FPinkCabRoadRuleDefinition& Rule,
        const FPinkCabRoadObservation& Observation);
    static FPinkCabStableId MakeEventId(
        const FPinkCabRoadRuleDefinition& Rule,
        const FPinkCabRoadObservation& Observation);
};
