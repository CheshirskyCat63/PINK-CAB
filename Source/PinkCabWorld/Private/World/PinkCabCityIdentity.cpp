#include "World/PinkCabCityIdentity.h"

uint64 PinkCabWorldId::StableFnv1a64(const FString& Text)
{
    FTCHARToUTF8 Utf8(*Text);
    const uint8* Data = reinterpret_cast<const uint8*>(Utf8.Get());
    uint64 Hash = 14695981039346656037ull;
    for (int32 Index = 0; Index < Utf8.Length(); ++Index)
    {
        Hash ^= Data[Index];
        Hash *= 1099511628211ull;
    }
    return Hash;
}

FString PinkCabWorldId::StableToken(const FString& Prefix, const FString& Payload)
{
    return FString::Printf(
        TEXT("%s%016llx"),
        *Prefix,
        static_cast<unsigned long long>(StableFnv1a64(Payload)));
}

FPinkCabCityIdentity FPinkCabCityIdentity::Create(
    const FString& InCityCode,
    const FString& InGeneratorVersion,
    const FString& InContentSetVersion)
{
    FPinkCabCityIdentity Result;
    Result.CityCode = InCityCode.TrimStartAndEnd();
    Result.GeneratorVersion = InGeneratorVersion.TrimStartAndEnd();
    Result.ContentSetVersion = InContentSetVersion.TrimStartAndEnd();
    return Result;
}

bool FPinkCabCityIdentity::IsValid() const
{
    return !CityCode.IsEmpty()
        && !GeneratorVersion.IsEmpty()
        && !ContentSetVersion.IsEmpty();
}

FString FPinkCabCityIdentity::GetStableKey() const
{
    return PinkCabWorldId::StableToken(
        TEXT("city:"),
        CityCode + TEXT("|") + GeneratorVersion + TEXT("|") + ContentSetVersion);
}

bool FPinkCabCityIdentity::IsSameCampaignCity(const FPinkCabCityIdentity& Other) const
{
    return CityCode == Other.CityCode;
}

const FString& FPinkCabCityIdentity::GetCityCode() const { return CityCode; }
const FString& FPinkCabCityIdentity::GetGeneratorVersion() const { return GeneratorVersion; }
const FString& FPinkCabCityIdentity::GetContentSetVersion() const { return ContentSetVersion; }
