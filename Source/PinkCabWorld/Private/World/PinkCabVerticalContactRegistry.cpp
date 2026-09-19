#include "World/PinkCabVerticalContactRegistry.h"

bool FPinkCabVerticalContactRecord::IsValid() const
{
    return !ContactId.IsEmpty()
        && LaneId.IsValid()
        && !SemanticKey.IsEmpty()
        && Layer >= 0;
}

FPinkCabVerticalContactRegistry::FPinkCabVerticalContactRegistry(int32 InMaxContacts)
    : MaxContacts(FMath::Max(1, InMaxContacts))
{
}

FString FPinkCabVerticalContactRegistry::MakeContactId(
    const FPinkCabCityIdentity& City,
    const FPinkCabLaneId& LaneId,
    EPinkCabVerticalContactKind Kind,
    const FString& SemanticKey)
{
    const FString CleanKey = SemanticKey.TrimStartAndEnd();
    if (!City.IsValid() || !LaneId.IsValid() || CleanKey.IsEmpty())
    {
        return FString();
    }

    const FString Payload = FString::Printf(
        TEXT("%s|%s|%d|%s"),
        *City.GetStableKey(),
        *LaneId.Serialize(),
        static_cast<int32>(Kind),
        *CleanKey);
    return PinkCabWorldId::StableToken(TEXT("vertical-contact:"), Payload);
}

bool FPinkCabVerticalContactRegistry::TryAdd(
    const FPinkCabCityIdentity& City,
    const FPinkCabLaneId& LaneId,
    EPinkCabVerticalContactKind Kind,
    const FString& SemanticKey,
    int32 Layer,
    FString& OutContactId)
{
    const FString CleanKey = SemanticKey.TrimStartAndEnd();
    const FString ContactId = MakeContactId(City, LaneId, Kind, CleanKey);
    if (ContactId.IsEmpty()
        || Layer < 0
        || Contacts.Num() >= MaxContacts
        || Contacts.Contains(ContactId))
    {
        return false;
    }

    FPinkCabVerticalContactRecord Record;
    Record.ContactId = ContactId;
    Record.LaneId = LaneId;
    Record.Kind = Kind;
    Record.SemanticKey = CleanKey;
    Record.Layer = Layer;
    Contacts.Add(ContactId, Record);
    OutContactId = ContactId;
    return true;
}

bool FPinkCabVerticalContactRegistry::TryGet(
    const FString& ContactId,
    FPinkCabVerticalContactRecord& OutRecord) const
{
    const FPinkCabVerticalContactRecord* Found = Contacts.Find(ContactId);
    if (!Found) return false;

    OutRecord = *Found;
    return true;
}

int32 FPinkCabVerticalContactRegistry::Num() const { return Contacts.Num(); }

FString FPinkCabVerticalContactRegistry::GetReconstructionSignature() const
{
    TArray<FString> Keys;
    Contacts.GetKeys(Keys);
    Keys.Sort();
    FString Payload;
    for (const FString& Key : Keys)
    {
        const FPinkCabVerticalContactRecord& Record = Contacts[Key];
        Payload += FString::Printf(
            TEXT("%s|%s|%d|%s|%d;"),
            *Record.ContactId,
            *Record.LaneId.Serialize(),
            static_cast<int32>(Record.Kind),
            *Record.SemanticKey,
            Record.Layer);
    }
    return PinkCabWorldId::StableToken(TEXT("vertical-contacts:"), Payload);
}
