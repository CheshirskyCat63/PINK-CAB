#include "World/PinkCabCityDeltaState.h"

FPinkCabCityDeltaState::FPinkCabCityDeltaState(int32 InMaxDeltas)
    : MaxDeltas(FMath::Max(1, InMaxDeltas))
{
}

FString FPinkCabCityDeltaState::MakeDeltaId(
    const FPinkCabCityIdentity& City,
    EPinkCabCityDeltaKind Kind,
    const FString& SubjectStableId,
    const FString& OperationKey)
{
    const FString CleanOperation = OperationKey.TrimStartAndEnd();
    if (!City.IsValid() || SubjectStableId.IsEmpty() || CleanOperation.IsEmpty())
    {
        return FString();
    }
    const FString Payload = FString::Printf(
        TEXT("%s|%s|%s|%s"),
        *City.GetStableKey(),
        *KindToken(Kind),
        *SubjectStableId,
        *CleanOperation);
    return PinkCabWorldId::StableToken(TEXT("delta:"), Payload);
}

bool FPinkCabCityDeltaState::TryAddLaneClosure(
    const FPinkCabCityIdentity& City,
    const FPinkCabLaneId& LaneId,
    const FString& OperationKey,
    FString& OutDeltaId)
{
    const FString DeltaId = MakeDeltaId(
        City,
        EPinkCabCityDeltaKind::LaneClosure,
        LaneId.Serialize(),
        OperationKey);
    if (DeltaId.IsEmpty() || Deltas.Contains(DeltaId) || Deltas.Num() >= MaxDeltas)
    {
        return false;
    }

    FPinkCabCityDeltaRecord Record;
    Record.DeltaId = DeltaId;
    Record.Kind = EPinkCabCityDeltaKind::LaneClosure;
    Record.SubjectStableId = LaneId.Serialize();
    Record.OperationKey = OperationKey.TrimStartAndEnd();
    Deltas.Add(DeltaId, MoveTemp(Record));
    ClosedLaneIds.Add(LaneId.Serialize());
    OutDeltaId = DeltaId;
    return true;
}

bool FPinkCabCityDeltaState::IsLaneClosed(const FPinkCabLaneId& LaneId) const
{
    return LaneId.IsValid() && ClosedLaneIds.Contains(LaneId.Serialize());
}

int32 FPinkCabCityDeltaState::Num() const { return Deltas.Num(); }

FString FPinkCabCityDeltaState::GetReconstructionSignature() const
{
    TArray<FString> Keys;
    Deltas.GetKeys(Keys);
    Keys.Sort();
    FString Payload;
    for (const FString& Key : Keys)
    {
        const FPinkCabCityDeltaRecord& Record = Deltas[Key];
        Payload += Record.DeltaId;
        Payload += TEXT("|");
        Payload += KindToken(Record.Kind);
        Payload += TEXT("|");
        Payload += Record.SubjectStableId;
        Payload += TEXT("|");
        Payload += Record.OperationKey;
        Payload += TEXT(";");
    }
    return PinkCabWorldId::StableToken(TEXT("city-deltas:"), Payload);
}

FString FPinkCabCityDeltaState::KindToken(EPinkCabCityDeltaKind Kind)
{
    switch (Kind)
    {
    case EPinkCabCityDeltaKind::LaneClosure: return TEXT("lane-closure");
    case EPinkCabCityDeltaKind::LaneSlowdown: return TEXT("lane-slowdown");
    case EPinkCabCityDeltaKind::ModuleOverride: return TEXT("module-override");
    case EPinkCabCityDeltaKind::Incident: return TEXT("incident");
    default: return TEXT("unknown");
    }
}
