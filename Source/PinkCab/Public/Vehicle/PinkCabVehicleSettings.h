#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "UObject/SoftObjectPath.h"
#include "PinkCabVehicleSettings.generated.h"

class UPinkCabVehicleDefinition;

UCLASS(Config=Game, DefaultConfig)
class PINKCAB_API UPinkCabVehicleSettings : public UObject
{
    GENERATED_BODY()

public:
    UPROPERTY(Config, EditAnywhere, Category="PinkCab|Vehicle")
    TSoftObjectPtr<UPinkCabVehicleDefinition> DefaultVehicleDefinition;

    static FSoftObjectPath ResolveDefinitionPath(
        const FString& CommandLine,
        const FSoftObjectPath& DefaultPath);

    const UPinkCabVehicleDefinition* LoadSelectedDefinition(
        const FString& CommandLine) const;
};
