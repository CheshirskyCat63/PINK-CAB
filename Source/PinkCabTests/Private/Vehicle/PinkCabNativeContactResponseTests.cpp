#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Vehicle/PinkCabNativeWheelResponse.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPinkCabNativeSurfaceResponseTest,
    "PinkCab.Vehicle.Actuation.NativeWheelCurrentContact",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPinkCabNativeSurfaceResponseTest::RunTest(const FString& Parameters)
{
    for (const bool ToSlippery : {false, true})
    {
        Chaos::FSimpleWheelConfig Config;
        Config.EngineEnabled = true;
        Config.TorqueRatio = 1.0f;
        Config.WheelRadius = 32.0f;
        Chaos::FSimpleWheeledVehicle Vehicle;
        Vehicle.Wheels.Emplace(&Config);
        auto& Original = Vehicle.Wheels[0];
        Original.SetMassPerWheel(414.25f);
        Original.SetMaxOmega(2000.0f);
        Original.SetWheelLoadForce(450000.0f);
        Original.SetSurfaceFriction(ToSlippery ? 1.0f : 0.02f);
        Original.SetAngularVelocity(700.0f / 32.0f);
        FWheelState State;
        State.Init(1);
        State.LocalWheelVelocity[0] = FVector(700,0,0);
        auto* CurrentMaterial = NewObject<UPhysicalMaterial>();
        CurrentMaterial->Friction = ToSlippery ? 0.02f : 1.0f;
        State.TraceResult[0].bBlockingHit = true;
        State.TraceResult[0].PhysMaterial = CurrentMaterial;
        auto Reference = Original;
        Reference.SetSurfaceFriction(CurrentMaterial->Friction);
        Reference.SetVehicleGroundSpeed(State.LocalWheelVelocity[0]);
        Reference.SetDriveTorque(Chaos::TorqueMToCm(200.0f));
        Reference.Simulate(1.0f / 60.0f);
        const auto Candidate = PinkCabNativeWheelBoundary::EvaluateNativeWheels(Vehicle,State,200.0,1.0,1.0,1.0/60.0);
        TestTrue(TEXT("native prediction uses current trace material, not prior surface"),
            Candidate.bValid && FMath::Abs(Candidate.PredictedOmega[0] - Reference.GetAngularVelocity()) < 0.0001);
        TestEqual(TEXT("candidate does not mutate the actual wheel friction"), Original.SurfaceFriction, ToSlippery ? 1.0f : 0.02f);
        AddInfo(FString::Printf(TEXT("T6_CONTACT_RESPONSE slippery=%d actual_expected=%.6f predicted=%.6f"),ToSlippery,Reference.GetAngularVelocity(),Candidate.PredictedOmega[0]));
    }
    // A missing new physical material preserves Chaos's current coefficient.
    // The native friction step does not overwrite it with an arbitrary 1.0.
    {
        Chaos::FSimpleWheelConfig Config;
        Config.EngineEnabled = true;
        Config.TorqueRatio = 1.0f;
        Config.WheelRadius = 32.0f;
        Chaos::FSimpleWheeledVehicle Vehicle;
        Vehicle.Wheels.Emplace(&Config);
        auto& Wheel = Vehicle.Wheels[0];
        Wheel.SetMassPerWheel(414.25f);
        Wheel.SetMaxOmega(2000.0f);
        Wheel.SetWheelLoadForce(450000.0f);
        Wheel.SetSurfaceFriction(0.02f);
        Wheel.SetAngularVelocity(700.0f / 32.0f);
        FWheelState State;
        State.Init(1);
        State.LocalWheelVelocity[0] = FVector(700, 0, 0);
        State.TraceResult[0].bBlockingHit = true;
        auto Reference = Wheel;
        Reference.SetVehicleGroundSpeed(State.LocalWheelVelocity[0]);
        Reference.SetDriveTorque(Chaos::TorqueMToCm(200.0f));
        Reference.Simulate(1.0f / 60.0f);
        const auto Candidate = PinkCabNativeWheelBoundary::EvaluateNativeWheels(
            Vehicle, State, 200.0, 1.0, 1.0, 1.0 / 60.0);
        TestTrue(TEXT("missing current material preserves native prior friction"),
            Candidate.bValid
            && FMath::Abs(Candidate.PredictedOmega[0] - Reference.GetAngularVelocity()) < 0.0001);
        TestEqual(TEXT("missing material does not mutate actual surface"),
            Wheel.SurfaceFriction, 0.02f);
    }
    // Liftoff is a current-frame no-contact condition, regardless of a high
    // normal force still cached on the previous simulation's native wheel.
    {
        Chaos::FSimpleWheelConfig Config;
        Config.EngineEnabled = true;
        Config.TorqueRatio = 1.0f;
        Config.WheelRadius = 32.0f;
        Chaos::FSimpleWheeledVehicle Vehicle;
        Vehicle.Wheels.Emplace(&Config);
        auto& Original = Vehicle.Wheels[0];
        Original.SetMassPerWheel(414.25f);
        Original.SetMaxOmega(2000.0f);
        Original.SetWheelLoadForce(450000.0f);
        Original.SetSurfaceFriction(1.0f);
        Original.SetAngularVelocity(700.0f / 32.0f);
        FWheelState State;
        State.Init(1);
        State.LocalWheelVelocity[0] = FVector(700, 0, 0);
        State.TraceResult[0].bBlockingHit = false;
        auto Reference = Original;
        Reference.SetWheelLoadForce(0.0f);
        Reference.SetSurfaceFriction(1.0f);
        Reference.SetVehicleGroundSpeed(State.LocalWheelVelocity[0]);
        Reference.SetDriveTorque(Chaos::TorqueMToCm(200.0f));
        Reference.Simulate(1.0f / 60.0f);
        const auto Candidate = PinkCabNativeWheelBoundary::EvaluateNativeWheels(
            Vehicle, State, 200.0, 1.0, 1.0, 1.0 / 60.0);
        TestTrue(TEXT("liftoff prediction clears prior contact load"),
            Candidate.bValid
            && FMath::Abs(Candidate.PredictedOmega[0] - Reference.GetAngularVelocity()) < 0.0001);
        TestEqual(TEXT("liftoff candidate leaves actual wheel load untouched"),
            Original.ForceIntoSurface, 450000.0f);
        AddInfo(FString::Printf(TEXT("T6_LIFTOFF_RESPONSE expected=%.6f predicted=%.6f"),
            Reference.GetAngularVelocity(), Candidate.PredictedOmega[0]));
    }
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPinkCabNativeLossDirectionTest,
    "PinkCab.Vehicle.Actuation.NativeGearLossDirection",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPinkCabNativeLossDirectionTest::RunTest(const FString& Parameters)
{
    Chaos::FSimpleWheelConfig Config;
    Config.EngineEnabled = true;
    Config.TorqueRatio = 1.0f;
    Config.WheelRadius = 32.0f;
    Chaos::FSimpleWheeledVehicle Vehicle;
    Vehicle.Wheels.Emplace(&Config);
    auto& Wheel = Vehicle.Wheels[0];
    Wheel.SetMassPerWheel(414.25f);
    Wheel.SetMaxOmega(2000.0f);
    Wheel.SetWheelLoadForce(0.0f);
    Wheel.SetSurfaceFriction(1.0f);
    FWheelState State;
    State.Init(1);
    State.LocalWheelVelocity[0] = FVector(3.2,0,0);
    Wheel.SetAngularVelocity(0.1f);
    const double Torque = -(Wheel.Inertia / 10000.0) * 0.1 * 60.0;
    const auto Candidate = PinkCabNativeWheelBoundary::EvaluateNativeWheels(Vehicle,State,Torque,1.0,0.9,1.0/60.0);
    TestTrue(TEXT("gear direction crossing must produce an actual native response"), Candidate.bValid);
    bool Matches = Candidate.bValid;
    if (Candidate.bValid)
    {
        auto Reference = Wheel;
        Reference.SetVehicleGroundSpeed(State.LocalWheelVelocity[0]);
        Reference.SetDriveTorque(Chaos::TorqueMToCm(static_cast<float>(Torque * Candidate.TorqueFactor)));
        Reference.Simulate(1.0f/60.0f);
        Matches = FMath::Abs(Reference.GetAngularVelocity() - Candidate.PredictedOmega[0]) < 0.0001;
    }
    TestTrue(TEXT("direction reversal either converges consistently or reports invalid, never stale accepted response"), Matches);
    AddInfo(FString::Printf(TEXT("T6_DIRECTION_BOUNDARY torque=%.8f factor=%.8f predicted=%.8f valid=%d"),Torque,Candidate.TorqueFactor,Candidate.PredictedOmega[0],Candidate.bValid));
    return true;
}
#endif
