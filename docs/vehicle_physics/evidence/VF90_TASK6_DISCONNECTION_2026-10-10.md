# VF90 Task6 - neutral/open-clutch disconnection / 2026-10-10

Status: bounded disconnection and ordinary-profile handbrake verified locally; whole Task6 remains RED. Draft PR73, not merged or delivered. No whole-car/owner acceptance.

## Source identity and actual changes

Runtime correction: `1d6929d58428b251ac0bf94fab4ea68a9eab6010`. Additional live-coast test and canonical-list update: `e6950e178f5f58384f05dc31cdae40ad55120103`; this changes no production runtime code. The editor was built from the corresponding working files, subsequently captured by these commits. No new packaged executable or installer was produced. Accepted installed source remains `6edea7747d3a8433188c9fb394b98ae9c320d49b`; main remains Task5 `ed18a57` at this checkpoint.

`FPinkCabChaosDrivelineSimulation` is the single standard Chaos wheeled simulation instance, overriding only native input processing. When the actual native transmission is neutral, its per-instance wheels are temporarily ineligible for engine braking during the native ApplyInput calculation; their driven-wheel flags are immediately restored. No shared wheel configuration is changed. Forward/reverse mechanical simulation, wheel friction, suspension and chassis integration remain inherited. EngineBrakeEffect is not zeroed; service and handbrake remain native. This is not a second dynamics solver or the retired external partial-torque path.

The existing provider still maps clutch coupling below0.95 to native neutral. Therefore this change repairs neutral and fully-open-clutch isolation, NOT continuous partial coupling. That missing requirement is retained as real failing tests.

## Verified physical outcomes

The original RED reproduced138.75Nm rear braking on idle neutral and larger drag after free revving despite an open connection. The repaired11-case native fixture covers N/open clutch, ignition permission, throttle, foot brake, handbrake, connected1/R and return toN.

- Neutral / fully open clutch: zero measured engine propulsion and zero unwanted wheel braking.
- Foot brake25%: front600Nm, rear550Nm, without added engine drag.
- Handbrake50%: rear850Nm, front0.
- Connected first: positive native rear drive about691.64Nm per wheel in the recorded restrained fixture; reverse about-691.25Nm; subsequent neutral returns drive/brake to0. These are fixture outputs, not a car performance target.
- The stationary counter-torque test keeps physics awake only within that test so it measures newly evaluated torque, not a cached sleeping output. Separate AnalogHandbrakeTorque retains natural sleep and proves proportional input/release with the ordinary engine profile.

A separate actual-road test uses normal native throttle to launch, then opens the clutch and selects neutral. It contains no velocity/position reset, direct force or test sleep override. After launch, open-clutch coast travelled357.604cm in1.024s and neutral coast354.043cm in1.024s. Each coast interval sampled39 frames with four contacts and zero engine drive/brake torque. Launch-stage torque maxima were not sampled; their zero-initialized log fields must not be interpreted as a zero-force launch.

## Regression results, kept separate

| Run | Result | Source / limits |
|---|---|---|
| Canonical final runtime list | **23/23 PASS** | `t6-canonical23-20261010-wrapper.log` ends with PINKCAB_FOCUSED_RUNTIME=PASS tests=23; includes original20 plus disconnect, live coast and ordinary handbrake |
| Joint controls / cockpit / interaction / provider / actuation | **75/77 PASS, 2 FAIL** | `t6-controls-final-20261010.log`, runtime source1d6929d; fails are PartialClutchTransfer and ClutchCapability; additional coast test was run separately afterwards |
| Native live-coast addition | **PASS** | `t6-live-coast-20261010.log`; no test-generated launch velocity/force |
| Script suite | **56/56 PASS** | captured in t6-disconnect-scripts-20261010.log; later changes were C++ tests and the explicit runtime test list |
| Static gates | **PASS** | code-health0, hygiene0, authority/scope/diff checks0 on the runtime correction; no guard allowlist expansion |
| Independent code review | No major issues reported | PR73 review at1d6929d; not approval of missing partial-clutch work or later unreviewed changes |

Counts are overlapping, separately scoped runs, not a summed game-readiness score. Hosted final-head CI/review and applicable packaging remain separate. A green focused list does not close the two RED whole-control requirements.

## Regression failures investigated, not hidden

The first canonical22 run failed an obsolete bool-handbrake assertion and static axle-load/sag checks. The bool test now checks the actual project analog command against the parked lever, while asserting the retired boolean channel stays false. Old provider tests now instantiate the correct project component and supply explicit combustion authority. A test that confused optional RMB capture with forbidden capture was corrected without changing input grammar. Retired external-partial-torque assertions were replaced by explicit transport checks plus a genuine physical partial-clutch test, which remains RED.

Read-only diagnostics showed frozen spring output from sleeping bodies. A native-sleep A/B retained identical mass/CoM/springs and the53%-57% balance criterion: the stock aggressive vehicle-sleep threshold10 froze about58.5% rear share; threshold0 passed the same criterion. Production now disables only that documented aggressive shortcut. Ordinary rigid-body sleep remains, with no per-frame forced wake in the game. Empty/reference/max/reference load checks subsequently passed in the final canonical run. This does not certify cross-FPS/long-run suspension behaviour or turn raw spring output into calibrated absolute tyre force.

The unadopted Modular clutch probe is preserved byte-exact as `RESEARCH_NATIVE_CLUTCH_UNADOPTED_2026-10-10.cpp.txt`, outside compilation. Unity compilation exposed conflicting standard/Modular EForceFlags declarations. The experiment had never proved torque transfer and was never an accepted production test. Actual disconnection and partial-clutch requirements remain executable.

## Log integrity (SHA256, under Saved/VF90)

| File | SHA256 |
|---|---|
| t6-disconnect-red3-20261010.log | D8A8DA91E91E1AA1B87192E1915E97B5DC16A29C7BE9E354A7C64983C19216CF |
| t6-disconnect-live-20261010.log | E93B4CA6E1EA16722AAF6FF01869C22E2FFB44AA62CC2D35C0221CFED8CF375F |
| t6-controls-final-20261010.log | 8A32B3ACC624C8DE485ADE586187F6255109D1C0393F40F9802B81AF5CFF3F90 |
| t6-canonical23-20261010-wrapper.log | 865942E6EE7B5C6AA2F27E02CE1995FC312E4403DFF39C0381EE9A47D82DCC63 |
| t6-live-coast-20261010.log | C98C5ECE1BC9B796E247286E5B682FE9B2D7D1BE9A9F8C68EB6C07EB6F515A90 |
| t6-settlement-ab-20261010.log | 074E599122767ECA0499CAAF9A1D5BB46E284F3E9102DAE04C344F8E2C19E8B2 |
| t6-disconnect-scripts-20261010.log | 7064235257E5BBCC3B302CDE2E6C1B3C23175301F78A4CAB7DA396AF85FCC51F |

## Remaining work / no false closure

Continuous torque capacity and engine/wheel reaction at partial clutch, capability declaration, combined thermal/parking/focus/FPS coverage remain Task6 work. Tasks7–9 and the owner's final package remain held. The installer rollback-retention defect in CD-559 is still open. Do not merge this draft as completed controls or replace the accepted desktop build. Some bundled tool commands were rejected and did not execute; subsequent successful operations and actual file/log readbacks, not intent, establish the states above.
