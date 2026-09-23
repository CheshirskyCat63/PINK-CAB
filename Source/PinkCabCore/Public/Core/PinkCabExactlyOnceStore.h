#pragma once

#include "CoreMinimal.h"
#include "Core/PinkCabStableId.h"

enum class EPinkCabReplayDisposition : uint8
{
    Executed,
    Replayed,
    InvalidId
};

template <typename TResult>
struct TPinkCabReplayOutcome
{
    EPinkCabReplayDisposition Disposition = EPinkCabReplayDisposition::InvalidId;
    TResult Result{};
};

template <typename TResult>
class TPinkCabExactlyOnceStore
{
public:
    template <typename TCallable>
    TPinkCabReplayOutcome<TResult> Execute(
        const FPinkCabStableId& OperationId,
        TCallable&& Callable);

    void Clear();
    int32 Num() const;

private:
    TMap<FString, TResult> Results;
};

#include "Core/Detail/PinkCabExactlyOnceStore.inl"
