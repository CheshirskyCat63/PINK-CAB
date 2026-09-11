# PINK CAB · Daily Insurance & Terminal Crash Recovery Contract

**Status:** POST-FIRST-EURO BACKLOG AUTHORITY / NOT IN FIRST-YEAR BASE-100 / NOT IMPLEMENTED / NOT VERIFIED
**Confluence:** `10321936` — 39 · PINK CAB · Daily Insurance & Terminal Crash Recovery Contract
**Owner lock:** `CD-741`
**Physical interaction:** `CD-742`
**Recovery:** `CD-598` / `CD-743`
**Economy:** `CD-569` / `CD-744`
**Save/schema:** `CD-560`
**QA:** `CD-745`
**Vehicle stack:** `CD-730`
**Damage:** `CD-600` / `CD-740`

## Delivery boundary

Owner direction 2026-09-10: **daily insurance is implemented only after the first 12-month FIRST EURO single-player product**.

The complete insurance feature below is preserved as future design authority, but it is excluded from FIRST EURO BASE-100/START-90 implementation coverage. FIRST EURO code preserves only a generic `RecoveryPolicy/RecoveryHook` boundary between terminal Vehicle Health state, Workday/Session, Economy and Persistence so this feature can be connected later without restructuring those core owners.

No insurance switch/toggle, premium, policy state, claim settlement, extra-life transaction or insurance-specific QA is required in the first-year implementation.

## Future feature purpose

Daily insurance is a deliberately simple arcade-sim risk-management system: **one additional terminal-crash life for the current workday**. It is represented by a physical cabin control and an authoritative economy/session state.

It is not a shield, damage reducer, stability aid, general repair subscription or vehicle-physics modifier.

## Future purchase / cabin interaction

- insurance is purchased from inside the Tatra through a dedicated physical switch/toggle;
- purchase is accepted only while the Tatra is deliberately stopped;
- moving actuation is rejected atomically: no charge and no policy-state change;
- one successful purchase creates one `ACTIVE` policy for the current workday;
- an already active policy cannot be bought or charged twice;
- switch/adjacent indication communicates `OFF/UNPAID`, `ACTIVE`, and `CONSUMED/EXPIRED`;
- policy state is authoritative economy/save data; switch representation is presentation only;
- the insurance switch is not automatically mapped to quick targets `1–4`; normal physical targeting and `START → ATTENTION → GO` apply unless explicitly changed later.

## Future lifetime / state machine

`OFF → ACTIVE → {EXPIRED at normal workday boundary | CONSUMED by qualifying terminal crash}`

One policy lasts only the current workday. Unused coverage does not carry into the next workday.

## Future qualifying crash

Insurance is consumed only when the authored FGear/VDS/Vehicle Health recovery classifier declares the Tatra **terminal/immobilized: mechanically unable to continue normal driving**.

Ordinary dents, broken lamps, partial suspension/alignment damage, brake fade, air-cooled thermal faults, door faults or other still-driveable states do not automatically consume insurance.

## Future recovery branches

### Uninsured

The first-year terminal-crash recovery rule remains independently owner-open under BASE-100 state/session work (`CD-750` / Pack `I06`). Insurance must not define that FIRST EURO rule merely by being a future feature.

### Insured future branch

`TERMINAL/IMMOBILIZING CRASH + ACTIVE POLICY → CLAIM CONSUMED EXACTLY ONCE → CURRENT WORKDAY ENDS → NEXT-WORKDAY RECOVERY`

The insured branch does not continue from the wreck. No tow-truck minigame, magical roadside rebuild or instant continuation is part of this authority.

Both branches invalidate transient hand/gaze/pedal/quick-target states and preserve the same CityCode identity unless future recovery authority explicitly changes that invariant.

## Vehicle-technology boundary

`PHYSICAL CRASH → FGEAR/VDS/VEHICLE HEALTH RESULT → TERMINAL? → RECOVERY POLICY HOOK → POLICY-SPECIFIC BRANCH`

Insurance never writes tire force, grip, steering, brake, suspension, collision or deformation parameters. It never makes the Tatra physically tougher.

## Future persistence / exactly-once contract

When implemented post-FIRST-EURO, persist/version at minimum:

- policy id;
- workday id;
- policy state `OFF/ACTIVE/CONSUMED/EXPIRED`;
- premium transaction id;
- purchase event boundary;
- linked terminal-crash event id;
- claim transaction id;
- settlement-version id.

Purchase, expiry and claim consumption are exactly-once transitions. Save/load/reset may not duplicate premium, claim, fare, tip, fine, fuel, service, repair or passenger state.

## POST-FIRST-EURO feature scope

Required only when this future feature is promoted:

- physical in-cabin switch and status indication;
- stopped-only purchase and moving rejection;
- one-current-workday validity;
- one qualifying crash consumption;
- insured next-workday recovery branch;
- exact-once persistent premium/policy/claim transitions;
- no mutation of FGear/VDS crash behavior;
- acceptance under `CD-745` or its future replacement.

## Remaining future owner/economy locks

These remain intentionally OPEN but do **not** block FIRST EURO:

1. daily premium amount/formula;
2. deductible or zero deductible;
3. which already committed fares/tips/fines/fuel/service transactions survive an insured forced day-end;
4. next-day Tatra restoration depth;
5. insufficient-funds behavior interaction;
6. whether claim history affects future premium.

Final switch placement/travel/detent/indication is future presentation/calibration and is outside current BASE-100 code readiness.

## Maturity

Feature direction is **PRESERVED POST-FIRST-EURO AUTHORITY**. It is **not part of the first-year implementation plan**, and runtime remains **NOT IMPLEMENTED / NOT VERIFIED**.
