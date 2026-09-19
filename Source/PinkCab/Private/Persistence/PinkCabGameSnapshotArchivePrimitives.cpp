#include "Persistence/PinkCabGameSnapshotArchivePrimitives.h"

bool FPinkCabGameSnapshotArchivePrimitives::IsCapacityReasonable(
    int32 Value)
{
    return Value >= 0 && Value <= MaxArrayElements;
}

void FPinkCabGameSnapshotArchivePrimitives::SerializeStableId(
    FArchive& Ar,
    FPinkCabStableId& Id)
{
    FString Value = Id.Serialize();
    Ar << Value;
    if (Ar.IsLoading())
    {
        Id = FPinkCabStableId(Value);
    }
}

void FPinkCabGameSnapshotArchivePrimitives::SerializeCityIdentity(
    FArchive& Ar,
    FPinkCabCityIdentity& City)
{
    FString CityCode = City.GetCityCode();
    FString Generator = City.GetGeneratorVersion();
    FString Content = City.GetContentSetVersion();
    Ar << CityCode;
    Ar << Generator;
    Ar << Content;
    if (Ar.IsLoading())
    {
        City = FPinkCabCityIdentity::Create(
            CityCode,
            Generator,
            Content);
    }
}
