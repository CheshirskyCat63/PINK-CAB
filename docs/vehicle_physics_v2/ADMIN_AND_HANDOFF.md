# Vehicle Physics V2 — administration and executor handoff

## Source-of-truth map

- GitHub = implementation, exact SHA, tests/workflows/release evidence.
- Jira = production status/owners/blockers/acceptance evidence.
- Confluence = durable canon/design/physics calibration authority.
- Runtime logs/tests/playtest = actual behavior.
- Chat = command/human gate, not durable sole storage.

## Administrative work completed by the 2026-09-26 audit

### Jira
- CD-921 created as a Subtask of CD-848.
- Kept BACKLOG because no runtime implementation was executed by this planning pass.
- Linked to existing engine/drivetrain/handling/persistence/Vehicle Health/model owners instead of generating duplicate Jira cards.
- CD-722 relation explicitly records that stale FGear/VDS owner wording is superseded by CD-843/current Native Chaos authority.

### Confluence
- Created page 22413517 with the full current Vehicle Physics V2 calibration program.
- Corrected active page26 input grammar, pedal ownership and hidden trajectory-assist conflict.
- Reconciled page27 old drivetrain numerics with current-source provenance + 900–950 idle target.
- Reconciled page28 control precedence/current handling execution.
- Registered CD-921/page22413517 on page47 and Product Family / Authority Index.

### Git
This directory is a documentation-only program surface. It must be merged independently of R4 world work. The branch/PR that introduced it must not claim vehicle behavior is fixed.

## Executor start protocol

Before the first P00 code change:

1. Open the canonical local PINK-CAB workspace.
2. Record repository root.
3. Record current branch and exact HEAD.
4. Record remote and upstream.
5. Run status and record modified/untracked files.
6. Inspect last commits and active PR.
7. Never overwrite existing user uncommitted work.
8. Read current main versions of page47 mirror, Native Chaos tech stack and this directory.
9. Read Jira CD-921 plus the subsystem owner issue for the phase.
10. Read Confluence 22413517 and page47.
11. Verify the latest owner-accepted vehicle/control baseline; do not assume this planning snapshot remains newest.

## Implementation discipline

- Start with P00 telemetry/preflight, not acceleration tuning.
- For a bug, instrument the causal chain first.
- For deterministic code, write/fix a test first where technically possible.
- Change one coefficient family at a time in handling A/B work.
- Never fix engine-off movement by zeroing velocity.
- Never fix sluggish acceleration with an unowned force/torque boost.
- Never fix oversteer by globally deleting rear grip before mass/contact/suspension/torque are measured.
- Never create three diverging Pawns for 613/603/77.
- Never invent historical Tatra data; mark unknowns.
- Preserve accepted input/H-gate mechanics exactly.
- Independent review + exact-head verification before completion.
- Human gate remains mandatory for feel changes.

## Human decision boundaries

Routine calibration implementation inside the accepted phase constraints is autonomous.

Ask for an explicit owner decision only when selecting a final feel candidate, changing hero/canon identity, changing an accepted control mechanic, approving a new irreversible data/schema compatibility break, or introducing a new paid/legal asset/dependency boundary.

## Completion record

For each accepted phase add to CD-921:

- exact SHA/branch/PR;
- workflow/run/job;
- executable/profile hash;
- tests executed and results;
- runtime critical-log scan;
- A/B metrics;
- known issues;
- rollback SHA/build;
- human result when required.

Stable accepted numeric/profile decisions should then be reconciled into Confluence and Git. Do not rewrite historical evidence as if it were current runtime proof.
