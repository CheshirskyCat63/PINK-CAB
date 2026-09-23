#include "Service/PinkCabServiceNode.h"

FPinkCabServiceNode::FPinkCabServiceNode(
    EPinkCabServiceNodeKind InKind)
    : Kind(InKind)
{
}

EPinkCabServiceNodeState FPinkCabServiceNode::GetState() const
{
    return State;
}

EPinkCabServiceNodeKind FPinkCabServiceNode::GetKind() const
{
    return Kind;
}

bool FPinkCabServiceNode::BindContext(
    const FPinkCabCityIdentity& City,
    const FString& SemanticKey,
    const FPinkCabServiceOwnerIdentity& Owners)
{
    const FString CleanKey = SemanticKey.TrimStartAndEnd();
    const FString NodeId = FPinkCabServiceContext::MakeNodeId(
        City,
        static_cast<uint8>(Kind),
        CleanKey);
    if (NodeId.IsEmpty() || !Owners.IsValid())
    {
        return false;
    }

    if (Context.IsValid())
    {
        return Context.ServiceNodeId == NodeId
            && Context.Owners.IsSameAs(Owners);
    }

    Context.City = City;
    Context.SemanticKey = CleanKey;
    Context.ServiceNodeId = NodeId;
    Context.Owners = Owners;
    return true;
}

const FPinkCabServiceContext&
FPinkCabServiceNode::GetContext() const
{
    return Context;
}

const FString& FPinkCabServiceNode::GetServiceNodeId() const
{
    return Context.ServiceNodeId;
}

bool FPinkCabServiceNode::HasSameOwners(
    const FPinkCabServiceOwnerIdentity& Owners) const
{
    return Context.IsValid()
        && Context.Owners.IsSameAs(Owners);
}

bool FPinkCabServiceNode::MarkEligible()
{
    return Transition(
        EPinkCabServiceNodeState::Approach,
        EPinkCabServiceNodeState::Eligible);
}

bool FPinkCabServiceNode::Enter()
{
    return Transition(
        EPinkCabServiceNodeState::Eligible,
        EPinkCabServiceNodeState::Enter);
}

bool FPinkCabServiceNode::Activate()
{
    return Transition(
        EPinkCabServiceNodeState::Enter,
        EPinkCabServiceNodeState::NodeActive);
}

bool FPinkCabServiceNode::BeginCommit()
{
    return Transition(
        EPinkCabServiceNodeState::NodeActive,
        EPinkCabServiceNodeState::Commit);
}

bool FPinkCabServiceNode::Exit()
{
    return Transition(
        EPinkCabServiceNodeState::Commit,
        EPinkCabServiceNodeState::Exit);
}

bool FPinkCabServiceNode::ResumeWorld()
{
    return Transition(
        EPinkCabServiceNodeState::Exit,
        EPinkCabServiceNodeState::ResumeWorld);
}

void FPinkCabServiceNode::Abort()
{
    State = EPinkCabServiceNodeState::Approach;
}

void FPinkCabServiceNode::ResetContext()
{
    State = EPinkCabServiceNodeState::Approach;
    Context = FPinkCabServiceContext{};
}

bool FPinkCabServiceNode::Transition(
    EPinkCabServiceNodeState Expected,
    EPinkCabServiceNodeState Next)
{
    if (State != Expected)
    {
        return false;
    }
    State = Next;
    return true;
}
