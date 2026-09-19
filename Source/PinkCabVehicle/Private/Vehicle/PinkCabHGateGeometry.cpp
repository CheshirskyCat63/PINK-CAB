#include "Vehicle/PinkCabHGateGeometry.h"

int32 FPinkCabHGateGeometry::GearColumn(const int32 Gear)
{
    if (Gear == 1 || Gear == 2) return 0;
    if (Gear == 5 || Gear == -1) return 2;
    return 1;
}

bool FPinkCabHGateGeometry::MoveGate(FPinkCabHGateState& State, const float X, const float Y)
{
    const int32 NewRow = Y > 0.35f ? 1 : (Y < -0.35f ? -1 : 0);
    const int32 Column = X < -0.33f ? 0 : (X > 0.33f ? 2 : 1);
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
    constexpr float CountsX = 160.0f;
    constexpr float CountsY = 140.0f;
    const float GateDx = DriverRightCounts / CountsX;
    const float GateDy = DriverForwardCounts / CountsY;
    const int32 Steps = FMath::Max(
        1,
        FMath::CeilToInt(FMath::Max(FMath::Abs(GateDx), FMath::Abs(GateDy)) / 0.20f));
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
