#pragma once

#include "Commandlets/Commandlet.h"
#include "PinkCabTatraPhysicsAssetCommandlet.generated.h"

// Explicit editor authoring only; never called by gameplay or the automation suite.
UCLASS()
class UPinkCabTatraPhysicsAssetCommandlet : public UCommandlet
{
    GENERATED_BODY()
public:
    UPinkCabTatraPhysicsAssetCommandlet();
    virtual int32 Main(const FString& Params) override;
};
