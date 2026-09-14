#pragma once

#include "CoreMinimal.h"
#include "Core/PinkCabDeterministicSeed.h"
#include "Core/PinkCabStableId.h"
#include "Taxi/PinkCabPassengerIdentity.h"
#include "Taxi/PinkCabPassengerTemplate.h"

enum class EPinkCabPassengerMutationResult : uint8
{
    Applied,
    Duplicate,
    CapacityExceeded,
    PermissionDenied,
    Blocked,
    Invalid
};

struct FPinkCabPassengerNeuralMessage
{
    FPinkCabStableId MessageId;
    FString Text;
};

struct FPinkCabPassengerRideMemory
{
    FPinkCabStableId MemoryId;
    FPinkCabStableId FareId;
    FName OutcomeTag = NAME_None;
};

struct FPinkCabPassengerRecord
{
    FPinkCabStableId IdentityId;
    FName TemplateId = NAME_None;
    FString ContextKey;
    uint64 IdentitySeed = 0;
    uint64 AppearanceSeed = 0;
    FName AppearanceProfileId = NAME_None;
    TArray<FName> AppearanceTraitIds;
    float ResolvedMassKg = 0.0f;
    TArray<FName> PreferenceTags;
    FPinkCabPassengerRelationship Relationship;

    EPinkCabPassengerMutationResult RegisterPaidFareOnce(
        const FPinkCabStableId& EventId,
        const FPinkCabPassengerRelationship& Delta)
    {
        const EPinkCabPassengerMutationResult Begin = TryBeginSocialEvent(EventId, Delta);
        if (Begin != EPinkCabPassengerMutationResult::Applied) return Begin;
        ++PaidFareCount;
        ApplyRelationshipDelta(Delta);
        if (PaidFareCount >= 2) bRepeatEligible = true;
        return EPinkCabPassengerMutationResult::Applied;
    }

    EPinkCabPassengerMutationResult RegisterAuthoredEventOnce(
        const FPinkCabStableId& EventId,
        const FPinkCabPassengerRelationship& Delta)
    {
        const EPinkCabPassengerMutationResult Begin = TryBeginSocialEvent(EventId, Delta);
        if (Begin != EPinkCabPassengerMutationResult::Applied) return Begin;
        ++AuthoredEventCount;
        ApplyRelationshipDelta(Delta);
        bRepeatEligible = true;
        return EPinkCabPassengerMutationResult::Applied;
    }
    EPinkCabPassengerMutationResult AddRideMemoryOnce(
        const FPinkCabStableId& MemoryId,
        const FPinkCabStableId& FareId,
        const FName OutcomeTag)
    {
        if (!MemoryId.IsValid() || !FareId.IsValid() || OutcomeTag.IsNone())
            return EPinkCabPassengerMutationResult::Invalid;
        const EPinkCabPassengerMutationResult Begin = TryBeginSocialEvent(MemoryId);
        if (Begin != EPinkCabPassengerMutationResult::Applied) return Begin;
        if (RideMemories.Num() >= MaxRideMemories) RideMemories.RemoveAt(0);
        RideMemories.Add({MemoryId, FareId, OutcomeTag});
        return EPinkCabPassengerMutationResult::Applied;
    }

    int32 GetPaidFareCount() const { return PaidFareCount; }
    int32 GetAuthoredEventCount() const { return AuthoredEventCount; }
    bool IsRepeatEligible() const { return bRepeatEligible; }
    const TArray<FPinkCabPassengerRideMemory>& GetRideMemories() const { return RideMemories; }
    const TArray<FString>& GetAppliedSocialEventIds() const { return AppliedSocialEventIds; }

    void SetNeuralPermission(bool bGranted) { bNeuralPermissionGranted = bGranted; }
    bool HasNeuralPermission() const { return bNeuralPermissionGranted; }
    void SetNeuralBlocked(bool bBlocked) { bNeuralBlocked = bBlocked; }
    bool IsNeuralBlocked() const { return bNeuralBlocked; }

    EPinkCabPassengerMutationResult AddMessageOnce(
        const FPinkCabStableId& MessageId,
        const FString& Text)
    {
        const FString Normalized = Text.TrimStartAndEnd();
        if (!IdentityId.IsValid() || !MessageId.IsValid() || Normalized.IsEmpty())
            return EPinkCabPassengerMutationResult::Invalid;
        if (!bNeuralPermissionGranted) return EPinkCabPassengerMutationResult::PermissionDenied;
        if (bNeuralBlocked) return EPinkCabPassengerMutationResult::Blocked;

        const FString Key = MessageId.Serialize();
        if (AppliedNeuralMessageIds.Contains(Key)) return EPinkCabPassengerMutationResult::Duplicate;
        if (AppliedNeuralMessageIds.Num() >= MaxNeuralReplayJournalEntries)
            return EPinkCabPassengerMutationResult::CapacityExceeded;

        AppliedNeuralMessageIds.Add(Key);
        if (NeuralMessages.Num() >= MaxNeuralMessages) NeuralMessages.RemoveAt(0);
        NeuralMessages.Add({MessageId, Normalized});
        return EPinkCabPassengerMutationResult::Applied;
    }

    const TArray<FPinkCabPassengerNeuralMessage>& GetNeuralMessages() const { return NeuralMessages; }
    const TArray<FString>& GetAppliedNeuralMessageIds() const { return AppliedNeuralMessageIds; }

private:
    static bool IsRelationshipFinite(const FPinkCabPassengerRelationship& Value)
    {
        return FMath::IsFinite(Value.Trust)
            && FMath::IsFinite(Value.Satisfaction)
            && FMath::IsFinite(Value.RiskTolerance);
    }
    EPinkCabPassengerMutationResult TryBeginSocialEvent(const FPinkCabStableId& EventId)
    {
        if (!EventId.IsValid()) return EPinkCabPassengerMutationResult::Invalid;
        const FString Key = EventId.Serialize();
        if (AppliedSocialEventIds.Contains(Key)) return EPinkCabPassengerMutationResult::Duplicate;
        if (AppliedSocialEventIds.Num() >= MaxReplayJournalEntries)
            return EPinkCabPassengerMutationResult::CapacityExceeded;
        AppliedSocialEventIds.Add(Key);
        return EPinkCabPassengerMutationResult::Applied;
    }

    EPinkCabPassengerMutationResult TryBeginSocialEvent(
        const FPinkCabStableId& EventId,
        const FPinkCabPassengerRelationship& Delta)
    {
        if (!IsRelationshipFinite(Delta)) return EPinkCabPassengerMutationResult::Invalid;
        return TryBeginSocialEvent(EventId);
    }

    void ApplyRelationshipDelta(const FPinkCabPassengerRelationship& Delta)
    {
        Relationship.Trust = FMath::Clamp(Relationship.Trust + Delta.Trust, -1.0f, 1.0f);
        Relationship.Satisfaction = FMath::Clamp(Relationship.Satisfaction + Delta.Satisfaction, -1.0f, 1.0f);
        Relationship.RiskTolerance = FMath::Clamp(Relationship.RiskTolerance + Delta.RiskTolerance, -1.0f, 1.0f);
    }

    int32 MaxRideMemories = 32;
    int32 MaxReplayJournalEntries = 512;
    int32 PaidFareCount = 0;
    int32 AuthoredEventCount = 0;
    bool bRepeatEligible = false;
    TArray<FPinkCabPassengerRideMemory> RideMemories;
    TArray<FString> AppliedSocialEventIds;
    int32 MaxNeuralMessages = 64;
    int32 MaxNeuralReplayJournalEntries = 256;
    bool bNeuralPermissionGranted = false;
    bool bNeuralBlocked = false;
    TArray<FPinkCabPassengerNeuralMessage> NeuralMessages;
    TArray<FString> AppliedNeuralMessageIds;

    friend class FPinkCabPassengerRegistry;
    friend class FPinkCabPassengerSnapshotCodec;
    friend class FPinkCabGameSnapshotArchive;
};

inline bool PinkCabPassengerRecordIsValid(const FPinkCabPassengerRecord& Record)
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
