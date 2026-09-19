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

class PINKCAB_API FPinkCabServiceNode
{
public:
    explicit FPinkCabServiceNode(
        EPinkCabServiceNodeKind InKind);

    EPinkCabServiceNodeState GetState() const;
    EPinkCabServiceNodeKind GetKind() const;

    bool BindContext(
        const FPinkCabCityIdentity& City,
        const FString& SemanticKey,
        const FPinkCabServiceOwnerIdentity& Owners);
    const FPinkCabServiceContext& GetContext() const;
    const FString& GetServiceNodeId() const;
    bool HasSameOwners(
        const FPinkCabServiceOwnerIdentity& Owners) const;

    bool MarkEligible();
    bool Enter();
    bool Activate();
    bool BeginCommit();
    bool Exit();
    bool ResumeWorld();
    void Abort();
    void ResetContext();

private:
    bool Transition(
        EPinkCabServiceNodeState Expected,
        EPinkCabServiceNodeState Next);

    EPinkCabServiceNodeKind Kind;
    EPinkCabServiceNodeState State =
        EPinkCabServiceNodeState::Approach;
    FPinkCabServiceContext Context;
};
