#include "Vehicle/PinkCabHGateGeometry.h"

namespace
{
constexpr float HGateRowEngage = 0.65f;
constexpr float HGateRowRelease = 0.35f;
constexpr float HGateCountsX = 320.0f;
constexpr float HGateCountsY = 480.0f;
constexpr float HGateMaxSubstep = 0.20f;

// Forgiving owner-approved H-pattern:
//   1/2 owns only the extreme-left region.
//   5/R owns only the extreme-right region.
//   3/4 owns the broad middle region between them, so the driver does not
//   have to pixel-hunt a narrow center rail with the mouse.
constexpr float HGateLeftRailX = -1.0f;
constexpr float HGateMiddleRailX = 1.0f;
constexpr float HGateRightRailX = 2.0f;
constexpr float HGateLeftZoneMaxX = -0.50f;
constexpr float HGateRightZoneMinX = 1.50f;

int32 ResolveColumn(const float X)
{
    if (X <= HGateLeftZoneMaxX) return 0;
    if (X >= HGateRightZoneMinX) return 2;
    return 1;
}

int32 ResolveRow(const FPinkCabHGateState& State, const float Y)
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
    return Y > HGateRowEngage ? 1 : (Y < -HGateRowEngage ? -1 : 0);
}

float ColumnPosition(const int32 Column)
{
    if (Column == 0) return HGateLeftRailX;
    if (Column == 2) return HGateRightRailX;
    return HGateMiddleRailX;
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
    const int32 NewRow = ResolveRow(State, Y);
    const int32 Column = ResolveColumn(X);
    if (State.LastGateRow != 0 && NewRow != 0
        && (NewRow != State.LastGateRow || Column != State.LastGateColumn))
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

    // The entire comfortable middle region resolves to the 3/4 column.
    // Only the two outer edge regions resolve to 1/2 and 5/R.
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
    const int32 Before = State.RequestedGear;

    // Inside a gear rail, fore/aft owns the sample until N is physically
    // reached. Horizontal input in that same sample is discarded.
    if (State.LastGateRow != 0)
    {
        const int32 Steps = FMath::Max(
            1,
            FMath::CeilToInt(FMath::Abs(GateDy) / HGateMaxSubstep));
        const float StepY = GateDy / static_cast<float>(Steps);

        for (int32 Index = 0; Index < Steps; ++Index)
        {
            const float CandidateY =
                FMath::Clamp(State.LeverY + StepY, -1.0f, 1.0f);
            if (MoveGate(State, State.LeverX, CandidateY))
            {
                State.LeverY = CandidateY;
            }

            if (State.LastGateRow == 0)
            {
                break;
            }
        }
        return State.RequestedGear != Before;
    }

    // Across neutral, exactly one axis is accepted per sample. Horizontal wins
    // ties, preventing a diagonal cut through an H-gate wall.
    const bool bHorizontalPhase =
        FMath::Abs(GateDx) >= FMath::Abs(GateDy);

    if (bHorizontalPhase)
    {
        const int32 Steps = FMath::Max(
            1,
            FMath::CeilToInt(FMath::Abs(GateDx) / HGateMaxSubstep));
        const float StepX = GateDx / static_cast<float>(Steps);
        for (int32 Index = 0; Index < Steps; ++Index)
        {
            const float CandidateX =
                FMath::Clamp(State.LeverX + StepX, HGateLeftRailX, HGateRightRailX);
            if (MoveGate(State, CandidateX, State.LeverY))
            {
                State.LeverX = CandidateX;
            }
        }
    }
    else
    {
        // At neutral every horizontal position belongs to a deliberate column
        // region, so fore/aft motion is always available without precision aiming.
        const int32 Steps = FMath::Max(
            1,
            FMath::CeilToInt(FMath::Abs(GateDy) / HGateMaxSubstep));
        const float StepY = GateDy / static_cast<float>(Steps);
        for (int32 Index = 0; Index < Steps; ++Index)
        {
            const float CandidateY =
                FMath::Clamp(State.LeverY + StepY, -1.0f, 1.0f);
            if (MoveGate(State, State.LeverX, CandidateY))
            {
                State.LeverY = CandidateY;
            }

            if (State.LastGateRow != 0)
            {
                break;
            }
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
    State.LeverX = ColumnPosition(State.LastGateColumn);
    State.LeverY = State.LastGateRow > 0 ? 1.0f : (State.LastGateRow < 0 ? -1.0f : 0.0f);
}
