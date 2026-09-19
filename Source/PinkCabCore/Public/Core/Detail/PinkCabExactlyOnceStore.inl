#pragma once

template <typename TResult>
template <typename TCallable>
TPinkCabReplayOutcome<TResult> TPinkCabExactlyOnceStore<TResult>::Execute(
    const FPinkCabStableId& OperationId,
    TCallable&& Callable)
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
    return {
        EPinkCabReplayDisposition::Executed,
        MoveTemp(Result)};
}

template <typename TResult>
void TPinkCabExactlyOnceStore<TResult>::Clear()
{
    Results.Reset();
}
template <typename TResult>
int32 TPinkCabExactlyOnceStore<TResult>::Num() const
{
    return Results.Num();
}
