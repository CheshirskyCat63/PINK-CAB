template <typename TEnum>
void FPinkCabGameSnapshotArchivePrimitives::SerializeEnum(
    FArchive& Ar,
    TEnum& Value)
{
    uint8 Raw = static_cast<uint8>(Value);
    Ar << Raw;
    if (Ar.IsLoading())
    {
        Value = static_cast<TEnum>(Raw);
    }
}

template <typename TItem, typename TSerializer>
void FPinkCabGameSnapshotArchivePrimitives::SerializeArray(
    FArchive& Ar,
    TArray<TItem>& Items,
    int32 MaxCount,
    TSerializer Serializer)
{
    int32 Count = Items.Num();
    Ar << Count;
    if (Count < 0 || Count > MaxCount)
    {
        Ar.SetError();
        return;
    }

    if (Ar.IsLoading())
    {
        Items.SetNum(Count);
    }

    for (int32 Index = 0; Index < Count && !Ar.IsError(); ++Index)
    {
        Serializer(Ar, Items[Index]);
    }
}
