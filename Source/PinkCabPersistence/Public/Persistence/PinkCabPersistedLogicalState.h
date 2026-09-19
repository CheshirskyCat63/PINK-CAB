#pragma once

#include "CoreMinimal.h"

struct FPinkCabPersistedVehicleHealth
{
    TArray<float> ChannelHealth;
    uint32 FunctionalDamageSerial = 0;
};

struct FPinkCabPersistedPassengerIdentity
{
    FString IdentityId;
    FString TemplateId;
    uint64 IdentitySeed = 0;
    uint64 AppearanceSeed = 0;
    FString AppearanceProfileId;
    int32 PaidFareCount = 0;
    int32 AuthoredEventCount = 0;
    bool bRepeatEligible = false;
    bool bNeuralPermissionGranted = false;
    bool bNeuralBlocked = false;
    float Trust = 0.0f;
    float Satisfaction = 0.0f;
    float RiskTolerance = 0.0f;
    TArray<int32> ReviewStars;
    TArray<FString> ReviewTexts;
};

struct FPinkCabPersistedEconomyState
{
    int64 BalanceMinor = 0;
    int64 DebtLimitMinor = 0;
    TArray<FString> CommittedTransactionIds;
};

struct FPinkCabLogicalSaveState
{
    FPinkCabPersistedVehicleHealth VehicleHealth;
    FPinkCabPersistedPassengerIdentity Passenger;
    FPinkCabPersistedEconomyState Economy;
};
