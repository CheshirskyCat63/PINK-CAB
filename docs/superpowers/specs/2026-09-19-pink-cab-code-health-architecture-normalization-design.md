# PINK CAB Code Health & Architecture Normalization

**Date:** 2026-09-19
**Project:** PINK CAB
**Engine:** Unreal Engine 5.8.x / native Chaos Vehicles
**Architecture authority:** BASE-100 `CD-746`, current control/mechanics authority `CD-848` / Confluence page `16744449`
**Supersedes for code-health scope:** the one-time hotspot policy in `2026-09-14-pink-cab-modular-cockpit-code-health-design.md`; accepted behavior and cockpit contracts from that document remain valid.

## 1. Purpose

Normalize the PINK CAB codebase so a competent Unreal/C++ developer can locate the owner of a gameplay concern, understand its public contract, make a bounded change, and identify the relevant tests within five minutes.

This is a no-behavior-drift architecture program. It must improve ownership, modularity, testability and navigation without changing accepted driving, taxi, persistence, world or cockpit behavior unless a separately approved gameplay task explicitly requests such a change.

The end state is not "many tiny files". The rule is:

- one file = one coherent responsibility / primary type;
- one function = one action at one abstraction level;
- one authoritative owner = one responsibility;
- one dependency direction = no hidden back-edge;
- one device-coordinate normalization boundary = no distributed sign fixes;
- one source of runtime truth per domain state.

Literal one-function-per-file is explicitly rejected because it would fragment Unreal/C++ navigation and create unnecessary include/build overhead.

## 2. Current verified baseline

At the 2026-09-19 audit of branch `fix/CD-848-human-gate-regressions` at base HEAD `852530564f4de1b704ec5a356d02ac707f402895`:

- production modules physically present: `PinkCab`, `PinkCabTests`;
- production source: 157 files / ~14,245 non-comment LOC;
- domain folders: Vehicle, Persistence, World, Taxi, Cockpit, Service, Enforcement, Interaction, Traffic, Core, Economy;
- largest production file: `Private/Vehicle/PinkCabChaosTatraPawn.cpp`, ~1,226 non-comment LOC;
- that file contains ~51 includes and owns/coordinates multiple unrelated concerns;
- the current cross-domain include graph contains a strongly connected component spanning `Cockpit + Persistence + Service + Taxi + Vehicle`;
- current BASE-100 doctrine already requires one authoritative owner per responsibility and locked shipping runtime domains `Core / Vehicle / Taxi / World / Traffic / Economy / Persistence / Interaction`;
- prior CD-823 code-health work successfully reduced earlier monoliths but did not install a hard architecture-regression ratchet, so monolithic growth returned.

The current dirty CD-848 gearbox/input repair is not part of this spec commit and must be completed/preserved independently before architecture migration touches the same behavior.

## 3. Success criteria

The architecture normalization gate is complete only when all of the following are true on one exact HEAD:

1. A new developer can use `docs/ENGINEERING_START_HERE.md` to find the owner, public contract and tests for common changes in under five minutes.
2. The shipping dependency graph is acyclic at domain/module level.
3. Raw keyboard/mouse/device-coordinate interpretation is owned by Interaction/Input only.
4. Chaos API writes are owned by Vehicle adapters/runtime only.
5. Persistence owns serialization/checkpoint/migration; gameplay domains do not depend on persistence internals.
6. Presentation code consumes authoritative state but cannot become a second gameplay owner.
7. `APinkCabChaosTatraPawn` is a composition/lifecycle root rather than the implementation of every vehicle/cockpit concern.
8. New architecture violations are machine-detected and fail the code-health gate.
9. UBT project-only static analysis has zero actionable project diagnostics.
10. Full project automation, runtime acceptance and package smoke remain green.

## 4. Architectural doctrine

### 4.1 Ownership

Every responsibility must answer four questions without reading implementation internals:

- What does it own?
- What public inputs does it accept?
- What public outputs/state does it expose?
- What may it depend on?

A type that cannot answer those questions concisely is not a valid architectural unit.

### 4.2 Dependency direction

Dependencies point from orchestration toward stable contracts and from high-level product behavior toward lower-level adapters. Back-edges are forbidden unless represented through an intentionally inverted interface/event contract owned by the lower-level neutral layer.

Private cross-domain mutation is forbidden.

### 4.3 Unreal-specific boundaries

Use Unreal modules for stable shipping-domain boundaries where moving code is asset-safe. Use ActorComponents for behavior/lifecycle that belongs to one actor. Use Subsystems only for services with world/game-instance/editor lifetimes that are truly independent of one actor.

Do not move reflected `UCLASS`/`USTRUCT` types between modules merely to satisfy cosmetics. `/Script/PinkCab.*` compatibility and asset references take priority; safe thin shells may remain in the root module while pure logic/adapters migrate behind stable interfaces.

### 4.4 Pure logic before UObject

State machines, calculations, validation, mapping and codecs should be plain C++ structs/classes where engine lifetime/reflection is unnecessary. UObject/Actor/Component layers adapt those units to Unreal lifecycle and assets.

### 4.5 Configuration over literals

Gameplay tuning belongs in versioned config/DataAssets/profiles where appropriate. Device sign conventions, gear topology and authoritative semantic mappings are contracts, not distributed literals.

## 5. Target module/domain map

The locked BASE-100 runtime domains remain the target. Do not invent additional shipping modules unless a real ownership boundary cannot fit these domains.

```text
PinkCab (Core / composition compatibility shell)
  |
  +-- PinkCabInteraction
  |      input device adapter
  |      semantic input router
  |      physical interaction grammar
  |      canonical coordinate convention
  |
  +-- PinkCabVehicle
  |      vehicle control runtime
  |      drivetrain / gearbox / clutch / pedals
  |      steering / handbrake
  |      Chaos adapter
  |      vehicle health
  |      normalized vehicle telemetry
  |
  +-- PinkCabTaxi
  |      orders / fare / taximeter
  |      passenger identity/runtime
  |      repeat-client / basic Neural product logic
  |
  +-- PinkCabEconomy
  |      money / transaction ownership
  |
  +-- PinkCabWorld
  |      CityCode / route / materialization / ServiceNode world contracts
  |
  +-- PinkCabTraffic
  |      traffic entities / incidents over World contracts
  |
  +-- PinkCabPersistence
         snapshots / codecs / migration / checkpoints / restore orchestration
```

`Cockpit` and `Service` remain focused packages/components but do not become extra shipping modules unless later evidence proves that module separation materially reduces coupling without violating the locked A03 anti-overfragmentation rule.

## 6. Allowed dependency DAG

The intended high-level direction is:

```text
Interaction -> Core contracts
Vehicle     -> Core contracts
Economy     -> Core contracts
World       -> Core contracts
Traffic     -> World + Core contracts
Taxi        -> Core + Economy + Vehicle-read-contracts + World-read-contracts
Persistence -> Core snapshot contracts + domain snapshot DTOs
Composition -> all public domain interfaces needed to wire runtime
```

Rules:

- no domain depends on `Persistence` runtime internals;
- `Persistence` may serialize domain-owned snapshot DTOs but may not mutate private gameplay state directly;
- `Vehicle` must not depend on `Taxi` or `Persistence` implementation;
- `Cockpit` is presentation/input adaptation and must not own Taxi/Vehicle truth;
- `Service` orchestrates public Economy/Vehicle/World contracts, not private members;
- `Interaction` does not depend on gameplay-domain implementation;
- no cycle at domain/module level is accepted.

## 7. Vehicle composition-root decomposition

`APinkCabChaosTatraPawn` remains the Unreal possession/vehicle composition root for compatibility, but its implementation must be reduced to lifecycle, wiring and very thin delegation.

Target responsibilities extracted from the pawn:

### 7.1 `FPinkCabPhysicalInputConvention`

Own exactly one conversion from device/engine coordinates into driver-space semantic axes.

Permanent driver-space contract:

- `+X = right`;
- gearbox `+Y = forward/top row 1/3/5`;
- gearbox `-Y = rear/bottom row 2/4/R`.

Gearbox, steering, handbrake and presentation code are forbidden from inventing independent device sign conventions.

### 7.2 Vehicle input bridge

Reads semantic controls from Interaction and produces a normalized vehicle-control command. It owns no drivetrain simulation.

### 7.3 Vehicle control runtime

Coordinates clutch, throttle, brake, gearbox, handbrake, launch/stall and steering controllers at the gameplay-mechanics level. It does not read raw keys/mouse.

### 7.4 Chaos vehicle adapter

The only owner of writes to Chaos movement APIs and engine-specific sign/unit conversion. It exposes normalized telemetry upward.

### 7.5 Cockpit presentation adapter

Consumes read-only vehicle/taxi/cockpit presentation state and drives meshes/materials/transforms. It cannot mutate authoritative gameplay state except by emitting semantic user commands through the interaction contract.

### 7.6 Vehicle state/persistence adapter

Converts authoritative vehicle state to/from explicit snapshot DTOs. Serialization format and migration remain Persistence-owned.

### 7.7 System menu / input-focus presentation

Menu, cursor capture and focus-loss cleanup belong to a focused UI/input-mode unit rather than drivetrain code.

Target pawn shape is intentionally small:

```cpp
void APinkCabChaosTatraPawn::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    InputBridge->Update(DeltaSeconds);
    VehicleRuntime->Update(DeltaSeconds);
    Presentation->Update(DeltaSeconds);
}
```

Exact implementation may use components or plain owned C++ objects, but the pawn must not regain those responsibilities afterward.

## 8. File and function policy

### 8.1 Default limits

These are code-health defaults, not license to game the metrics:

- production `.cpp`: target <= 350 non-comment LOC;
- production `.h`: target <= 250 non-comment LOC;
- ordinary function: target <= 80 LOC;
- cyclomatic complexity: target <= 15;
- one primary production type/responsibility per file;
- no anonymous `Utils`, `Manager`, `Common` dumping ground without a narrow domain noun and contract.

Exceptions require an allowlist entry with a written single-responsibility rationale. Generated code, declarative tables and cohesive immutable data definitions may qualify. `APinkCabChaosTatraPawn.cpp` does not.

### 8.2 Header policy

Public headers expose contracts and data required by consumers. Implementation helpers belong in Private headers/cpp. Header-only business/state-machine implementation is migrated to `.cpp` unless it is a small value type, template or demonstrably clearer immutable mapping.

### 8.3 Naming

Names encode role:

- `...Controller` = deterministic control/state logic;
- `...Adapter` = converts between two contracts/technologies;
- `...Component` = Unreal ActorComponent lifecycle/behavior;
- `...Service` = domain operation owner with explicit lifetime;
- `...Snapshot` / `...Dto` = data only;
- `...Codec` = encode/decode only;
- `...Profile` = authored/versioned configuration;
- `...Presenter` / `...Presentation` = read-only view projection/visual drive.

## 9. Persistence normalization

Persistence is a serialization/orchestration boundary, not a dependency magnet.

Each domain provides explicit versioned snapshot data and restore/apply contracts. Persistence owns:

- archive format;
- codec selection;
- schema version/migration;
- atomic checkpoint orchestration;
- failure semantics.

Domains own invariants and state validation. Persistence may request `CaptureSnapshot()` / `RestoreSnapshot()`-style public contracts but must not reach into private domain members.

Taxi/Vehicle/Service must not include persistence implementation headers. Snapshot DTO placement is chosen to preserve one-way dependencies: either neutral Core contracts or domain-owned data contracts consumed by Persistence.

## 10. Input and coordinate normalization

All physical device samples enter through one Interaction boundary.

Pipeline:

```text
OS / UE input
 -> device normalization
 -> semantic action/axis state
 -> gameplay command
 -> domain controller
 -> engine adapter
```

Forbidden:

- feature code calling raw key/mouse state directly;
- `-MouseDeltaX` / `-MouseDeltaY` fixes outside the device normalization boundary;
- Gearbox or Steering knowing Windows screen-space conventions;
- visual presentation deciding gameplay sign;
- tests constructing already-correct semantic signs when the behavior under test is raw input mapping.

Runtime tests must cover the real PlayerController/input path for critical owner controls.

## 11. Code-health analyzer and architecture ratchet

Create a repository-owned deterministic analyzer, runnable locally and in CI.

It reports and gates:

1. source LOC by file;
2. function length;
3. approximate/function cyclomatic complexity;
4. include fan-in/fan-out;
5. domain dependency graph;
6. strongly connected components;
7. forbidden include edges;
8. raw-input API usage outside Interaction;
9. Chaos write API usage outside Vehicle adapter;
10. direct Persistence-runtime dependency from gameplay domains;
11. allowlisted size/complexity exceptions with rationale;
12. architecture delta versus committed baseline.

The initial migration may baseline existing debt, but the ratchet is monotonic:

- no new cycle;
- no larger God Object;
- no new forbidden ownership violation;
- no worse complexity/LOC violation without an explicit architecture-review exception.

The final normalization gate removes all temporary debt baseline entries covered by this program.

## 12. Static analysis

Canonical compiler/static analysis command remains UnrealBuildTool project-only analysis, for example:

```text
Build.bat PinkCabEditor Win64 Development <PinkCab.uproject> -WaitMutex -NoHotReloadFromIDE -StaticAnalyzer=VisualCpp -StaticAnalyzerProjectOnly
```

Static analysis complements, not replaces, the repository architecture analyzer. Compiler analyzers find correctness hazards; the code-health analyzer enforces ownership and modularity.

## 13. Test strategy

This program uses characterization-first TDD.

Before extracting a responsibility:

1. identify public behavior relied on by current runtime;
2. add/strengthen a focused test that would fail if the extraction changes that behavior;
3. prove RED where a missing architectural seam or known bug is being introduced/fixed;
4. make the smallest extraction/change;
5. rerun focused tests;
6. rerun the affected domain suite;
7. periodically rerun full `Automation RunTests PinkCab`.

For pure behavior-preserving moves where the behavior test is already demonstrably capable of failing, preserve that characterization evidence and do not invent meaningless implementation-detail tests.

Critical real-input controls require PlayerController/Tick-path acceptance rather than only direct internal-frame calls.

## 14. Migration sequence

### Phase 0 — stabilize current human-gate repair

Complete the currently dirty CD-848 gearbox/input diagnostic/fix first. Do not mix behavior repair with architecture extraction.

### Phase 1 — install architecture observability

Add code-health analyzer, dependency graph, baseline, thresholds and `ENGINEERING_START_HERE.md`. This creates visibility before moving code.

### Phase 2 — normalize input ownership

Centralize device-coordinate conventions and semantic input ownership. Remove raw input/sign interpretation from Vehicle/Cockpit feature code.

### Phase 3 — decompose `APinkCabChaosTatraPawn`

Extract input bridge, control runtime, Chaos adapter, presentation/menu-focus units and state adapters in small green steps. Pawn becomes composition root.

### Phase 4 — break the current five-domain SCC

Remove `Cockpit/Persistence/Service/Taxi/Vehicle` cycles by replacing private implementation dependencies with explicit interfaces/events/DTOs and correcting ownership.

### Phase 5 — persistence boundary cleanup

Separate snapshot DTOs, codecs, migration and orchestration. Remove gameplay-domain dependency on persistence implementation.

### Phase 6 — physical UE module boundaries

Migrate asset-safe pure logic/contracts into the locked shipping modules. Preserve reflected-type compatibility where asset moves would create avoidable redirect risk. Do not overfragment beyond A03.

### Phase 7 — hotspot normalization

Process remaining files over policy thresholds by responsibility, not by arbitrary slicing. Each retained exception requires explicit rationale and analyzer allowlist.

### Phase 8 — harden CI ratchet

Architecture analyzer becomes fail-closed for forbidden edges, cycles, new debt and expired allowlist entries. UBT static analysis becomes part of the same code-health verification workflow.

### Phase 9 — onboarding and final evidence

Update architecture map, ownership tables, common-change lookup, build/test commands and exact-head evidence. Run independent review before merge.

## 15. Five-minute developer onboarding contract

`docs/ENGINEERING_START_HERE.md` must contain, at minimum:

- project root / `.uproject` / UE version;
- canonical build and test commands;
- runtime module map;
- dependency diagram;
- "I want to change X -> go here" table;
- owner/interface/test location for steering, gearbox, throttle, clutch, handbrake, fare, economy, persistence, world/traffic;
- architecture rules and code-health command;
- links to current authority docs only, clearly marking superseded history.

Example lookup:

```text
steering behavior       -> Vehicle/Steering
H-gate topology          -> Vehicle/Transmission
mouse/device sign        -> Interaction/InputConvention
pedal response           -> Vehicle/Pedals
Chaos translation        -> Vehicle/ChaosAdapter
fare logic               -> Taxi/Fare
money settlement         -> Economy
save/migration           -> Persistence
cockpit visual transform -> Cockpit presentation
```

## 16. Quantitative definition of 100% code health

For this program, "100% clean" is an auditable gate:

- domain/module dependency SCCs larger than 1: `0`;
- forbidden dependency edges: `0`;
- raw device interpretation outside Interaction/Input boundary: `0`;
- direct Chaos control writes outside Vehicle adapter boundary: `0`;
- gameplay-domain dependency on Persistence implementation: `0`;
- unallowlisted production `.cpp` over 350 LOC: `0`;
- unallowlisted production `.h` over 250 LOC: `0`;
- unallowlisted functions over 80 LOC: `0`;
- unallowlisted complexity over 15: `0`;
- duplicate authoritative state owners identified by architecture review: `0`;
- actionable project UBT static-analysis diagnostics: `0`;
- `git diff --check`: PASS;
- canonical build: PASS;
- full PinkCab automation: PASS;
- critical real-input runtime acceptance: PASS;
- package smoke fatal/assert/ensure/unhandled: `0`;
- onboarding map matches exact accepted HEAD.

This score does not claim mathematical perfection or zero future technical debt. It means every rule in this agreed architecture contract is satisfied and machine-verifiable at the accepted release head.

## 17. Risks and mitigations

### Reflected Unreal type moves

Risk: asset/script references break if a reflected class changes module/package.
Mitigation: migrate pure logic first; retain compatibility shells; use CoreRedirects only when a move is materially valuable and separately verified.

### Behavior drift during extraction

Risk: vehicle feel/input semantics change while code is reorganized.
Mitigation: characterization tests, exact owner-control regression, small commits and no tuning changes in refactor commits.

### Cosmetic file splitting

Risk: many files but same hidden coupling.
Mitigation: analyzer evaluates dependency edges/ownership/cycles, not only LOC.

### Over-modularization

Risk: Unreal compile/config complexity grows with too many modules.
Mitigation: keep the locked A03 module set and use focused classes/components inside those domains.

### Architecture documentation drift

Risk: diagrams become stale after code changes.
Mitigation: generate dependency facts from source; documentation references generated/verified ownership data where practical; CI checks architecture contract.

## 18. Non-goals

This program does not:

- retune steering, gears, throttle, clutch, tires or suspension;
- redesign controls;
- replace Chaos Vehicles;
- change FIRST EURO scope;
- replace final Tatra assets;
- rewrite all code from scratch;
- create speculative frameworks for post-FIRST-EURO systems;
- split every function into a separate file.

## 19. Final acceptance evidence

Before claiming completion, record exact HEAD and attach:

- architecture analyzer report;
- dependency DAG with zero SCC > 1;
- UBT static-analysis result;
- clean build result;
- full automation count/pass result;
- critical real-input runtime result;
- package smoke result;
- independent code review findings and disposition;
- Jira/Confluence synchronization to the verified implementation state.

Documentation or a green compile alone is never sufficient evidence.
