#pragma once

#include "CoreMinimal.h"
#include "Core/PinkCabStableId.h"

struct PINKCABTAXI_API FPinkCabNeuralContactState
{
    FPinkCabNeuralContactState();
    explicit FPinkCabNeuralContactState(
        const FPinkCabStableId& InIdentityId,
        int32 InMaxMessages = 64);

    void SetPermission(bool bGranted);
    bool HasPermission() const;
    void SetBlocked(bool bInBlocked);
    bool IsBlocked() const;
    bool AddSimpleMessage(const FString& Message);
    const TArray<FString>& GetMessages() const;

    FPinkCabStableId IdentityId;

private:
    int32 MaxMessages = 64;
    bool bPermissionGranted = false;
    bool bBlocked = false;
    TArray<FString> Messages;
};
