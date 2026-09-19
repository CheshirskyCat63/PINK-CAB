#pragma once

#include "CoreMinimal.h"
#include "Persistence/PinkCabGameSnapshot.h"

class PINKCAB_API FPinkCabGameSnapshotArchivePrimitives
{
protected:
    static constexpr int32 MaxArrayElements = 65536;

    template <typename TEnum>
    static void SerializeEnum(FArchive& Ar, TEnum& Value);

    template <typename TItem, typename TSerializer>
    static void SerializeArray(
        FArchive& Ar,
        TArray<TItem>& Items,
        int32 MaxCount,
        TSerializer Serializer);

    static bool IsCapacityReasonable(int32 Value);
    static void SerializeStableId(
        FArchive& Ar,
        FPinkCabStableId& Id);
    static void SerializeCityIdentity(
        FArchive& Ar,
        FPinkCabCityIdentity& City);
};

#include "Persistence/Detail/PinkCabGameSnapshotArchivePrimitives.inl"
