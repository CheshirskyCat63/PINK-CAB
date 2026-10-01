#pragma once

#include "Vehicle/PinkCabClutchDrivelineModel.h"

namespace PinkCabClutchIntegration
{
struct FResult
{
    float InitialRequestedTorqueNm = 0.0f;
    float AverageTransmittedTorqueNm = 0.0f;
    float EngineReactionDeltaRpm = 0.0f;
    float FinalRequestedTorqueNm = 0.0f;
    float FinalSlipRpm = 0.0f;
    bool bAnyTorqueLimited = false;
};

FResult Integrate(
    const FPinkCabClutchDrivelineConfig& Config,
    const FPinkCabClutchDrivelineInput& Input,
    float TorqueCapacityNm,
    float NetEngineTorqueNm);
}
