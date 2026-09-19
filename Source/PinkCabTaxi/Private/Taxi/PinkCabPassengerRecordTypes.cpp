#include "Taxi/PinkCabPassengerRecordTypes.h"

EPinkCabPassengerMutationResult FPinkCabPassengerRecord::RegisterPaidFareOnce(
    const FPinkCabStableId& EventId,
    const FPinkCabPassengerRelationship& Delta)
{
    const EPinkCabPassengerMutationResult Begin =
        TryBeginSocialEvent(EventId, Delta);
    if (Begin != EPinkCabPassengerMutationResult::Applied) return Begin;

    ++PaidFareCount;
    ApplyRelationshipDelta(Delta);
    if (PaidFareCount >= 2)
    {
        bRepeatEligible = true;
    }
    return EPinkCabPassengerMutationResult::Applied;
}

EPinkCabPassengerMutationResult FPinkCabPassengerRecord::RegisterAuthoredEventOnce(
    const FPinkCabStableId& EventId,
    const FPinkCabPassengerRelationship& Delta)
{
    const EPinkCabPassengerMutationResult Begin =
        TryBeginSocialEvent(EventId, Delta);
    if (Begin != EPinkCabPassengerMutationResult::Applied) return Begin;

    ++AuthoredEventCount;
    ApplyRelationshipDelta(Delta);
    bRepeatEligible = true;
    return EPinkCabPassengerMutationResult::Applied;
}

EPinkCabPassengerMutationResult FPinkCabPassengerRecord::AddRideMemoryOnce(
    const FPinkCabStableId& MemoryId,
    const FPinkCabStableId& FareId,
    FName OutcomeTag)
{
    if (!MemoryId.IsValid() || !FareId.IsValid() || OutcomeTag.IsNone())
    {
        return EPinkCabPassengerMutationResult::Invalid;
    }

    const EPinkCabPassengerMutationResult Begin =
        TryBeginSocialEvent(MemoryId);
    if (Begin != EPinkCabPassengerMutationResult::Applied) return Begin;

    if (RideMemories.Num() >= MaxRideMemories)
    {
        RideMemories.RemoveAt(0);
    }
    RideMemories.Add({MemoryId, FareId, OutcomeTag});
    return EPinkCabPassengerMutationResult::Applied;
}

int32 FPinkCabPassengerRecord::GetPaidFareCount() const { return PaidFareCount; }
int32 FPinkCabPassengerRecord::GetAuthoredEventCount() const { return AuthoredEventCount; }
bool FPinkCabPassengerRecord::IsRepeatEligible() const { return bRepeatEligible; }

const TArray<FPinkCabPassengerRideMemory>&
FPinkCabPassengerRecord::GetRideMemories() const
{
    return RideMemories;
}

const TArray<FString>& FPinkCabPassengerRecord::GetAppliedSocialEventIds() const
{
    return AppliedSocialEventIds;
}

void FPinkCabPassengerRecord::SetNeuralPermission(bool bGranted)
{
    bNeuralPermissionGranted = bGranted;
}

bool FPinkCabPassengerRecord::HasNeuralPermission() const
{
    return bNeuralPermissionGranted;
}

void FPinkCabPassengerRecord::SetNeuralBlocked(bool bBlocked)
{
    bNeuralBlocked = bBlocked;
}

bool FPinkCabPassengerRecord::IsNeuralBlocked() const
{
    return bNeuralBlocked;
}

EPinkCabPassengerMutationResult FPinkCabPassengerRecord::AddMessageOnce(
    const FPinkCabStableId& MessageId,
    const FString& Text)
{
    const FString Normalized = Text.TrimStartAndEnd();
    if (!IdentityId.IsValid() || !MessageId.IsValid() || Normalized.IsEmpty())
    {
        return EPinkCabPassengerMutationResult::Invalid;
    }
    if (!bNeuralPermissionGranted)
    {
        return EPinkCabPassengerMutationResult::PermissionDenied;
    }
    if (bNeuralBlocked)
    {
        return EPinkCabPassengerMutationResult::Blocked;
    }

    const FString Key = MessageId.Serialize();
    if (AppliedNeuralMessageIds.Contains(Key))
    {
        return EPinkCabPassengerMutationResult::Duplicate;
    }
    if (AppliedNeuralMessageIds.Num() >= MaxNeuralReplayJournalEntries)
    {
        return EPinkCabPassengerMutationResult::CapacityExceeded;
    }

    AppliedNeuralMessageIds.Add(Key);
    if (NeuralMessages.Num() >= MaxNeuralMessages)
    {
        NeuralMessages.RemoveAt(0);
    }
    NeuralMessages.Add({MessageId, Normalized});
    return EPinkCabPassengerMutationResult::Applied;
}

const TArray<FPinkCabPassengerNeuralMessage>&
FPinkCabPassengerRecord::GetNeuralMessages() const
{
    return NeuralMessages;
}

const TArray<FString>& FPinkCabPassengerRecord::GetAppliedNeuralMessageIds() const
{
    return AppliedNeuralMessageIds;
}

bool FPinkCabPassengerRecord::IsRelationshipFinite(
    const FPinkCabPassengerRelationship& Value)
{
    return FMath::IsFinite(Value.Trust)
        && FMath::IsFinite(Value.Satisfaction)
        && FMath::IsFinite(Value.RiskTolerance);
}

EPinkCabPassengerMutationResult FPinkCabPassengerRecord::TryBeginSocialEvent(
    const FPinkCabStableId& EventId)
{
    if (!EventId.IsValid())
    {
        return EPinkCabPassengerMutationResult::Invalid;
    }

    const FString Key = EventId.Serialize();
    if (AppliedSocialEventIds.Contains(Key))
    {
        return EPinkCabPassengerMutationResult::Duplicate;
    }
    if (AppliedSocialEventIds.Num() >= MaxReplayJournalEntries)
    {
        return EPinkCabPassengerMutationResult::CapacityExceeded;
    }

    AppliedSocialEventIds.Add(Key);
    return EPinkCabPassengerMutationResult::Applied;
}

EPinkCabPassengerMutationResult FPinkCabPassengerRecord::TryBeginSocialEvent(
    const FPinkCabStableId& EventId,
    const FPinkCabPassengerRelationship& Delta)
{
    if (!IsRelationshipFinite(Delta))
    {
        return EPinkCabPassengerMutationResult::Invalid;
    }
    return TryBeginSocialEvent(EventId);
}

void FPinkCabPassengerRecord::ApplyRelationshipDelta(
    const FPinkCabPassengerRelationship& Delta)
{
    Relationship.Trust =
        FMath::Clamp(Relationship.Trust + Delta.Trust, -1.0f, 1.0f);
    Relationship.Satisfaction =
        FMath::Clamp(Relationship.Satisfaction + Delta.Satisfaction, -1.0f, 1.0f);
    Relationship.RiskTolerance =
        FMath::Clamp(Relationship.RiskTolerance + Delta.RiskTolerance, -1.0f, 1.0f);
}

bool PinkCabPassengerRecordIsValid(const FPinkCabPassengerRecord& Record)
{
    return Record.IdentityId.IsValid()
        && !Record.TemplateId.IsNone()
        && !Record.ContextKey.IsEmpty()
        && Record.IdentitySeed != 0
        && Record.AppearanceSeed != 0
        && !Record.AppearanceProfileId.IsNone()
        && Record.AppearanceTraitIds.Num() >= 3
        && FMath::IsFinite(Record.ResolvedMassKg)
        && Record.ResolvedMassKg > 0.0f;
}
