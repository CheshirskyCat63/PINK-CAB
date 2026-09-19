#include "Core/PinkCabSchemaVersion.h"

FPinkCabSchemaVersion::FPinkCabSchemaVersion(
    int32 InMajor,
    int32 InMinor,
    int32 InPatch)
    : Major(InMajor),
      Minor(InMinor),
      Patch(InPatch)
{
}

bool FPinkCabSchemaVersion::IsValid() const
{
    return Major >= 0 && Minor >= 0 && Patch >= 0;
}

FString FPinkCabSchemaVersion::Serialize() const
{
    return FString::Printf(TEXT("%d.%d.%d"), Major, Minor, Patch);
}

bool FPinkCabSchemaVersion::operator==(
    const FPinkCabSchemaVersion& Other) const
{
    return Major == Other.Major
        && Minor == Other.Minor
        && Patch == Other.Patch;
}
bool FPinkCabSchemaVersion::operator<(
    const FPinkCabSchemaVersion& Other) const
{
    if (Major != Other.Major)
    {
        return Major < Other.Major;
    }
    if (Minor != Other.Minor)
    {
        return Minor < Other.Minor;
    }
    return Patch < Other.Patch;
}

bool FPinkCabSchemaVersion::TryParse(
    const FString& Text,
    FPinkCabSchemaVersion& OutVersion)
{
    TArray<FString> Parts;
    Text.ParseIntoArray(Parts, TEXT("."), false);
    if (Parts.Num() != 3)
    {
        return false;
    }

    int32 Parsed[3] = {0, 0, 0};
    for (int32 Index = 0; Index < 3; ++Index)
    {
        if (Parts[Index].IsEmpty())
        {
            return false;
        }
        for (const TCHAR Character : Parts[Index])
        {
            if (!FChar::IsDigit(Character))
            {
                return false;
            }
        }
        Parsed[Index] = FCString::Atoi(*Parts[Index]);
    }

    OutVersion = FPinkCabSchemaVersion(
        Parsed[0],
        Parsed[1],
        Parsed[2]);
    return true;
}
