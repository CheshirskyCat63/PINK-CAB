## Jira / scope

- Issue: `CD-___`
- Domain / gate: `BASE100-___ / FIRST-EURO-___`
- Scope: `FIRST EURO 12M / POST-FIRST-EURO / EXPERIMENTAL NON-AUTHORITY`

## Outcome

What player/runtime/production outcome does this PR change?

## Authority / ownership

- Relevant Jira owner(s):
- Relevant Confluence authority:
- Relevant Git implementation contract:
- Authoritative runtime/data owner:
- Engine/plugin/vendor adapter boundary, if any:
- Does this PR change approved product intent? `No / Yes — linked decision + Confluence/Git reconciliation`

## FIRST EURO scope guard

- [ ] The change belongs to the current 12-month PC single-player scope or is explicitly marked post-year/experimental.
- [ ] It does not silently promote multiplayer/coop/common rooms, L3 gameplay, lifestyle social nodes, full Taxi Regulator or daily insurance into FIRST EURO.
- [ ] It does not convert an OPEN owner decision or temporary calibration value into hidden canon.
- [ ] It does not introduce a competing production owner/solver for a responsibility already locked to engine/plugin/domain authority.

## Changes

-

## Verification / evidence

- Exact commit/build tested:
- Engine/plugin/content/schema/profile versions:
- Deterministic CityCode/seed/fixture IDs where relevant:
- Tests/run configuration:
- Expected vs observed runtime result:
- Screenshots/video/logs/telemetry/performance evidence:
- Save/retry/idempotency/migration evidence where state changes persist:

## Asset / license impact

- [ ] No new third-party/vendor/reference material ships, or
- [ ] New/changed material is recorded in `docs/ASSET_LICENSE_LEDGER.csv` with provenance/license status.

## Checklist

- [ ] Jira issue/acceptance is current.
- [ ] Branch/PR references the owning `CD-*` key.
- [ ] Generated Unreal output and secrets are not committed.
- [ ] Relevant automated/runtime checks were actually run.
- [ ] Performance/bounded-runtime impact was considered for hot-path/world/traffic/crowd changes.
- [ ] Jira + Confluence + Git were reconciled if product/system authority changed.
- [ ] Merge alone will not be reported as Jira DONE or runtime VERIFIED.
