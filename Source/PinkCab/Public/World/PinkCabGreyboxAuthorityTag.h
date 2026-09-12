#pragma once

#include "CoreMinimal.h"

struct FPinkCabGreyboxAuthorityTag
{
    static FString Marker()
    {
        return TEXT("NON_AUTHORITATIVE_GEOMETRY");
    }

    static constexpr bool IsAuthoritativeGeometry()
    {
        return false;
    }
};
