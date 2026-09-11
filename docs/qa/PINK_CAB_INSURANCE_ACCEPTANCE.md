# POST-FIRST-EURO · PINK CAB · Daily Insurance Acceptance Contract

**Status:** FUTURE QA SPEC / EXCLUDED FROM FIRST-YEAR BASE-100 / NOT IMPLEMENTED / NOT VERIFIED
**Spec:** `docs/PINK_CAB_DAILY_INSURANCE_CRASH_RECOVERY.md`
**Confluence:** `10321936`
**Jira QA:** `CD-745`; future owners `CD-741..744`
**FIRST EURO scope:** `CD-753`

Daily insurance is not implemented or verified in the first 12-month FIRST EURO. This file preserves the future acceptance family only. Year-one QA verifies the generic terminal-recovery event and `RecoveryPolicy/RecoveryHook` extension boundary under `CD-598/CD-750`; it does not create policy/premium/claim/switch state.

## Future acceptance families

When insurance is explicitly promoted after FIRST EURO, exact-build tests must cover:

- `INS-PURCHASE-001`: purchase succeeds only under the then-current authoritative stopped condition, creates one premium transaction and one ACTIVE policy;
- `INS-PURCHASE-002`: moving purchase rejects atomically with no charge/state change;
- `INS-PURCHASE-003`: active policy cannot be double-bought or double-charged;
- `INS-DAY-001`: unused one-workday policy expires exactly once and does not auto-renew;
- `INS-CRASH-001`: matched policy OFF/ACTIVE fixtures produce the same FGear/VDS/Vehicle Health physical crash result before recovery routing;
- `INS-CRASH-002`: no-policy terminal recovery follows the common recovery policy that is authoritative **at future implementation time**; this file does not pre-lock the currently pending FIRST EURO rollback reconfirmation;
- `INS-CRASH-003`: ACTIVE policy consumes exactly once on a qualifying terminal/immobilizing event and routes to the future owner-approved next-workday recovery/settlement;
- `INS-CRASH-004`: ordinary still-driveable damage does not consume policy unless the common classifier declares terminal/immobilized;
- `INS-SAVE-001/002/003`: purchase/claim/expiry survive retry/save/reload without duplicated premium, claim, life or state transition;
- `INS-ECO-001`: insufficient-funds behavior inherits the common EconomyService/debt policy rather than inventing insurance-specific credit logic;
- `INS-UI-001`: physical switch/indicator synchronizes from authoritative policy data and never owns truth;
- `INS-INPUT-001`: purchase/rejection/recovery leaves no stuck gaze/hand/pedal/quick-target state.

## Preserved future design direction

Insurance remains a one-workday risk-management layer: physical cabin toggle, stopped-only purchase, one extra terminal-crash life, no change to vehicle toughness/FGear/VDS physics, exactly-once persistent premium/policy/claim/expiry transitions.

Future open owner values remain premium, deductible, committed-day transaction retention, restoration depth, debt interaction and any claim-history pricing.

## Evidence when promoted

Record exact build/commit, UE/FGear/VDS/profile/schema versions, CityCode/workday/session, policy and transaction IDs, stopped/speed result, physical terminal classifier **before** policy routing, balance/settlement expected vs observed and artifact paths.

Documentation establishes only future SPECIFIED direction. This QA family must not be used as a FIRST EURO gate.
