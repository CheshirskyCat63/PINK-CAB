template <typename TIsKeyDown>
FPinkCabVehicleInputFrame FPinkCabVehicleInputFrame::FromRouter(
    const FPinkCabSemanticInputRouter& Router,
    TIsKeyDown&& IsKeyDown)
{
    const auto Down =
        [&Router, &IsKeyDown](EPinkCabSemanticAction Action)
    {
        const FKey Key = Router.GetKeyForAction(Action);
        return Key.IsValid() && IsKeyDown(Key);
    };

    return FromDigital(
        Down(EPinkCabSemanticAction::GazeHold),
        Down(EPinkCabSemanticAction::Clutch),
        Down(EPinkCabSemanticAction::Brake),
        Down(EPinkCabSemanticAction::Throttle));
}
