#include "Vehicle/PinkCabChaosNativeClutchJoint.h"
#include "Chaos/PBDJointConstraints.h"
#include "Physics/ImmediatePhysics/ImmediatePhysicsChaos/ImmediatePhysicsSimulation_Chaos.h"
#include "Physics/ImmediatePhysics/ImmediatePhysicsChaos/ImmediatePhysicsActorHandle_Chaos.h"
#include "Physics/ImmediatePhysics/ImmediatePhysicsChaos/ImmediatePhysicsJointHandle_Chaos.h"

namespace { constexpr double MetreSquaredToCentimetreSquared = 10000.0; }

struct FPinkCabChaosNativeClutchJoint::FImplementation
{
    ImmediatePhysics_Chaos::FSimulation Simulation;
    ImmediatePhysics_Chaos::FActorHandle* Rotors[2] = {};
    ImmediatePhysics_Chaos::FJointHandle* Joint = nullptr;
    Chaos::FPBDJointSettings Settings;
    Chaos::FPBDJointSolverSettings SolverSettings;

    FImplementation()
    {
        using namespace ImmediatePhysics_Chaos;
        for (int32 Index = 0; Index < 2; ++Index)
        {
            FActorSetup Setup;
            Setup.ActorType = EActorType::DynamicActor;
            Setup.Transform = FTransform::Identity;
            Setup.CoMTransform = FTransform::Identity;
            Setup.Mass = 1.0;
            Setup.Inertia = FVector(MetreSquaredToCentimetreSquared);
            Setup.LinearDamping = Setup.AngularDamping = 0.0;
            Setup.bInertiaConditioningEnabled = false;
            Setup.bEnableGravity = false;
            Setup.bGyroscopicTorqueEnabled = false;
            Setup.CollisionEnabled = ECollisionEnabled::NoCollision;
            Rotors[Index] = Simulation.CreateActor(MoveTemp(Setup));
            check(Rotors[Index]);
            Simulation.SetEnabled(Rotors[Index], true);
            Simulation.SetHasCollision(Rotors[Index], false);
            Rotors[Index]->SetMaxAngularVelocitySquared(1.e12);
        }
        Settings.ConnectorTransforms = Chaos::FTransformPair(
            Chaos::FRigidTransform3::Identity, Chaos::FRigidTransform3::Identity);
        Settings.LinearMotionTypes = Chaos::TVector<Chaos::EJointMotionType, 3>(Chaos::EJointMotionType::Free);
        Settings.AngularMotionTypes = Chaos::TVector<Chaos::EJointMotionType, 3>(Chaos::EJointMotionType::Free);
        Settings.bMassConditioningEnabled = false;
        Settings.bProjectionEnabled = false;
        Settings.bShockPropagationEnabled = false;
        Settings.AngularDriveForceMode = Chaos::EJointForceMode::Force;
        Settings.AngularDriveVelocityTarget = Chaos::FVec3(0);
        Settings.AngularDriveStiffness = Chaos::FVec3(0);
        Joint = Simulation.CreateJoint(FJointSetup(Settings, Rotors[0], Rotors[1]));
        check(Joint);
        Simulation.SetRewindVelocities(false);
        // A free-spinning shaft uses angular velocities, not wrapped quaternion
        // positional deltas at high RPM. Native velocity drives avoid that alias.
        SolverSettings.bUsePositionBasedDrives = false;
    }
};

FPinkCabChaosNativeClutchJoint::FPinkCabChaosNativeClutchJoint() = default;
FPinkCabChaosNativeClutchJoint::~FPinkCabChaosNativeClutchJoint() = default;

FPinkCabNativeClutchResult FPinkCabChaosNativeClutchJoint::Solve(
    double EngineOmega, double ShaftOmega, double EngineInertiaKgM2,
    double ShaftInertiaKgM2, double CapacityNm, double SynchronizationSeconds, double DeltaSeconds)
{
    FPinkCabNativeClutchResult Result;
    Result.EngineOmega = EngineOmega;
    const double Values[] = {EngineOmega, ShaftOmega, EngineInertiaKgM2,
        ShaftInertiaKgM2, CapacityNm, SynchronizationSeconds, DeltaSeconds};
    for (const double Value : Values) if (!FMath::IsFinite(Value)) return Result;
    if (EngineInertiaKgM2 <= 0 || ShaftInertiaKgM2 <= 0 || CapacityNm < 0
        || SynchronizationSeconds <= 0 || DeltaSeconds <= 0) return Result;
    if (!Implementation) Implementation = MakeUnique<FImplementation>();
    auto& Native = *Implementation;
    const double Scale = MetreSquaredToCentimetreSquared;
    Native.Rotors[0]->SetInverseInertia(FVector(1.0 / (EngineInertiaKgM2 * Scale)));
    Native.Rotors[1]->SetInverseInertia(FVector(1.0 / (ShaftInertiaKgM2 * Scale)));
    Native.Rotors[0]->SetAngularVelocity(FVector(EngineOmega, 0, 0));
    Native.Rotors[1]->SetAngularVelocity(FVector(ShaftOmega, 0, 0));
    Native.Settings.bAngularTwistVelocityDriveEnabled = CapacityNm > 0;
    Native.Settings.AngularDriveMaxTorque = Chaos::FVec3(CapacityNm * Scale);
    Native.Settings.AngularDriveDamping = Chaos::FVec3(EngineInertiaKgM2 * Scale / SynchronizationSeconds);
    Native.Joint->GetConstraint()->SetSettings(Native.Settings);
    Native.Simulation.SetSolverSettings(DeltaSeconds, -1, -1, 1, 1, 1, 0, 0);
    Native.Simulation.Simulate(DeltaSeconds, DeltaSeconds, 1, FVector::ZeroVector, &Native.SolverSettings);
    const double After0 = Native.Rotors[0]->GetAngularVelocity().X;
    const double After1 = Native.Rotors[1]->GetAngularVelocity().X;
    Result.EngineOmega = After0;
    Result.TransferredTorqueNm = ShaftInertiaKgM2 * (After1 - ShaftOmega) / DeltaSeconds;
    Result.MomentumResidual = EngineInertiaKgM2 * (After0 - EngineOmega)
        + ShaftInertiaKgM2 * (After1 - ShaftOmega);
    Result.DissipatedEnergyJ = 0.5 * (EngineInertiaKgM2 * (EngineOmega * EngineOmega - After0 * After0)
        + ShaftInertiaKgM2 * (ShaftOmega * ShaftOmega - After1 * After1));
    Result.bValid = FMath::IsFinite(After0) && FMath::IsFinite(Result.TransferredTorqueNm)
        && FMath::IsFinite(Result.DissipatedEnergyJ);
    return Result;
}
