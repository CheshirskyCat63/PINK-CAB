#pragma once

#include "CoreMinimal.h"
#include "Core/PinkCabStableId.h"

struct PINKCAB_API FPinkCabEnforcementEvent
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

    bool IsValid() const;
};

class PINKCAB_API FPinkCabEnforcementEventCodec
{
public:
    static bool Serialize(
        const FPinkCabEnforcementEvent& Event,
        TArray<uint8>& OutBytes);
    static bool Deserialize(
        const TArray<uint8>& Bytes,
        FPinkCabEnforcementEvent& OutEvent);

private:
    static constexpr uint32 PayloadMagic = 0x50434556u;
};
