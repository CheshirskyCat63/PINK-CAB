#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "../../../PinkCabVehicle/Private/Vehicle/PinkCabChaosCommandChannel.h"

// Drive the production channel with several real native packet identities queued
// before physics consumes the oldest. No threads or timing sleeps are needed.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPinkCabClutchAsyncFrameTest,
    "PinkCab.Vehicle.Actuation.ClutchAsyncFrameIdentity",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPinkCabClutchAsyncFrameTest::RunTest(const FString& Parameters)
{
    FPinkCabChaosCommandChannel Channel;
    FChaosVehicleAsyncInput Packets[4];
    for (int32 Index = 0; Index < 3; ++Index)
    {
        FPinkCabChaosCommandFrame Frame;
        Frame.bValid = true;
        Frame.Controls.SetDriveline(Index - 1, Index - 1, 0.25f * Index);
        Frame.Native.VehicleInputs.ThrottleInput = 0.1f * Index;
        Channel.Publish(Frame, &Packets[Index]);
    }
    for (int32 Index = 0; Index < 3; ++Index)
    {
        const auto Actual = Channel.ReadCommand(&Packets[Index]);
        TestTrue(TEXT("queued native packet retains its own clutch and gear"),
            Actual.bValid && Actual.Controls.EngagedGear == Index - 1
            && Actual.Controls.ClutchCoupling == 0.25f * Index);
        TestEqual(TEXT("queued throttle is from the same native packet"),
            Actual.Native.VehicleInputs.ThrottleInput, 0.1f * Index);
        TestEqual(TEXT("repeated physics substeps keep the same command"),
            Channel.ReadCommand(&Packets[Index]).Sequence, Actual.Sequence);
    }
    TestFalse(TEXT("unknown packet never borrows the latest controls"),
        Channel.ReadCommand(&Packets[3]).bValid);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPinkCabClutchQueuedResetTest,
    "PinkCab.Vehicle.Actuation.ClutchQueuedReset",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPinkCabClutchQueuedResetTest::RunTest(const FString& Parameters)
{
    FPinkCabChaosCommandChannel Channel;
    FChaosVehicleAsyncInput BeforeReset, AfterReset;
    FPinkCabChaosCommandFrame Frame;
    Frame.bValid = true;
    Frame.Controls.SetDriveline(1, 1, 0.5f);
    Channel.Publish(Frame, &BeforeReset);
    Channel.Reset();
    Channel.Publish({}, &AfterReset);
    const auto Old = Channel.ReadCommand(&BeforeReset);
    TestTrue(TEXT("future reset cannot rewrite an already queued native frame"),
        Old.bValid && Old.Controls.ClutchCoupling == 0.5f);
    TestFalse(TEXT("reset is carried by its own native frame"),
        Channel.ReadCommand(&AfterReset).bValid);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPinkCabClutchFrameRetentionTest,
    "PinkCab.Vehicle.Actuation.ClutchFrameRetention",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FPinkCabClutchFrameRetentionTest::RunTest(const FString& Parameters)
{
    FPinkCabChaosCommandChannel Channel;
    TArray<TUniquePtr<FChaosVehicleAsyncInput>> Packets;
    for (int32 Index = 0; Index <= FPinkCabChaosCommandChannel::MaxQueuedFrames; ++Index)
        Packets.Add(MakeUnique<FChaosVehicleAsyncInput>());
    FPinkCabChaosCommandFrame Frame;
    Frame.bValid = true;
    for (int32 Index = 0; Index < FPinkCabChaosCommandChannel::MaxQueuedFrames; ++Index)
    {
        Frame.Controls.SetDriveline(1, 1, float(Index) / Packets.Num());
        TestTrue(TEXT("retained native packet is accepted"), Channel.Publish(Frame, Packets[Index].Get()));
    }
    TestFalse(TEXT("backlog limit never evicts an unconsumed command"),
        Channel.Publish(Frame, Packets.Last().Get()));
    TestTrue(TEXT("oldest queued command survives capacity exhaustion"),
        Channel.ReadCommand(Packets[0].Get()).bValid);
    TestFalse(TEXT("unregistered rejected packet does not borrow other commands"),
        Channel.ReadCommand(Packets.Last().Get()).bValid);
    Channel.ReadCommand(Packets[Packets.Num() - 2].Get());
    TestTrue(TEXT("consumed history is reclaimed"), Channel.Publish(Frame, Packets.Last().Get()));
    TestTrue(TEXT("only current/repeated packet and future packet remain"), Channel.GetRetainedFrameCount() <= 2);
    // The native manager may recycle an address only after retiring its packet.
    for (int32 Index = 0; Index < 2048; ++Index)
    {
        auto* Packet = Packets[Index % Packets.Num()].Get();
        Frame.Controls.SetDriveline(1, 1, (Index % 7) / 6.0f);
        TestTrue(TEXT("recycled native packet identity accepts its new frame"), Channel.Publish(Frame, Packet));
        TestEqual(TEXT("recycled identity never returns an old clutch state"),
            Channel.ReadCommand(Packet).Controls.ClutchCoupling, Frame.Controls.ClutchCoupling);
    }
    TestTrue(TEXT("retention stays bounded after repeated publication"), Channel.GetRetainedFrameCount() <= 3);
    return true;
}
#endif
