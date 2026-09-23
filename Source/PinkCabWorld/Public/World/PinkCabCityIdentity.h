#pragma once

#include "CoreMinimal.h"

namespace PinkCabWorldId
{
    PINKCABWORLD_API uint64 StableFnv1a64(const FString& Text);
    PINKCABWORLD_API FString StableToken(const FString& Prefix, const FString& Payload);
}

struct PINKCABWORLD_API FPinkCabCityIdentity
{
    static FPinkCabCityIdentity Create(
        const FString& InCityCode,
        const FString& InGeneratorVersion,
        const FString& InContentSetVersion);

    bool IsValid() const;
    FString GetStableKey() const;
    bool IsSameCampaignCity(const FPinkCabCityIdentity& Other) const;
    const FString& GetCityCode() const;
    const FString& GetGeneratorVersion() const;
    const FString& GetContentSetVersion() const;

private:
    FString CityCode;
    FString GeneratorVersion;
    FString ContentSetVersion;
};
