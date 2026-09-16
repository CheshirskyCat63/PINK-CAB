# Contributing to PINK CAB

This dedicated repository is the sole active Git technical source of truth for **PINK CAB** under Jira `CD-519`. `DEADRACE` is legacy/migration-source only.

## Source of truth

- Jira owns live work, priority, dependencies, owner decisions, readiness, acceptance and closure evidence.
- Confluence owns durable approved product/design/world/system contracts.
- GitHub owns code/configuration, implementation-facing mirrors, commits, PR review, CI and build history.
- Exact executable evidence owns IMPLEMENTED/VERIFIED truth for a tested build.

Requirements advance through `CANON → SPECIFIED → IMPLEMENTED → VERIFIED`. Do not collapse these states.

## Before starting work

1. Link a current Jira `CD-*` issue.
2. Read `README.md`, `docs/AUTHORITY.yaml`, `docs/OPEN_DECISIONS.md` and the relevant Confluence/Git implementation contract.
3. Confirm the work belongs to the locked FIRST EURO scope or has explicit post-year/experimental classification.
4. Confirm unresolved owner decisions are not being silently guessed into canonical data/code.
5. If a bought/engine feature already owns the responsibility, extend through the approved adapter instead of introducing a competing production solver.

## FIRST EURO scope guard

Current year-one scope is 12 months, PC, single-player, full L1+L2. Multiplayer/coop/common rooms, L3 gameplay, lifestyle social ServiceNodes, full Taxi Regulator and daily insurance implementation are post-FIRST-EURO unless `CD-753` is formally reopened.

## Branch naming

Use short-lived Jira-keyed branches, for example:

- `feat/CD-787-chaos-calibration`
- `feat/CD-733-input-adapter`
- `fix/CD-792-native-vehicle-acceptance-regression`
- `docs/CD-673-owner-pack-reconciliation`

`main` is canonical integration. Use short-lived Jira-keyed branches; the old DEADRACE administrative/staging branches are historical migration evidence only.

## Commit / PR rules

- Keep commits focused and reference the owning `CD-*` key where practical.
- PR description states outcome, authority, scope class, tests/evidence and known limitations.
- A merge does not equal Jira DONE or runtime VERIFIED.
- Product-intent changes require Jira decision + Confluence authority update + Git mirror reconciliation.

## Definition of Done

DONE requires evidence matching the Jira acceptance criteria. Depending on the issue this may include automated tests, deterministic seed fixtures, packaged runtime smoke, performance/soak measurements, screenshots/video, telemetry/logs, migration/retry tests and provenance/license evidence.

Compilation alone is never gameplay acceptance.

## PINK CAB architecture rules

- Native Unreal Chaos is the sole production hero-Tatra road-dynamics owner under `CD-785..CD-792`.
- Damage/destruction uses native bounded authored states, detachable parts, pooled debris and selective Chaos events; VDS is not a required dependency.
- PINK CAB owns thin adapters, Chaos/Tatra profiles, bounded presentation/arcade controls, Vehicle Health, persistence and normalized telemetry.
- One authoritative owner per responsibility; no hidden second solver.
- C++ owns authoritative state/interfaces/invariants/persistence-critical orchestration; Blueprint remains thin composition/orchestration; DataAssets/DataTables/config own tunables.
- Persistent logical identity is separate from materialized Actor representation.
- Avoid unbounded Tick/world scans/hot-path synchronous loads/spawns and uncontrolled collection growth.
- Persistent money/fare/fuel/service operations use stable IDs and exactly-once semantics.

## Unreal repository hygiene

Never commit generated folders such as `Binaries/`, `Intermediate/`, `Saved/` or `DerivedDataCache/`. Use Git LFS for Unreal binary assets/large source media covered by `.gitattributes`. Keep vendor/third-party content isolated from PINK CAB-owned code/content and maintain provenance.

## Secrets

Do not commit credentials, tokens, private keys or local environment secrets. `.env*` stays ignored except an intentionally safe example file.

## Design-change rule

If implementation proves an approved contract wrong, record the new decision before treating changed behavior as authority. Do not let code, a prototype number or a plugin default silently become canon.
