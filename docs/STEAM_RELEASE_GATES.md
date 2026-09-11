# PINK CAB · Steam / PC Release Gates

**Status:** CURRENT RELEASE-CHECKLIST MIRROR / PLATFORM DETAILS MUST BE REVERIFIED AT EXECUTION
**Last external Steamworks verification recorded in this file family:** 2026-09-05
**Product root:** Jira `CD-519`
**FIRST EURO scope:** `CD-753`
**Roadmap/acceptance:** Confluence `6651945`
**QA/evidence:** Confluence `6750209`

The active repository is `CheshirskyCat63/PINK-CAB`; `DEADRACE` is legacy/migration-source only. Steam's live partner documentation and the app's current Steamworks checklist override this file if platform requirements change.

## Commercial target

FIRST EURO is a paid PC single-player PINK CAB release. Exact storefront package, supported OS matrix, pricing and SDK requirements remain runtime/release locks under `CD-557/CD-594` until implementation/release execution.

Historical Steamworks research recorded a $100 USD-equivalent Steam Direct fee, possible 30-day app-fee wait for early titles, a two-week public Coming Soon minimum, and separate store/build review processes. These values must be rechecked against live Steamworks before money/payment/submission decisions.

## G0 · Storefront foundation

Pass only when the chosen storefront/app account is actually configured and the current legal/platform requirements are verified. For Steam this includes partner onboarding, app creation, required tax/bank/company data, permissions and a dated release-calendar record.

No documentation-only state is treated as platform approval.

## Store-ready gate

Before store submission:

- final/near-final product name/logo and truthful feature list;
- required capsule/library artwork from the live checklist;
- screenshots and trailer taken from the actual PINK CAB build, not concept art presented as gameplay;
- supported language/system-requirement declarations backed by tested build scope;
- content survey/disclosures as applicable;
- price/currency proposal valid under the live storefront rules;
- copyright/trademark/license/provenance review complete;
- store copy describes the shipped FIRST EURO single-player scope, not post-year multiplayer/L3/lifestyle/insurance dreams.

## Build-review / RC gate

Pass only when an exact candidate build is uploaded and can be reproduced from identified source/config. Required internal evidence includes:

- clean install / reinstall launch;
- keyboard+mouse controls and declared settings;
- save/config path and migration behavior;
- exact build/commit/content/schema/plugin versions;
- no release-blocking crash or monotonic streaming/state leak in required soak tests;
- credits/licenses/disclosures present;
- correct depot/build selected;
- rollback/hotfix path documented;
- known issues explicitly accepted by the owner.

A compile/build pass is not gameplay verification.

## FIRST EURO release gate

Release only after the chosen storefront's current approval/timing requirements are satisfied and the exact release build is identified. Any live minimum-wait/Coming-Soon/review rules must be reverified immediately before submission/release.

## Scope truth

Release material may advertise only what is IMPLEMENTED/VERIFIED in the reviewed build. Current 12-month product scope is PC single-player with L1+L2, Tatra/FGear/VDS, taxi/fare/passenger/repeat-client/basic Neural, CityCode/world/traffic/rules, automotive ServiceNodes, moving refueling and save/build/QA architecture.

Post-FIRST-EURO systems—multiplayer/coop/common rooms, L3 gameplay, lifestyle social nodes, full Taxi Regulator and daily insurance—must not appear as shipped features unless scope is formally reopened and executable evidence exists.

## After release

Stabilization priority is reproduce → evidence → smallest verified fix → new exact candidate → regression proof. Do not expand feature scope during emergency stabilization.

## Authority rule

Jira owns live release status and blockers; Confluence owns durable product/release contracts; Git records code/config/build evidence; storefront live documentation owns current platform requirements.
