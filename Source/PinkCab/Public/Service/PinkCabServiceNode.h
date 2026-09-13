#pragma once

#include "CoreMinimal.h"
#include "Service/PinkCabServiceContext.h"

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

    bool BindContext(
        const FPinkCabCityIdentity& City,
        const FString& SemanticKey,
        const FPinkCabServiceOwnerIdentity& Owners)
    {
        const FString CleanKey = SemanticKey.TrimStartAndEnd();
        const FString NodeId = FPinkCabServiceContext::MakeNodeId(
            City, static_cast<uint8>(Kind), CleanKey);
        if (NodeId.IsEmpty() || !Owners.IsValid()) return false;
        if (Context.IsValid())
        {
            return Context.ServiceNodeId == NodeId && Context.Owners.IsSameAs(Owners);
        }
        Context.City = City;
        Context.SemanticKey = CleanKey;
        Context.ServiceNodeId = NodeId;
        Context.Owners = Owners;
        return true;
    }

    const FPinkCabServiceContext& GetContext() const { return Context; }
    const FString& GetServiceNodeId() const { return Context.ServiceNodeId; }
    bool HasSameOwners(const FPinkCabServiceOwnerIdentity& Owners) const
    {
        return Context.IsValid() && Context.Owners.IsSameAs(Owners);
    }

    bool MarkEligible() { return Transition(EPinkCabServiceNodeState::Approach, EPinkCabServiceNodeState::Eligible); }
    bool Enter() { return Transition(EPinkCabServiceNodeState::Eligible, EPinkCabServiceNodeState::Enter); }
    bool Activate() { return Transition(EPinkCabServiceNodeState::Enter, EPinkCabServiceNodeState::NodeActive); }
    bool BeginCommit() { return Transition(EPinkCabServiceNodeState::NodeActive, EPinkCabServiceNodeState::Commit); }
    bool Exit() { return Transition(EPinkCabServiceNodeState::Commit, EPinkCabServiceNodeState::Exit); }
    bool ResumeWorld() { return Transition(EPinkCabServiceNodeState::Exit, EPinkCabServiceNodeState::ResumeWorld); }

    void Abort()
    {
        State = EPinkCabServiceNodeState::Approach;
    }

    void ResetContext()
    {
        State = EPinkCabServiceNodeState::Approach;
        Context = FPinkCabServiceContext{};
    }

private:
    bool Transition(EPinkCabServiceNodeState Expected, EPinkCabServiceNodeState Next)
    {
        if (State != Expected) return false;
        State = Next;
        return true;
    }

    EPinkCabServiceNodeKind Kind;
    EPinkCabServiceNodeState State = EPinkCabServiceNodeState::Approach;
    FPinkCabServiceContext Context;
};
