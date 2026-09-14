#pragma once

#include "CoreMinimal.h"
#include "Persistence/PinkCabGameSnapshot.h"

class FPinkCabGameSnapshotArchivePrimitives
{
protected:
    static constexpr int32 MaxArrayElements = 65536;

    template <typename TEnum>
    static void SerializeEnum(FArchive& Ar, TEnum& Value)
    {
        uint8 Raw = static_cast<uint8>(Value);
        Ar << Raw;
        if (Ar.IsLoading()) Value = static_cast<TEnum>(Raw);
    }

    template <typename TItem, typename TSerializer>
    static void SerializeArray(
        FArchive& Ar, TArray<TItem>& Items, int32 MaxCount, TSerializer Serializer)
    {
        int32 Count = Items.Num();
        Ar << Count;
        if (Count < 0 || Count > MaxCount)
        {
            Ar.SetError();
            return;
        }
        if (Ar.IsLoading()) Items.SetNum(Count);
        for (int32 Index = 0; Index < Count && !Ar.IsError(); ++Index)
        {
            Serializer(Ar, Items[Index]);
        }
    }

    static bool IsCapacityReasonable(int32 Value)
    {
        return Value >= 0 && Value <= MaxArrayElements;
    }
    static void SerializeStableId(FArchive& Ar, FPinkCabStableId& Id)
    {
        FString Value = Id.Serialize();
        Ar << Value;
        if (Ar.IsLoading()) Id = FPinkCabStableId(Value);
    }

    static void SerializeCityIdentity(FArchive& Ar, FPinkCabCityIdentity& City)
    {
        FString CityCode = City.GetCityCode();
        FString Generator = City.GetGeneratorVersion();
        FString Content = City.GetContentSetVersion();
        Ar << CityCode;
        Ar << Generator;
        Ar << Content;
        if (Ar.IsLoading())
        {
            City = FPinkCabCityIdentity::Create(CityCode, Generator, Content);
        }
    }
};
