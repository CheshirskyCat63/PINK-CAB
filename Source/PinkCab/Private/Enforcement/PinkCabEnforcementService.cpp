#include "Enforcement/PinkCabEnforcementService.h"

bool FPinkCabEnforcementService::Evaluate(
    const FPinkCabRoadRuleProfile& Profile,
    const FPinkCabRoadObservation& Observation,
    TArray<FPinkCabEnforcementEvent>& OutEvents)
{
    OutEvents.Reset();
    if (!Observation.IsValid())
    {
        return false;
    }

    static const EPinkCabRoadRuleKind Kinds[] = {
        EPinkCabRoadRuleKind::SpeedLimit,
        EPinkCabRoadRuleKind::LaneRestriction,
        EPinkCabRoadRuleKind::TurnRestriction,
        EPinkCabRoadRuleKind::ParkingRestriction,
        EPinkCabRoadRuleKind::TransitRestriction};

    for (EPinkCabRoadRuleKind Kind : Kinds)
    {
        FPinkCabRoadRuleDefinition Rule;
        if (!Profile.TryGetRule(Observation.LaneId, Kind, Rule)
            || !IsViolation(Rule, Observation))
        {
            continue;
        }

        FPinkCabEnforcementEvent Event;
        Event.EventId = MakeEventId(Rule, Observation);
        Event.TypeId = Rule.TypeId;
        Event.TimestampMs = Observation.TimestampMs;
        Event.CityLocationId = Observation.CityLocationId;
        Event.VehicleId = Observation.VehicleId;
        Event.Severity = ResolveSeverity(Rule, Observation);
        Event.EvidenceFlags = 1u << static_cast<uint8>(Kind);
        if (!Event.IsValid())
        {
            return false;
        }
        OutEvents.Add(MoveTemp(Event));
    }
    return true;
}

EPinkCabEnforcementSettlementResult FPinkCabEnforcementService::Settle(
    const FPinkCabRoadRuleProfile& Profile,
    const FPinkCabEnforcementEvent& Event,
    FPinkCabEconomyLedger& Economy,
    FPinkCabEnforcementLedger& Reputation,
    FPinkCabEnforcementSettlement& OutSettlement)
{
    FPinkCabRoadRuleDefinition Rule;
    if (!Event.IsValid())
    {
        return EPinkCabEnforcementSettlementResult::InvalidEvent;
    }
    if (!Profile.TryGetRuleByType(Event.TypeId, Rule))
    {
        return EPinkCabEnforcementSettlementResult::MissingRule;
    }

    OutSettlement.TransactionId = MakeFineTransactionId(Event.EventId);
    OutSettlement.TransactionType = EPinkCabTransactionType::Fine;
    OutSettlement.FineAmountMinor = Rule.FineAmountMinor;
    OutSettlement.ReputationDelta = Rule.ReputationDelta;

    if (Reputation.HasRecordedEvent(Event.EventId))
    {
        return EPinkCabEnforcementSettlementResult::Duplicate;
    }

    const FPinkCabEconomyTransaction Fine =
        FPinkCabEconomyTransaction::Debit(
            OutSettlement.TransactionId,
            EPinkCabTransactionType::Fine,
            Rule.FineAmountMinor,
            EPinkCabEconomyPurpose::MandatoryDayObligation);
    const EPinkCabSettlementResult EconomyResult = Economy.Commit(Fine);
    if (EconomyResult == EPinkCabSettlementResult::Duplicate)
    {
        return EPinkCabEnforcementSettlementResult::Duplicate;
    }
    if (EconomyResult != EPinkCabSettlementResult::Committed)
    {
        return EPinkCabEnforcementSettlementResult::EconomyRejected;
    }
    if (!Reputation.Record(Event, Rule.ReputationDelta))
    {
        return EPinkCabEnforcementSettlementResult::Duplicate;
    }
    return EPinkCabEnforcementSettlementResult::Committed;
}

FPinkCabTransactionId FPinkCabEnforcementService::MakeFineTransactionId(
    const FPinkCabStableId& EventId)
{
    if (!EventId.IsValid())
    {
        return FPinkCabTransactionId();
    }
    return FPinkCabTransactionId(
        PinkCabWorldId::StableToken(
            TEXT("fine:"),
            EventId.Serialize()));
}

bool FPinkCabEnforcementService::IsViolation(
    const FPinkCabRoadRuleDefinition& Rule,
    const FPinkCabRoadObservation& Observation)
{
    switch (Rule.Kind)
    {
    case EPinkCabRoadRuleKind::SpeedLimit:
        return Observation.SpeedCmPerSec > Rule.SpeedLimitCmPerSec;
    case EPinkCabRoadRuleKind::LaneRestriction:
        return Observation.bLaneViolation;
    case EPinkCabRoadRuleKind::TurnRestriction:
        return Observation.bTurnViolation;
    case EPinkCabRoadRuleKind::ParkingRestriction:
        return Observation.bParkingViolation;
    case EPinkCabRoadRuleKind::TransitRestriction:
        return Observation.bTransitViolation;
    default:
        return false;
    }
}

float FPinkCabEnforcementService::ResolveSeverity(
    const FPinkCabRoadRuleDefinition& Rule,
    const FPinkCabRoadObservation& Observation)
{
    if (Rule.Kind != EPinkCabRoadRuleKind::SpeedLimit)
    {
        return FMath::Clamp(Rule.BaseSeverity, 0.0f, 1.0f);
    }

    const double ExcessRatio = Rule.SpeedLimitCmPerSec > 0.0
        ? (Observation.SpeedCmPerSec - Rule.SpeedLimitCmPerSec)
            / Rule.SpeedLimitCmPerSec
        : 0.0;
    return FMath::Clamp(
        Rule.BaseSeverity
            + static_cast<float>(
                FMath::Max(0.0, ExcessRatio) * 0.5),
        0.0f,
        1.0f);
}

FPinkCabStableId FPinkCabEnforcementService::MakeEventId(
    const FPinkCabRoadRuleDefinition& Rule,
    const FPinkCabRoadObservation& Observation)
{
    const FString Payload = FString::Printf(
        TEXT("%s|%s|%s|%s|%s|%lld"),
        *Observation.ObservationKey.TrimStartAndEnd(),
        *Observation.LaneId.Serialize(),
        *Rule.TypeId.ToString(),
        *Observation.CityLocationId.Serialize(),
        *Observation.VehicleId.Serialize(),
        static_cast<long long>(Observation.TimestampMs));
    return FPinkCabStableId(
        PinkCabWorldId::StableToken(
            TEXT("event:"),
            Payload));
}
