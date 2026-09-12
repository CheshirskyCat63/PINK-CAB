#pragma once

#include "CoreMinimal.h"

enum class EPinkCabServiceNodeKind : uint8
{
    Parking,
    GarageTuning,
    Parts,
    RepairService
};

enum class EPinkCabServiceNodeState : uint8
{
    Approach,
    Eligible,
    Enter,
    NodeActive,
    Commit,
    Exit,
    ResumeWorld
};

class FPinkCabServiceNode
{
public:
    explicit FPinkCabServiceNode(EPinkCabServiceNodeKind InKind) : Kind(InKind) {}
    EPinkCabServiceNodeState GetState() const { return State; }
    EPinkCabServiceNodeKind GetKind() const { return Kind; }
    bool MarkEligible() { return Transition(EPinkCabServiceNodeState::Approach, EPinkCabServiceNodeState::Eligible); }
    bool Enter() { return Transition(EPinkCabServiceNodeState::Eligible, EPinkCabServiceNodeState::Enter); }
    bool Activate() { return Transition(EPinkCabServiceNodeState::Enter, EPinkCabServiceNodeState::NodeActive); }
    bool BeginCommit() { return Transition(EPinkCabServiceNodeState::NodeActive, EPinkCabServiceNodeState::Commit); }
    bool Exit() { return Transition(EPinkCabServiceNodeState::Commit, EPinkCabServiceNodeState::Exit); }
    bool ResumeWorld() { return Transition(EPinkCabServiceNodeState::Exit, EPinkCabServiceNodeState::ResumeWorld); }

private:
    bool Transition(EPinkCabServiceNodeState Expected, EPinkCabServiceNodeState Next)
    {
        if (State != Expected) return false;
        State = Next;
        return true;
    }

    EPinkCabServiceNodeKind Kind;
    EPinkCabServiceNodeState State = EPinkCabServiceNodeState::Approach;
};
