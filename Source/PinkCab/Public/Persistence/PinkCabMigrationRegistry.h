#pragma once

#include "CoreMinimal.h"

class FPinkCabMigrationRegistry
{
public:
    void RegisterBoundary(const FString& FromSchema, const FString& ToSchema)
    {
        Boundaries.Add(MakeKey(FromSchema, ToSchema));
    }

    bool HasBoundary(const FString& FromSchema, const FString& ToSchema) const
    {
        return Boundaries.Contains(MakeKey(FromSchema, ToSchema));
    }

private:
    static FString MakeKey(const FString& FromSchema, const FString& ToSchema)
    {
        return FromSchema + TEXT("->") + ToSchema;
    }

    TSet<FString> Boundaries;
};
