#pragma once

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"

namespace PinkCabAutomationWriteGuard
{
inline bool IsAssetAuthoringAllowed()
{
    return FParse::Param(
        FCommandLine::Get(),
        TEXT("PinkCabAllowAssetAuthoring"));
}

inline bool SkipUnlessAssetAuthoringAllowed(
    FAutomationTestBase& Test,
    const TCHAR* TestName)
{
    if (IsAssetAuthoringAllowed())
    {
        return false;
    }

    Test.AddInfo(FString::Printf(
        TEXT("PINKCAB_ASSET_AUTHORING_SKIPPED test=%s reason=missing_-PinkCabAllowAssetAuthoring"),
        TestName));
    return true;
}
}

#endif
