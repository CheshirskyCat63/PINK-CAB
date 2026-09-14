#pragma once

#include "CoreMinimal.h"
#include "Persistence/PinkCabSaveHeader.h"

class FPinkCabCheckpointRing
{
public:
    void Push(const TArray<uint8>& Bytes)
    {
        Checkpoints.Add(Bytes);
        while (Checkpoints.Num() > FPinkCabPersistencePolicy::RollingCheckpointCount)
        {
            Checkpoints.RemoveAt(0);
        }
    }

    int32 Num() const { return Checkpoints.Num(); }

    const TArray<uint8>& GetOldest() const
    {
        return Checkpoints[0];
    }

    const TArray<uint8>& GetNewest() const
    {
        return Checkpoints.Last();
    }

    const TArray<uint8>* GetFromNewestOffset(int32 Offset) const
    {
        if (Offset < 0 || Offset >= Checkpoints.Num())
        {
            return nullptr;
        }
        return &Checkpoints[Checkpoints.Num() - 1 - Offset];
    }
private:
    TArray<TArray<uint8>> Checkpoints;
};
