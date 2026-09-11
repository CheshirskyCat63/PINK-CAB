# PINK CAB Tatra E34-Reference Handling Administrative Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Normalize the approved E34-reference/Tatra-expression handling model across Jira, Confluence, Git authority, QA and the remaining Level 1 roadmap residue.

**Architecture:** BMW E34 530i V8 5MT is a behavioral steering/chassis reference only. PINK CAB keeps rear-engine air-cooled V8 identity, rear-heavy layout, 1450/1550 kg mass model, no ABS/ESP, 195 km/h target, analog controls, and an intentionally exaggerated old-Tatra body/cabin expression layer. Runtime remains unimplemented; this plan closes documentation/admin authority and executable-test ownership only.

**Tech Stack:** Jira, Confluence, GitHub Markdown/YAML authority mirrors and QA specifications.

**Spec:** `docs/superpowers/specs/2026-09-10-pink-cab-tatra-e34-handling-design.md`

## Global Constraints

- Do not silently import BMW front-engine packaging, ABS, 218 PS/290 Nm, 235 km/h, exact body dimensions or exact suspension hardware into PINK CAB.
- Keep Tatra mass lock: 1450 kg base, 1550 kg full fuel; heroine 58 kg; daughter 49 kg; passenger ranges 55–70 / 65–90 kg.
- Keep physical Tatra ABS=false and ESP=false.
- Keep Level 1 residual magnet endpoints 5.0 s @1657 kg and 4.0 s @2107 kg with interpolation OPEN.
- Steering/throttle/brake/clutch/handbrake are continuous analog states; no binary drift controls.
- Body/camera/audio expression may exaggerate suspension motion but may not independently change tire forces/grip/collision outcome.
- `LOCKED SPEC != IMPLEMENTED != VERIFIED`.

---

### Task 1: Jira handling authority reconciliation

**Files:** Jira `CD-562`, `CD-641`, `CD-648..659`, `CD-592`, `CD-701`, `CD-722`, `CD-588`.

**Interfaces:**
- Consumes: approved design spec and existing mass/load authority.
- Produces: one Jira statement of donor-reference handling, analog control and Tatra expression ownership.

- [ ] Update `CD-648` epic to name BMW E34 530i V8 5MT as behavioral donor and define donor-vs-Tatra boundary.
- [ ] Update `CD-649` steering to E34-like calm/progressive analog steering; existing numeric gain/lock become calibration candidates, not immutable owner identity.
- [ ] Update `CD-650/651/654` to preserve continuous grip→slide→spin, catchable oversteer and no auto-countersteer/Drift Mode.
- [ ] Update `CD-652` to split physical chassis response from amplified visual/body/camera/audio expression.
- [ ] Update `CD-653`, `CD-656`, `CD-659` for analog handbrake, no-ABS braking and finely adjustable clutch-release speed.
- [ ] Update `CD-657/658` telemetry and acceptance for physical-vs-expression channels and speed fixtures 100/140/170/195 km/h.
- [ ] Update `CD-562`, `CD-722`, `CD-588` to record the owner lock and remaining calibration questions.
- [ ] Update `CD-641` only to clarify that power is not the difficulty at 140–195 km/h; 180 hp/240 Nm remains current unless separately superseded.
- [ ] Read back all edited issues and verify no donor ABS/front-engine/power values became PINK CAB canon.

### Task 2: Durable Confluence handling authority

**Files:** Confluence handling `6225936`, Tatra/cockpit `6619137`, Open Decisions `5832744`, Active Baseline `5537802`, Level 1 World `8388630`, L1→L2 Roadmap `8716289`.

**Interfaces:**
- Consumes: Jira lock from Task 1.
- Produces: durable product/handling authority with no fixed-five-second residue.

- [ ] Rewrite handling authority around E34-control/Tatra-expression two-layer model.
- [ ] Add donor-reference boundary and analog controls to Tatra/cockpit authority.
- [ ] Add `OD-TATRA-E34-FEEL-001` and analog/control calibration residuals to Open Decisions.
- [ ] Update Active Baseline with speed-character bands and suspension-expression doctrine.
- [ ] Update Level 1 World magnetic section to load-sensitive 5.0→4.0 endpoints.
- [ ] Update Roadmap Phase F to load-sensitive endpoint/matrix wording rather than a single fixed 5.0-second boundary.
- [ ] Read back all pages and verify current status/version.

### Task 3: Git implementation-facing authority mirrors

**Files:**
- Create: `docs/PINK_CAB_TATRA_HANDLING_E34_REFERENCE.md`
- Create: `docs/qa/PINK_CAB_TATRA_HANDLING_ACCEPTANCE.md`
- Modify: `docs/AUTHORITY.yaml`
- Modify: `docs/OPEN_DECISIONS.md`
- Modify: `docs/PINK_CAB_ACTIVE_BASELINE.md`
- Modify: `docs/PINK_CAB_VEHICLE_MASS_LOAD_DYNAMICS.md`
- Modify: `docs/PINK_CAB_LEVEL1_WORLD_BIBLE.md`
- Modify: `docs/PINK_CAB_L1_TO_L2_ROADMAP.md`
- Modify: `docs/VERIFICATION_MATRIX.md`
- Modify: `docs/qa/PINK_CAB_LEVEL1_CORRIDOR_ZERO.md`

**Interfaces:**
- Consumes: Jira/Confluence owner lock.
- Produces: implementation-facing mirrors and test matrix.

- [ ] Create handling authority with donor provenance, donor-vs-Tatra boundary, speed-character bands, analog controls and physical-vs-expression layer.
- [ ] Create QA acceptance contract covering steering, no ABS/ESP, analog controls, surface/body feedback, drift/recovery, 100/140/170/195 fixtures and load variants.
- [ ] Register both files in `AUTHORITY.yaml` and add `OD-TATRA-E34-FEEL-001` to `OPEN_DECISIONS.md`.
- [ ] Normalize existing mirrors so no text says residual magnetism is globally fixed at 5.0 s.
- [ ] Update verification matrix to reference the new handling QA authority.
- [ ] Fetch every changed file after writing and inspect the critical sections.

### Task 4: Contradiction sweep and branch integration readiness

**Files:** Jira/Confluence/Git search/readback; Git compare.

**Interfaces:**
- Consumes: Tasks 1–3.
- Produces: evidence-backed administrative closure state and integration-ready branch.

- [ ] Search Jira for obsolete `1470 kg`, `fixed 5.0`, `ABS`, `ESP`, `Drift Mode`, and incompatible clutch/handbrake binary wording in active PINK CAB handling owners.
- [ ] Search Confluence for fixed-five-second residue and obsolete handling wording; correct active authority hits only.
- [ ] Search/fetch Git branch authority files for the same contradictions.
- [ ] Fresh compare `main` vs `docs/pinkcab-l1-l2-admin-20260909` and record ahead/behind/file count.
- [ ] Mark this plan complete only after fresh readback proves Jira/Confluence/Git agreement.
- [ ] Do not claim runtime implementation or handling verification; only admin/spec closure.
