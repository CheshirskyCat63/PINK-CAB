#pragma once

#include "CoreMinimal.h"

enum class EPinkCabErrorCode : uint8
{
    None = 0,
    InvalidInput,
    IncompatibleSchema,
    NotFound,
    Conflict,
    Unavailable,
    InternalError
};

struct FPinkCabResult
{
    static FPinkCabResult Success()
    {
        return FPinkCabResult(true, EPinkCabErrorCode::None, FString());
    }

    static FPinkCabResult Failure(EPinkCabErrorCode InCode, const FString& InMessage)
    {
        return FPinkCabResult(false, InCode, InMessage);
    }

    bool IsSuccess() const { return bSuccess; }
    EPinkCabErrorCode GetErrorCode() const { return ErrorCode; }
    const FString& GetMessage() const { return Message; }

private:
    FPinkCabResult(bool bInSuccess, EPinkCabErrorCode InCode, const FString& InMessage)
        : bSuccess(bInSuccess), ErrorCode(InCode), Message(InMessage)
    {
    }

    bool bSuccess = false;
    EPinkCabErrorCode ErrorCode = EPinkCabErrorCode::InternalError;
    FString Message;
};
