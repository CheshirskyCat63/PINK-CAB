#include "Persistence/PinkCabCheckpointRing.h"

void FPinkCabCheckpointRing::Push(const TArray<uint8>& Bytes)
{
    Checkpoints.Add(Bytes);
    while (Checkpoints.Num() > FPinkCabPersistencePolicy::RollingCheckpointCount)
    {
        Checkpoints.RemoveAt(0);
    }
}

int32 FPinkCabCheckpointRing::Num() const
{
    return Checkpoints.Num();
}

const TArray<uint8>& FPinkCabCheckpointRing::GetOldest() const
{
    return Checkpoints[0];
}

const TArray<uint8>& FPinkCabCheckpointRing::GetNewest() const
{
    return Checkpoints.Last();
}

const TArray<uint8>* FPinkCabCheckpointRing::GetFromNewestOffset(int32 Offset) const
{
    if (Offset < 0 || Offset >= Checkpoints.Num())
    {
        return nullptr;
    }
    return &Checkpoints[Checkpoints.Num() - 1 - Offset];
}
