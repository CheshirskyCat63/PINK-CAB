#pragma once

#include "CoreMinimal.h"
#include "Core/PinkCabDeterministicSeed.h"
#include "Core/PinkCabStableId.h"
#include "Taxi/PinkCabPassengerIdentity.h"
#include "Taxi/PinkCabPassengerTemplate.h"

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

class FPinkCabPassengerRegistry
{
public:
    explicit FPinkCabPassengerRegistry(int32 InMaxRecords = 256, int32 InMaxPreferences = 8)
        : MaxRecords(FMath::Max(1, InMaxRecords))
        , MaxPreferences(FMath::Max(0, InMaxPreferences))
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
        OutRecord = nullptr;        const FString CleanContext = ContextKey.TrimStartAndEnd();
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
            CleanContext + TEXT(":") + Template.TemplateId.ToString());        Record.IdentitySeed = FPinkCabDeterministicSeed::Derive(
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
    {        return IdentityId.IsValid() ? Records.Find(IdentityId.Serialize()) : nullptr;
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
                + FString::Printf(TEXT("|%llu|%llu|%s|%.3f|"),
                    Record.IdentitySeed, Record.AppearanceSeed,
                    *Record.AppearanceProfileId.ToString(), Record.ResolvedMassKg);
            for (const FName Trait : Record.AppearanceTraitIds)
                Payload += Trait.ToString() + TEXT(",");
            Payload += TEXT("|");
            for (const FName Tag : Record.PreferenceTags)
                Payload += Tag.ToString() + TEXT(",");
            Payload += TEXT(";");
        }
        return FPinkCabDeterministicSeed::FromText(Payload);
    }

private:
    int32 MaxRecords = 256;
    int32 MaxPreferences = 8;
    TMap<FString, FPinkCabPassengerRecord> Records;
};