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

struct PINKCABTAXI_API FPinkCabPassengerRecord
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
        const FPinkCabPassengerRelationship& Delta);
    EPinkCabPassengerMutationResult RegisterAuthoredEventOnce(
        const FPinkCabStableId& EventId,
        const FPinkCabPassengerRelationship& Delta);
    EPinkCabPassengerMutationResult AddRideMemoryOnce(
        const FPinkCabStableId& MemoryId,
        const FPinkCabStableId& FareId,
        FName OutcomeTag);

    int32 GetPaidFareCount() const;
    int32 GetAuthoredEventCount() const;
    bool IsRepeatEligible() const;
    const TArray<FPinkCabPassengerRideMemory>& GetRideMemories() const;
    const TArray<FString>& GetAppliedSocialEventIds() const;

    void SetNeuralPermission(bool bGranted);
    bool HasNeuralPermission() const;
    void SetNeuralBlocked(bool bBlocked);
    bool IsNeuralBlocked() const;
    EPinkCabPassengerMutationResult AddMessageOnce(
        const FPinkCabStableId& MessageId,
        const FString& Text);
    const TArray<FPinkCabPassengerNeuralMessage>& GetNeuralMessages() const;
    const TArray<FString>& GetAppliedNeuralMessageIds() const;

private:
    static bool IsRelationshipFinite(const FPinkCabPassengerRelationship& Value);
    EPinkCabPassengerMutationResult TryBeginSocialEvent(
        const FPinkCabStableId& EventId);
    EPinkCabPassengerMutationResult TryBeginSocialEvent(
        const FPinkCabStableId& EventId,
        const FPinkCabPassengerRelationship& Delta);
    void ApplyRelationshipDelta(const FPinkCabPassengerRelationship& Delta);

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
    friend class FPinkCabPassengerSnapshotArchive;
};

PINKCABTAXI_API bool PinkCabPassengerRecordIsValid(
    const FPinkCabPassengerRecord& Record);
