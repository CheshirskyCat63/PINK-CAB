#pragma once

#include "Taxi/PinkCabPassengerRecordTypes.h"

class FPinkCabPassengerRegistry
{
public:
    explicit FPinkCabPassengerRegistry(
        int32 InMaxRecords = 256,
        int32 InMaxPreferences = 8,
        int32 InMaxRideMemories = 32,
        int32 InMaxReplayJournalEntries = 512,
        int32 InMaxNeuralMessages = 64,
        int32 InMaxNeuralReplayJournalEntries = 256)
        : MaxRecords(FMath::Max(1, InMaxRecords))
        , MaxPreferences(FMath::Max(0, InMaxPreferences))
        , MaxRideMemories(FMath::Max(1, InMaxRideMemories))
        , MaxReplayJournalEntries(FMath::Max(1, InMaxReplayJournalEntries))
        , MaxNeuralMessages(FMath::Max(1, InMaxNeuralMessages))
        , MaxNeuralReplayJournalEntries(FMath::Max(1, InMaxNeuralReplayJournalEntries))
    {
    }

    int32 Num() const { return Records.Num(); }

    bool TryCreate(
        const FPinkCabStableId& IdentityId,
        const FPinkCabPassengerTemplate& Template,
        const FString& ContextKey,
        const TArray<FName>& PreferenceTags,
        FPinkCabPassengerRecord*& OutRecord)
    {
        OutRecord = nullptr;
        const FString CleanContext = ContextKey.TrimStartAndEnd();
        if (!IdentityId.IsValid() || Template.TemplateId.IsNone()
            || !FMath::IsFinite(Template.DefaultMassKg) || Template.DefaultMassKg <= 0.0f
            || CleanContext.IsEmpty() || Records.Num() >= MaxRecords)
        {
            return false;
        }

        const FString Key = IdentityId.Serialize();
        if (Records.Contains(Key)) return false;

        TArray<FName> CleanPreferences;
        for (const FName Tag : PreferenceTags)
        {
            if (Tag.IsNone()) return false;
            CleanPreferences.AddUnique(Tag);
        }
        if (CleanPreferences.Num() > MaxPreferences) return false;
        CleanPreferences.Sort([](const FName A, const FName B)
        {
            return A.LexicalLess(B);
        });

        FPinkCabPassengerRecord Record;
        Record.IdentityId = IdentityId;
        Record.TemplateId = Template.TemplateId;
        Record.ContextKey = CleanContext;
        const uint64 Root = FPinkCabDeterministicSeed::FromText(
            CleanContext + TEXT(":") + Template.TemplateId.ToString());
        Record.IdentitySeed = FPinkCabDeterministicSeed::Derive(
            Root, IdentityId, TEXT("passenger-record"));
        Record.AppearanceSeed = FPinkCabDeterministicSeed::Derive(
            Record.IdentitySeed, IdentityId, TEXT("passenger-appearance"));
        Record.AppearanceProfileId = FName(*FString::Printf(
            TEXT("profile.%s.%02llu"), *Template.TemplateId.ToString(),
            Record.AppearanceSeed % 16ull));
        Record.AppearanceTraitIds = {
            FName(*FString::Printf(TEXT("body.%02llu"), (Record.AppearanceSeed >> 8) % 16ull)),
            FName(*FString::Printf(TEXT("face.%02llu"), (Record.AppearanceSeed >> 16) % 32ull)),
            FName(*FString::Printf(TEXT("hair.%02llu"), (Record.AppearanceSeed >> 24) % 32ull)),
            FName(*FString::Printf(TEXT("palette.%02llu"), (Record.AppearanceSeed >> 32) % 16ull))};
        Record.ResolvedMassKg = Template.DefaultMassKg;
        Record.PreferenceTags = MoveTemp(CleanPreferences);
        Record.MaxRideMemories = MaxRideMemories;
        Record.MaxReplayJournalEntries = MaxReplayJournalEntries;
        Record.MaxNeuralMessages = MaxNeuralMessages;
        Record.MaxNeuralReplayJournalEntries = MaxNeuralReplayJournalEntries;
        if (!PinkCabPassengerRecordIsValid(Record)) return false;

        Records.Add(Key, MoveTemp(Record));
        OutRecord = Records.Find(Key);
        return OutRecord != nullptr;
    }

    FPinkCabPassengerRecord* Find(const FPinkCabStableId& IdentityId)
    {
        return IdentityId.IsValid() ? Records.Find(IdentityId.Serialize()) : nullptr;
    }

    const FPinkCabPassengerRecord* Find(const FPinkCabStableId& IdentityId) const
    {
        return IdentityId.IsValid() ? Records.Find(IdentityId.Serialize()) : nullptr;
    }

    uint64 GetReconstructionSignature() const
    {
        TArray<FString> Keys;
        Records.GetKeys(Keys);
        Keys.Sort();
        FString Payload;
        for (const FString& Key : Keys)
        {
            const FPinkCabPassengerRecord& Record = Records.FindChecked(Key);
            Payload += Key + TEXT("|") + Record.TemplateId.ToString()
                + TEXT("|") + Record.ContextKey
                + FString::Printf(TEXT("|%llu|%llu|%s|%.3f|%.3f|%.3f|%.3f|%d|%d|%d|"),
                    Record.IdentitySeed, Record.AppearanceSeed,
                    *Record.AppearanceProfileId.ToString(), Record.ResolvedMassKg,
                    Record.Relationship.Trust, Record.Relationship.Satisfaction,
                    Record.Relationship.RiskTolerance, Record.PaidFareCount,
                    Record.AuthoredEventCount, Record.bRepeatEligible ? 1 : 0);
            for (const FName Trait : Record.AppearanceTraitIds)
                Payload += Trait.ToString() + TEXT(",");
            Payload += TEXT("|");
            for (const FName Tag : Record.PreferenceTags)
                Payload += Tag.ToString() + TEXT(",");
            Payload += TEXT("|");
            for (const FPinkCabPassengerRideMemory& Memory : Record.RideMemories)
                Payload += Memory.MemoryId.Serialize() + TEXT(":")
                    + Memory.FareId.Serialize() + TEXT(":") + Memory.OutcomeTag.ToString() + TEXT(",");
            Payload += TEXT("|");
            for (const FString& EventId : Record.AppliedSocialEventIds)
                Payload += EventId + TEXT(",");
            Payload += FString::Printf(TEXT("|neural:%d:%d|"),
                Record.bNeuralPermissionGranted ? 1 : 0,
                Record.bNeuralBlocked ? 1 : 0);
            for (const FPinkCabPassengerNeuralMessage& Message : Record.NeuralMessages)
                Payload += Message.MessageId.Serialize() + TEXT(":")
                    + FString::Printf(TEXT("%llu,"), FPinkCabDeterministicSeed::FromText(Message.Text));
            Payload += TEXT("|");
            for (const FString& MessageId : Record.AppliedNeuralMessageIds)
                Payload += MessageId + TEXT(",");
            Payload += TEXT(";");
        }
        return FPinkCabDeterministicSeed::FromText(Payload);
    }

private:
    int32 MaxRecords = 256;
    int32 MaxPreferences = 8;
    int32 MaxRideMemories = 32;
    int32 MaxReplayJournalEntries = 512;
    int32 MaxNeuralMessages = 64;
    int32 MaxNeuralReplayJournalEntries = 256;
    TMap<FString, FPinkCabPassengerRecord> Records;

    friend class FPinkCabPassengerSnapshotCodec;
    friend class FPinkCabGameSnapshotArchive;
};
