#include "Vehicle/PinkCabChaosVehicleMovementComponent.h"

#include "PinkCabChaosVehicleSimulation.h"
#include "Physics/PhysicsInterfaceCore.h"

UPinkCabChaosVehicleMovementComponent::UPinkCabChaosVehicleMovementComponent(
    const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
}

void UPinkCabChaosVehicleMovementComponent::ConfigurePinkCabClutch(
    const FPinkCabClutchDrivelineConfig& InConfig)
{
    ClutchConfig = InConfig;
    PendingDrivelineCommand.ClutchConfig = ClutchConfig;

    FBodyInstance* Body = GetBodyInstance();
    if (!Body || !PinkCabSimulationPT)
    {
        return;
    }

    FPhysicsCommand::ExecuteWrite(
        Body->ActorHandle,
        [this](const FPhysicsActorHandle&)
        {
            if (PinkCabSimulationPT)
            {
                PinkCabSimulationPT->SetDrivelineCommand(PendingDrivelineCommand);
            }
        });
}

bool UPinkCabChaosVehicleMovementComponent::BeginPinkCabMechanicalEvidenceWindow(
    const float SettleSeconds,
    const float SampleSeconds)
{
    FBodyInstance* Body = GetBodyInstance();
    if (!Body || !PinkCabSimulationPT)
    {
        return false;
    }

    return FPhysicsCommand::ExecuteWrite(
        Body->ActorHandle,
        [this, SettleSeconds, SampleSeconds](const FPhysicsActorHandle&)
        {
            if (PinkCabSimulationPT)
            {
                PinkCabSimulationPT->BeginEvidenceWindow(
                    SettleSeconds,
                    SampleSeconds);
            }
        });
}

bool UPinkCabChaosVehicleMovementComponent::ReadPinkCabMechanicalEvidenceWindow(
    FPinkCabMechanicalEvidenceSnapshot& OutSnapshot)
{
    FBodyInstance* Body = GetBodyInstance();
    if (!Body || !PinkCabSimulationPT)
    {
        return false;
    }

    return FPhysicsCommand::ExecuteWrite(
        Body->ActorHandle,
        [this, &OutSnapshot](const FPhysicsActorHandle&)
        {
            if (PinkCabSimulationPT)
            {
                OutSnapshot =
                    PinkCabSimulationPT->ReadEvidenceWindow();
            }
        });
}

TUniquePtr<Chaos::FSimpleWheeledVehicle>
UPinkCabChaosVehicleMovementComponent::CreatePhysicsVehicle()
{
    TUniquePtr<FPinkCabChaosWheeledVehicleSimulation> Simulation =
        MakeUnique<FPinkCabChaosWheeledVehicleSimulation>(
            &MechanicalIntegrationStepCounter,
            &MechanicalIntegrationDeltaMicros);
    PinkCabSimulationPT = Simulation.Get();
    PinkCabSimulationPT->SetDrivelineCommand(PendingDrivelineCommand);
    VehicleSimulationPT = MoveTemp(Simulation);

    return UChaosVehicleMovementComponent::CreatePhysicsVehicle();
}

bool UPinkCabChaosVehicleMovementComponent::SetPinkCabDrivelineCommand(
    const FPinkCabChaosDrivelineCommand& InCommand)
{
    PendingDrivelineCommand = InCommand;
    PendingDrivelineCommand.ClutchConfig = ClutchConfig;
    const FPinkCabChaosDrivelineCommand Command =
        PendingDrivelineCommand;

    FBodyInstance* Body = GetBodyInstance();
    if (!Body || !PinkCabSimulationPT)
    {
        // The command remains authoritative and will be installed when the
        // physics representation is created. Pre-physics calls are valid.
        return true;
    }

    return FPhysicsCommand::ExecuteWrite(
        Body->ActorHandle,
        [this, Command](const FPhysicsActorHandle&)
        {
            if (PinkCabSimulationPT)
            {
                PinkCabSimulationPT->SetDrivelineCommand(Command);
            }
        });
}


#if WITH_DEV_AUTOMATION_TESTS
bool UPinkCabChaosVehicleMovementComponent::SetPinkCabDynamometerBrakeTorqueForTests(
    const float BrakeTorqueNm)
{
    FBodyInstance* Body = GetBodyInstance();
    if (!Body || !PinkCabSimulationPT)
    {
        return false;
    }

    return FPhysicsCommand::ExecuteWrite(
        Body->ActorHandle,
        [this, BrakeTorqueNm](const FPhysicsActorHandle&)
        {
            if (PinkCabSimulationPT)
            {
                PinkCabSimulationPT->SetDynamometerBrakeTorqueForTests(
                    FMath::Max(BrakeTorqueNm, 0.0f));
            }
        });
}
#endif
