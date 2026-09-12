#pragma once

#include "CoreMinimal.h"
#include "Serialization/MemoryReader.h"
#include "Serialization/MemoryWriter.h"
#include "Core/PinkCabStableId.h"

struct FPinkCabEnforcementEvent
{
    FPinkCabStableId EventId;
    FName TypeId = NAME_None;
    int64 TimestampMs = 0;
    FPinkCabStableId CityLocationId;
    FPinkCabStableId VehicleId;
    FPinkCabStableId PassengerId;
    float Severity = 0.0f;
    uint32 EvidenceFlags = 0;
    FString Context;

    bool IsValid() const
    {
        return EventId.IsValid() && TypeId != NAME_None
            && CityLocationId.IsValid() && VehicleId.IsValid()
            && Severity >= 0.0f && Severity <= 1.0f;
    }
};

class FPinkCabEnforcementEventCodec
{
public:
    static bool Serialize(const FPinkCabEnforcementEvent& Event, TArray<uint8>& OutBytes)
    {
        if (!Event.IsValid()) return false;
        OutBytes.Reset();
        FMemoryWriter Writer(OutBytes, true);
        uint32 Magic = PayloadMagic;
        Writer << Magic;
        FString EventId = Event.EventId.Serialize();
        FString TypeId = Event.TypeId.ToString();
        FString LocationId = Event.CityLocationId.Serialize();
        FString VehicleId = Event.VehicleId.Serialize();
        FString PassengerId = Event.PassengerId.Serialize();
        int64 TimestampMs = Event.TimestampMs;
        float Severity = Event.Severity;
        uint32 EvidenceFlags = Event.EvidenceFlags;
        FString Context = Event.Context;
        Writer << EventId << TypeId << TimestampMs << LocationId << VehicleId << PassengerId;
        Writer << Severity << EvidenceFlags << Context;
        Writer.Close();
        return !Writer.IsError() && OutBytes.Num() > 0;
    }

    static bool Deserialize(const TArray<uint8>& Bytes, FPinkCabEnforcementEvent& OutEvent)
    {
        if (Bytes.Num() == 0) return false;
        FMemoryReader Reader(Bytes, true);
        uint32 Magic = 0;
        Reader << Magic;
        if (Magic != PayloadMagic) return false;
        FString EventId, TypeId, LocationId, VehicleId, PassengerId;
        int64 TimestampMs = 0;
        float Severity = 0.0f;
        uint32 EvidenceFlags = 0;
        FString Context;
        Reader << EventId << TypeId << TimestampMs << LocationId << VehicleId << PassengerId;
        Reader << Severity << EvidenceFlags << Context;
        if (Reader.IsError() || TypeId.TrimStartAndEnd().IsEmpty()) return false;

        FPinkCabEnforcementEvent Restored;
        if (!FPinkCabStableId::TryParse(EventId, Restored.EventId)
            || !FPinkCabStableId::TryParse(LocationId, Restored.CityLocationId)
            || !FPinkCabStableId::TryParse(VehicleId, Restored.VehicleId)) return false;
        if (!PassengerId.IsEmpty()
            && !FPinkCabStableId::TryParse(PassengerId, Restored.PassengerId)) return false;
        Restored.TypeId = FName(*TypeId);
        Restored.TimestampMs = TimestampMs;
        Restored.Severity = Severity;
        Restored.EvidenceFlags = EvidenceFlags;
        Restored.Context = Context;
        if (!Restored.IsValid()) return false;
        OutEvent = MoveTemp(Restored);
        return true;
    }

private:
    static constexpr uint32 PayloadMagic = 0x50434556u;
};
