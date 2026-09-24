#include "Vehicle/PinkCabHGateGeometry.h"

namespace
{
constexpr float HGateColumnEngage = 0.50f;
constexpr float HGateCenterColumnCapture = 0.25f;
constexpr float HGateRowEngage = 0.65f;
constexpr float HGateCenterRowEngage = 0.78f;
constexpr float HGateRowRelease = 0.35f;
constexpr float HGateCountsX = 160.0f;
constexpr float HGateCountsY = 140.0f;
constexpr float HGateMaxSubstep = 0.20f;

int32 ResolveColumn(const float X)
{
    return X < -HGateColumnEngage ? 0 : (X > HGateColumnEngage ? 2 : 1);
}

int32 ResolveRow(
    const FPinkCabHGateState& State,
    const float Y,
    const int32 Column)
{
    if (State.LastGateRow > 0)
    {
        if (Y < -HGateRowEngage) return -1;
        return Y > HGateRowRelease ? 1 : 0;
    }
    if (State.LastGateRow < 0)
    {
        if (Y > HGateRowEngage) return 1;
        return Y < -HGateRowRelease ? -1 : 0;
    }

    const float EngageThreshold =
        Column == 1 ? HGateCenterRowEngage : HGateRowEngage;
    return Y > EngageThreshold
        ? 1
        : (Y < -EngageThreshold ? -1 : 0);
}
}

int32 FPinkCabHGateGeometry::GearColumn(const int32 Gear)
{
    if (Gear == 1 || Gear == 2) return 0;
    if (Gear == 5 || Gear == -1) return 2;
    return 1;
}

bool FPinkCabHGateGeometry::MoveGate(FPinkCabHGateState& State, const float X, const float Y)
{
    const int32 Column = ResolveColumn(X);
    const int32 NewRow = ResolveRow(State, Y, Column);
    if (State.LastGateRow != 0 && NewRow != 0
        && (NewRow != State.LastGateRow || Column != State.LastGateColumn))
    {
        return false;
    }

    // The centre 3/4 rail is intentionally narrower than the broad neutral
    // cross-gate. This prevents small vertical mouse drift while traversing N
    // from accidentally selecting 3rd/4th, without changing the already
    // accepted 1/2 and 5/R outer rails.
    if (State.LastGateRow == 0
        && NewRow != 0
        && Column == 1
        && FMath::Abs(X) > HGateCenterColumnCapture)
    {
        return false;
    }

    if (NewRow == 0)
    {
        State.RequestedGear = 0;
        State.LastGateRow = 0;
        State.LastGateColumn = Column;
        return true;
    }

    State.RequestedGear = NewRow > 0
        ? (Column == 0 ? 1 : (Column == 1 ? 3 : 5))
        : (Column == 0 ? 2 : (Column == 1 ? 4 : -1));
    State.LastGateRow = NewRow;
    State.LastGateColumn = Column;
    return true;
}

bool FPinkCabHGateGeometry::ApplyDriverDelta(
    FPinkCabHGateState& State,
    const float DriverRightCounts,
    const float DriverForwardCounts)
{
    const float GateDx = DriverRightCounts / HGateCountsX;
    const float GateDy = DriverForwardCounts / HGateCountsY;
    const int32 Steps = FMath::Max(
        1,
        FMath::CeilToInt(
            FMath::Max(FMath::Abs(GateDx), FMath::Abs(GateDy)) / HGateMaxSubstep));
    const float StepX = GateDx / static_cast<float>(Steps);
    const float StepY = GateDy / static_cast<float>(Steps);
    const int32 Before = State.RequestedGear;

    for (int32 Index = 0; Index < Steps; ++Index)
    {
        if (State.LastGateRow != 0)
        {
            const float CandidateY = FMath::Clamp(State.LeverY + StepY, -1.0f, 1.0f);
            if (MoveGate(State, State.LeverX, CandidateY))
            {
                State.LeverY = CandidateY;
            }
            if (State.LastGateRow != 0)
            {
                continue;
            }
        }

        const float CandidateX = FMath::Clamp(State.LeverX + StepX, -1.0f, 1.0f);
        if (MoveGate(State, CandidateX, State.LeverY))
        {
            State.LeverX = CandidateX;
        }

        const float CandidateY = FMath::Clamp(State.LeverY + StepY, -1.0f, 1.0f);
        if (MoveGate(State, State.LeverX, CandidateY))
        {
            State.LeverY = CandidateY;
        }
    }
    return State.RequestedGear != Before;
}

void FPinkCabHGateGeometry::ResetToGear(FPinkCabHGateState& State, const int32 Gear)
{
    State.RequestedGear = FMath::Clamp(Gear, -1, 5);
    State.LastGateRow = State.RequestedGear == 0 ? 0
        : (State.RequestedGear == 1 || State.RequestedGear == 3 || State.RequestedGear == 5 ? 1 : -1);
    State.LastGateColumn = GearColumn(State.RequestedGear);
    State.LeverX = State.LastGateColumn == 0 ? -1.0f : (State.LastGateColumn == 2 ? 1.0f : 0.0f);
    State.LeverY = State.LastGateRow > 0 ? 1.0f : (State.LastGateRow < 0 ? -1.0f : 0.0f);
}
