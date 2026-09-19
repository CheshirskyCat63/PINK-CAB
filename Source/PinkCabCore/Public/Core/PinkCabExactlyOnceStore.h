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
    TPinkCabReplayOutcome<TResult> Execute(const FPinkCabStableId& OperationId, TCallable&& Callable)
    {
        if (!OperationId.IsValid())
        {
            return {EPinkCabReplayDisposition::InvalidId, TResult{}};
        }
        const FString Key = OperationId.Serialize();
        if (const TResult* Existing = Results.Find(Key))
        {
            return {EPinkCabReplayDisposition::Replayed, *Existing};
        }

        TResult Result = Forward<TCallable>(Callable)();
        Results.Add(Key, Result);
        return {EPinkCabReplayDisposition::Executed, MoveTemp(Result)};
    }

    void Clear()
    {
        Results.Reset();
    }

    int32 Num() const
    {
        return Results.Num();
    }

private:
    TMap<FString, TResult> Results;
};
