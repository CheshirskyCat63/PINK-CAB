#pragma once

#include "CoreMinimal.h"
#include "Persistence/PinkCabSaveHeader.h"

class PINKCABPERSISTENCE_API FPinkCabCheckpointRing
{
public:
    void Push(const TArray<uint8>& Bytes);
    int32 Num() const;
    const TArray<uint8>& GetOldest() const;
    const TArray<uint8>& GetNewest() const;
    const TArray<uint8>* GetFromNewestOffset(int32 Offset) const;

private:
    TArray<TArray<uint8>> Checkpoints;
};
