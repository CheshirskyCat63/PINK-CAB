#include "Vehicle/PinkCabHGateGeometry.h"

namespace
{
constexpr float HGateColumnEngage = 0.50f;
constexpr float HGateRowEngage = 0.65f;
constexpr float HGateRowRelease = 0.35f;

// Deliberately long physical mouse throws. The old 160/140-count mapping made
// the H pattern too easy to cross accidentally. Horizontal and longitudinal
// travel are now clearly separated and require a larger driver gesture.
constexpr float HGateCountsX = 320.0f;
constexpr float HGateCountsY = 240.0f;
constexpr float HGateMaxSubstep = 0.20f;

int32 ResolveColumn(const float X)
{
    return X < -HGateColumnEngage ? 0 : (X > HGateColumnEngage ? 2 : 1);
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

    // When the lever is in a gear rail, only fore/aft travel is legal until
    // the lever physically reaches neutral. Horizontal mouse movement in the
    // same sample is ignored, preventing diagonal cuts through an H-gate wall.
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
                // Reaching N ends this physical phase. A later mouse sample
                // must perform the left/right cross-gate movement.
                break;
            }
        }
        return State.RequestedGear != Before;
    }

    // In neutral, one mouse sample moves exactly one H-gate axis. Horizontal
    // wins ties so a diagonal cross-gate gesture cannot accidentally fall into
    // 3rd/4th before the driver has deliberately centred the lever.
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
                FMath::Clamp(State.LeverX + StepX, -1.0f, 1.0f);
            if (MoveGate(State, CandidateX, State.LeverY))
            {
                State.LeverX = CandidateX;
            }
        }
    }
    else
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

            if (State.LastGateRow != 0)
            {
                // Once a gear rail is entered, this phase is complete.
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
    State.LeverX = State.LastGateColumn == 0 ? -1.0f : (State.LastGateColumn == 2 ? 1.0f : 0.0f);
    State.LeverY = State.LastGateRow > 0 ? 1.0f : (State.LastGateRow < 0 ? -1.0f : 0.0f);
}
