#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "SimModule/ClutchModule.h"
#include "SimModule/EngineModule.h"
#include "SimModule/TransmissionModule.h"
#include "SimModule/SimModuleTree.h"
#include "SimModule/ModuleInput.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPinkCabNativeClutchCandidateTest,
    "PinkCab.Vehicle.Actuation.NativeClutchCandidate",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPinkCabNativeClutchCandidateTest::RunTest(const FString& Parameters)
{
    // Characterize the installed native module BEFORE considering integration.
    // This test does not enable Modular Vehicles or replace the production solver.
    TArray<FModuleInputSetup> Setup = {
        FModuleInputSetup(TEXT("Clutch"), EModuleInputValueType::MAxis1D)};
    FModuleInputContainer::FInputNameMap Names;
    FModuleInputContainer Values;
    Values.Initialize(Setup, Names);
    FInputInterface Controls(Names, Values, EModuleInputQuantizationType::Default_16Bits);
    Chaos::FAllInputs Inputs;
    Inputs.ControlInputs = &Controls;
    Inputs.StateInputs = &Controls;
    float PreviousTorque = -1.0f;
    for (float Coupling : {0.0f, 0.25f, 0.5f, 0.949f, 0.95f, 0.999f, 1.0f})
    {
        Chaos::FSimModuleTree Tree;
        auto* EngineShaft = new Chaos::FEngineSimModule(Chaos::FEngineSettings());
        auto* GearboxShaft = new Chaos::FTransmissionSimModule(Chaos::FTransmissionSettings());
        Chaos::FClutchSettings Settings;
        Settings.ClutchStrength = 1.0f;
        auto* Clutch = new Chaos::FClutchSimModule(Settings);
        const int Root = Tree.AddRoot(EngineShaft);
        const int Middle = Tree.AddNodeBelow(Root, Clutch);
        Tree.AddNodeBelow(Middle, GearboxShaft);
        EngineShaft->SetRPM(2000.0f);
        GearboxShaft->SetRPM(1000.0f);
        GearboxShaft->SetLoadTorque(30.0f);
        Controls.SetFloat(TEXT("Clutch"), 1.0f - Coupling, false);
        // Seed torque arriving at the clutch input; the parent transmitter is not
        // itself under test and would also rewrite shaft speed during transfer.
        EngineShaft->SetDriveTorque(200.0f);
        Clutch->SetDriveTorque(200.0f);
        Clutch->SetRPM(1000.0f);
        Clutch->Simulate(1.0f / 120.0f, Inputs, Tree);
        FString NativeDebug;
        Clutch->GetDebugString(NativeDebug);
        AddInfo(FString::Printf(TEXT("T6_NATIVE_MODULE index=%d %s"), Clutch->GetTreeIndex(), *NativeDebug));
        const float Torque = GearboxShaft->GetDriveTorque();
        AddInfo(FString::Printf(TEXT("T6_NATIVE_CLUTCH c=%.3f output=%.6f engine_load=%.6f engine_rpm=%.3f gearbox_rpm=%.3f"),
            Coupling, Torque, EngineShaft->GetLoadTorque(), EngineShaft->GetRPM(), GearboxShaft->GetRPM()));
        TestTrue(TEXT("native candidate output is finite"), FMath::IsFinite(Torque));
        if (Coupling == 0.0f)
            TestTrue(TEXT("fully disengaged clutch transmits no input torque"), FMath::IsNearlyZero(Torque, 0.001f));
        else
        {
            TestTrue(TEXT("partial coupling transmits positive bounded torque"), Torque > 0.0f && Torque <= 200.01f);
            TestTrue(TEXT("coupling sweep is physically distinct, not a binary torque switch"), Torque > PreviousTorque + 0.0001f);
        }
        PreviousTorque = Torque;
    }
    return true;
}
#endif
