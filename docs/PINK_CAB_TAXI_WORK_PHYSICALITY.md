# PINK CAB · Physical Taxi Work, Passenger Exchange, Fare Risk & Daily Insurance

**Status:** CURRENT IMPLEMENTATION-FACING TAXI-WORK MIRROR
**Confluence fare:** `5832724`
**Confluence insurance/recovery:** `10321936`
**Jira:** loop `CD-522`; Fare Zero `CD-543`; meter `CD-625`; door/access `CD-619/CD-626`; seats `CD-667`; physical exchange `CD-720`; curb/off-meter `CD-721`; vehicle systems `CD-722`; insurance `CD-741..745`; future regulator `CD-723`; Fare QA `CD-724`.

## 1. Canonical loop

`ORDER / ROUTE → SEE PHYSICAL PASSENGER GROUP → APPROACH OFFICIAL STOP OR CURB → FULL STOP → VISUALLY ASSESS → ADMIT/DECLINE → PHYSICAL DOOR LEVER → PILLARLESS RIGHT-SIDE APERTURE → GROUP BOARDS → DOORS CLOSE → DRIVE FARE → FULL STOP DESTINATION → PAYMENT/CONSEQUENCE → DOORS OPEN → EXIT → NEXT ORDER`

The previous rolling-pickup interpretation is SUPERSEDED.

## 2. Physical passenger / full stop

Passenger/group exists visibly before boarding. Approved contexts are `OFFICIAL_STOP` and `CURB_PICKUP`; no icon-only invisible passenger may complete a fare.

Pickup and normal drop-off require deliberate full stop. Closed passenger doors block exchange. Accepted groups 1–5 may board in one compressed simultaneous event after stop + opening. Seating rear3→front2. Passenger transition/mass application is exact once.

Full-stop speed epsilon/dwell remains calibration, not permission for rolling pickup.

## 3. Pillarless physical doors

Passenger side has no fixed B-pillar. Front/rear passenger doors open away from center, exposing one continuous right-side aperture. Heroine operates a dedicated physical cabin lever/handle; no HUD-only door command is authoritative.

Hinge/latch/reinforcement/seal/animation details remain model/engineering calibration. Damage may create explicit jam/latch state under `CD-600/CD-736/CD-737/CD-740` without silently corrupting fare state.

## 4. Route / road rhythm

FIRST EURO gives route guidance to generated destination/zone. No street-address entry workflow required.

Ordinary road grammar has no traffic lights and no systemic standing traffic-jam state. Traffic may compress/slow/merge while continuing to move. Passenger exchange, parking, ServiceNode, insurance purchase and recovery are explicit stopped/state-boundary cases.

## 5. Metered vs off-meter fare

Official taximeter follows old-taxi logic:

**fare = distance contribution + elapsed fare-time contribution**.

Modes: `METERED` and `OFF_METER`. State may never silently flip.

Open meter/economy values: flagfall, price per distance/time, pulse/rounding, low-speed semantics, lever reset/receipt behavior and exact meter START timing.

## 6. Official stop vs curb risk/reward

`CURB_PICKUP` may produce higher tips, permit explicit off-meter/gray fare, increase non-payment/complaint risk and later regulator scrutiny. These are physical world/work consequences, not morality menu choices.

Exact values remain `CD-569/CD-721` owner/economy work.

## 7. Payment / fare evasion

At drop-off Tatra first stops. Passenger can leave only when passenger doors open.

- doors before payment commit may allow one eligible unpaid runaway event;
- committed payment before door opening disables ordinary unpaid escape for that completed fare;
- transaction cannot be both paid and unpaid;
- payment/receipt/tip/non-payment/complaint events are exactly once/save safe.

## 8. Vehicle technology / physical systems

FGear is the sole hero-Tatra road-dynamics owner. Debugging Delight Vehicle Damage System owns permanent body deformation. Vehicle Health maps authored failures to bounded functional consequences.

Every active real vehicle/taxi subsystem follows:

`CAUSE → DIEGETIC INDICATION → PHYSICAL/DRIVING/WORK SYMPTOM → SERVICE/RECOVERY`

FIRST EURO vehicle-health families include permanent deformation, glass/lamps, door jam/latch, tire/wheel/alignment/suspension consequences, brake heat/fade/hydraulic health and **air-cooled V8 oil/head/fan/oil-cooler/airflow thermal health**.

Generic `engine/coolant overheating` is SUPERSEDED. Current hero engine has no coolant/radiator subsystem.

## 9. Daily insurance — POST-FIRST-EURO future control

Daily insurance is **not implemented in FIRST EURO**. Year one keeps only the generic `RecoveryPolicy/RecoveryHook`; exact terminal/immobilizing recovery remains owner-open `I06` under `CD-750/CD-598`.

Future authority is `CD-741..745` / Confluence `10321936`. If promoted after FIRST EURO, the design may use the dedicated stopped-only physical in-car switch, exactly-once policy transactions and the normal `START → ATTENTION → GO` grammar. Those future policy/claim semantics must not be used to infer the year-one recovery rule.

## 10. Future taxi regulator

Full professional Taxi Regulator/inspection gameplay under `CD-723` is **POST-FIRST-EURO**. FIRST EURO keeps only stable `EnforcementEvent` producers/consumers and ordinary rule/fine consequences. Future regulator logic must subscribe through that interface without rewriting year-one rule producers.

## 11. Proof

- Fare Zero: `CD-724` / `docs/qa/PINK_CAB_FARE_ZERO.md`.
- Vehicle stack/damage: `CD-738` / `docs/qa/PINK_CAB_VEHICLE_STACK_ACCEPTANCE.md`.
- Daily insurance: `CD-745` / `docs/qa/PINK_CAB_INSURANCE_ACCEPTANCE.md`.

Documentation establishes CANON/SPECIFIED only. Runtime evidence is required for IMPLEMENTED/VERIFIED.
