# PINK CAB - FIRST EURO Execution Roadmap

**Status:** CURRENT EXECUTION MIRROR - reconciled 2026-10-09.
**Product:** CD-519. **Mechanics Freeze:** CD-848. **Scope:** CD-753.
**Current vehicle implementation umbrella:** CD-648. **Canonical branch:** protected main.

## Read this first

Only after Vehicle Feel 90 owner acceptance may broader world/taxi/traffic/economy/Neural implementation resume.

Deliver one correctly presented, fully interactive Tatra on a stable road with enjoyable manual driving before expanding the world/taxi/traffic/economy/Neural product.

| Identity / scope | Actual checkpoint |
|---|---|
| Accepted installed fallback | `accepted/p4-rig06-20261007` / `6edea7747d3a8433188c9fb394b98ae9c320d49b`; unchanged |
| Integrated runtime at audit | `c8459ba125ee70093f0ae4c82017e9c8c849199b` / PR #70; Tasks 0-4 scoped technical/admin integration |
| Task 5 branch | `feat/vf90-task5-physical-foundation` / `0b395c95b8866401182475f345e39350f40047b1` |
| Task 5 result | RED test and receipt only; no production physics correction in that branch delta |
| Current whole-car verdict | NOT ACCEPTED; Tasks 5-9 remain open |

A merge, a test PASS, a delivered package and an owner verdict are different facts. Old P02/P03/P04 acceptance remains valid only for its exact scope/candidate. Task counts are not a vehicle/game completion percentage.

## One execution route

`one task branch -> implementation + applicable tests -> PR / verify.yml -> reconciliation -> scoped closeout -> next task`

Human delivery additionally requires `deliver.yml -> exact package smoke -> protected rollback -> owner verdict`. The installed accepted fallback is never silently replaced by an administrative commit. No parallel recovery lane, second vehicle solver or feature-specific CI ecosystem is authorized. Git, Jira descriptions and Confluence current checkpoints must agree before task closure; old comments/receipts are retained evidence, not competing current instructions.

## Immediate roadmap: Vehicle Feel 90 / existing Tasks 0-9

These are the existing approved task numbers, not new Jira epics. Detailed implementation surfaces and tests live in `docs/superpowers/plans/2026-10-07-tatra-ready.md`. `docs/PINK_CAB_VEHICLE_FEEL_90.md` defines acceptance dimensions, not a conflicting execution order.

| Order | Work / player result | Existing owners | Exit / dependency |
|---|---|---|---|
| 0-4 | Guards, exact source/contact identity, stable road marks, V23 appearance and V24 authored cabin bindings | CD-559 / CD-855 / CD-648 | Integrated through PR70; retain exact receipts. Not whole cabin, whole car or commercial release acceptance |
| **5 NOW** | Appropriate physical Tatra rig/collision, mass moments, CoM/inertia; four coherent wheel contacts under load | CD-648 physical foundation, CD-748 boundary, CD-855 asset boundary | Real geometry and settled empty/reference/max-load measurements; no grip or steering compensation |
| 6 after 5 | Real analog clutch/handbrake, correct steering and manual gearbox/reverse actuation | CD-659 / CD-653 / CD-645 / CD-646; preserve CD-643/CD-644/CD-649 accepted constraints | Actual Chaos output is continuous; direction, held target, engagement and energy/RPM continuity verified |
| 7 after 6 | Predictable dry driving: suspension, tyres, braking, acceleration/engine braking and catchable RWD breakaway | CD-652 / CD-650 / CD-656 / CD-654 / CD-655 / CD-641 / CD-642 | Recorded same-input/load/surface before/after manoeuvres; no hidden assists or fabricated performance claims |
| 8 after 7 | Wet/load variants, FPS independence, durable state and normal-drive serviceability | CD-658 / CD-657 / CD-670 / CD-722 / CD-740; CD-559 evidence | 30/60/120 FPS, 30-minute drive, work-based heat/wear, no stuck input/NaN/free repair; declared tolerances |
| 9 after 8 | One exact-source complete-car package for the owner | CD-559 delivery; CD-648 owner milestone; CD-658/CD-921 scoped evidence | Applicable regression/package gates, identity/rollback and explicit owner ACCEPTED; rejection returns to failing task only |

**VF90_ACCEPTED at Task9 unlocks R01.** It is a scoped milestone within existing owners, not blanket closure of CD-648/CD-658/CD-921. Those broad cards retain later world/vertical, model-profile and terminal requirements. Requiring their entire closure before the world they test would create a dependency cycle.

## Required corrections, not more readiness theatre

- Task5 must change production data/assets: SportsCar paths, zero reference CoM and 45.17% rear spring share are recorded deficiencies, not implementation progress. Renaming an asset or proving only positive inertia is insufficient.
- The current mass-moment calculation omits base/crew positions. The reference project target is 45/55 front/rear at 1657 kg, not a historical factory claim. Geometry, load application and inertia require measured proof; do not alter tyre friction to pass a static-load check.
- The current provider uses a 0.95 neutral/gear clutch threshold and boolean handbrake. CD-659/CD-653 own continuous physical actuation. First resolve a supported native integration point; do not restore the retired competing simulation or introduce direct chassis-force propulsion.
- Current `verify-runtime.ps1` selects nine tests. This is focused smoke/regression, not whole-vehicle/full-product acceptance. The stale CockpitBridge test uses the wrong component and obsolete expectations; repair its fixture/contract, not production behavior merely to make an old test green. Use existing verification infrastructure.
- CD-646's former 30 km/h taper / 35 km/h reverse governor is retired; manual reverse uses the actual profile, physical load/shaft validation and recorded manoeuvres.
- Latest correct Task5 truth already existed in CD-648 comments 16576/16578 while several main descriptions lagged. Reconciliation promotes evidence into current entrypoints; it does not imply that no earlier administration occurred.

## Broader FIRST EURO queue - parked until Task9 owner acceptance

CD-869 is R01 route closure, not the owner of the retired similarly named delivery workflow. Keep its accepted road history but park new route work until VF90_ACCEPTED.

| Order / task | Player-visible result | Main reuse / boundary |
|---|---|---|
| R01 / CD-869 | Drive a representative L1 -> L2 -> L1 route and return through the same city | Existing CityCode/RoadGraph/vertical/streaming owners; bounded chunks, no voids/duplicate deltas; >=15-minute owner route check |
| R02 / CD-870 | Full-stop pickup/dropoff, door/boarding and parking work as one loop | Existing FareSession/passenger/cabin/parking owners; no second eligibility or money authority |
| R03 / CD-871 | Garage, parts and repair form a coherent transaction loop | ServiceNode, VehicleBuild/Health, Economy and persistence; exactly once |
| R04 / CD-872 | Moving refueling works during live-road play | Shared FuelTank/Economy/telemetry/RoadGraph; no duplicate fuel or settlement |
| R05 / CD-873 | Weather affects the complete driving/world contract coherently | Reuse Task8 wet-car proof; separately prove weather producers/surfaces/presentation, not retune a second car |
| R06 / CD-874 | Bounded traffic and incidents populate the route | Existing logical/materialized traffic, expiry/collision/persistence/performance budgets |
| R07 / CD-875 | Repeat clients retain identity/history and re-enter ordinary orders | PassengerIdentity, Fare/Order and persistence |
| R08 / CD-876 | Daughter has the bounded FIRST EURO role | Existing character/task/dialogue state; no hidden driving authority or broader social simulation |
| R09 / CD-877 | Basic Neural profile/contact/thread functions persist | Existing identity, permissions, bounded messages and save schema |
| R10 / CD-878 | Receipts, payments and fines are readable and consistent | Presentation of authoritative Economy/Enforcement transactions, never duplicate truth |
| R11 / CD-879 | Consolidated FIRST EURO mechanics acceptance | All WF-01..20 / NEU-01..22 included/excluded rows resolved; exact-candidate full regression/package and owner pass |

This is a finite mechanics closure queue, not permission to drop other included FIRST EURO production/release requirements. Full cabin CD-604, state/health/save CD-670/CD-722/CD-740, L1/L2/vertical/traffic presentation and performance proof remain with their existing owners and must be mapped into their applicable route/product gates. A logical primitive marked DONE does not certify its integrated player loop. Model profiles 603/77 and other wider physics-program requirements remain tracked; they are not prerequisites for this one current 613-car milestone.

## Production / scope locks

Native Unreal Chaos Vehicles on the current 5.8.3 line remains the sole hero-car road-dynamics solver. Vehicle Health and bounded authored/native damage own functional consequences. No ABS/TC/ESP, automatic countersteer/yaw rescue, hidden throttle/brake/clutch/gear decisions, speed governor, velocity overwrite or force boost. Presentation cannot own physics, persistence or economy. The accepted mouse/Space/QWE/H-gate/optional-RMB/analog-control grammar remains binding.

Current gameplay profile values 250 hp / 260 Nm / 8500 max RPM / 925 idle / first and reverse 4.0 are not measured 195 km/h performance or factory specifications. Do not replace the current 613 donor while tuning; future bespoke 603-family identity is unchanged.

FIRST EURO is PC single-player with full Level1 + Level2 and the included taxi/services/world/state/QA scope. Workday I08 is already LOCKED: 12 game hours / 120 real minutes, x6, eligible early end-day. I03/I04 forbid manual quit with an active fare/passenger and require a full stop after fare completion; I06 is no-free-reset damaged-car Repair recovery; M05 requires 0.25 s continuous valid wall contact to reset residual magnetism. These are existing decisions, not new questions or runtime-completion claims; see FIRST_EURO_SCOPE.md and the technical owner pack.

Post-FIRST-EURO: multiplayer/coop/common rooms, L3 gameplay, lifestyle/social ServiceNodes, full Taxi Regulator, daily insurance and broad online/social expansion. Preserve only the agreed extension interfaces now.

## Verification / release gates kept separate

CD-559 retains clean source/LFS/cook reproducibility, complete current-product regression, exact packaged OS-input coverage, crash evidence and safe cached-package provenance. Do not substitute nine focused tests for those broader obligations. Only verify.yml and deliver.yml are active; historical workflow graphs and old green runs are evidence only. Heavy Unreal work shares a physical host with KUKURUZA and must not stop another project's processes.

CD-856 pre-model handoff is DONE. CD-855 presentation/bindings Tasks1-4 are integrated, not deferred; remaining full-cabin features and commercial donor permission/public-exposure remediation are not closed. CD-947 being DONE means an exposure audit was completed, not rights granted. Internal engineering may continue; public/commercial delivery remains blocked until permission or a lawful replacement and exposure disposition are proven. Destructive history/LFS/visibility changes require separate authorization.

## Audit coverage and limits - 2026-10-09

The product-label filter plus CD-418 returned 314 Jira records over four pages; an additional name/parent search located legacy DEADRACE/CD-530, not a second active cockpit backlog. This was full inventory/title/status screening with detailed inspection of current authorities and conflict-bearing tasks, not 314 individually re-executed runtime audits. Historical/recovery/primitive DONE, active implementation, downstream product gates and post-year backlog are distinct categories.

The 48-row PHY registry and 97-row FIRST EURO verification matrix retain exact historical evidence and open criteria. No task/test row is promoted to current PASS by this reconciliation. The existing current entrypoints are updated instead of introducing another plan framework. Administrative edits change no Source, Content, Config, workflow or installed game bytes.
