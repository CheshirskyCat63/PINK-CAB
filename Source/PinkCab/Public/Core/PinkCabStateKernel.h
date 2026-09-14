#pragma once

#include "CoreMinimal.h"

class FPinkCabStateKernel
{
public:
    explicit FPinkCabStateKernel(uint64 InRootSeed)
        : RootSeed(InRootSeed)
    {
    }

    uint64 GetRootSeed() const
    {
        return RootSeed;
    }

    uint64 NextSequence()
    {
        return ++Sequence;
    }

    void ResetRuntime()
    {
        Sequence = 0;
    }

    void ResetWorkday(uint64 InRootSeed)
    {
        RootSeed = InRootSeed;
        Sequence = 0;
    }

private:
    friend class FPinkCabWorldSessionSnapshotCodec;

    uint64 RootSeed = 0;
    uint64 Sequence = 0;
};
