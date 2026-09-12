#pragma once

#include "CoreMinimal.h"
#include "PinkCabBuildIdentity.generated.h"

USTRUCT(BlueprintType)
struct PINKCAB_API FPinkCabBuildIdentity
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="PINK CAB|Build")
    FString ProductName = TEXT("PINK CAB");

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="PINK CAB|Build")
    FString SchemaVersion = TEXT("bootstrap-v1");

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="PINK CAB|Build")
    FString BuildChannel = TEXT("core-gameplay-native-chaos");

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="PINK CAB|Build")
    FString RuntimeVersion = TEXT("0.1.0-core-native-chaos");
};
