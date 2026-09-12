# PINK CAB · FGear Evaluation & Integration Plan

> **SUPERSEDED FOR PRODUCTION — 2026-09-12.** Owner decision moved PINK CAB vehicle physics and PS2 destruction to Unreal Engine 5.8 built-in Chaos systems. This document is retained only as historical evaluation/fallback research. No FGear/VDS purchase or integration is required. Current canonical plan: `docs/qa/PINK_CAB_NATIVE_UNREAL_CHAOS_ARCADE_SIM_PLAN.md` (`CD-785`).

Date: 2026-09-12
Program: `CD-784` — DONE / historical
Upstream completed gate: `CD-771 / PF-16`
Canonical base: `f7fa6f976fa1fa1e83acff9287e92588f0d0efce`
State: `ARCHIVED_FALLBACK_RESEARCH`

## Historical execution rules

The rules below describe the former FGear path and are no longer production-authoritative after the native Chaos pivot.

- PRE-FGEAR program is complete; do not reopen PF-00..PF-16 for new implementation.
- No FGear or VDS spend is authorized or required.
- Never extract plugin binaries/source from packaged demos and never use mirrors or pirated distributions.
- Production `main` remains free of FGear/VDS required dependencies.
- `IPinkCabVehicleDynamicsProvider` remains the game-facing boundary, now implemented by the native Chaos production path.

## Historical task pack

| Order | Jira | Deliverable | Final state |
| --- | --- | --- | --- |
| 1 | `CD-777` | Official FGear evaluation intake + license boundary | DONE / archived |
| 2 | `CD-778` | Isolated non-production Evaluation Lab | DONE / superseded |
| 3 | `CD-779` | FGear ↔ PINK CAB adapter mapping report | DONE / superseded |
| 4 | `CD-780` | Tatra FGear calibration matrix + acceptance targets | DONE / superseded |
| 5 | `CD-781` | Official plugin smoke + provider integration gate | DONE / not required |
| 6 | `CD-782` | Purchase decision + owner approval gate | DONE / NO PURCHASE |
| Deferred | `CD-783` | VDS evaluation gate after FGear integration | DONE / not required |

## Archived dependency graph

The former dependency chain was `CD-777 + CD-779 + CD-780 -> CD-781 -> CD-782`, with `CD-783` deferred after FGear integration. It is no longer active.

## Zero-cost evaluation evidence

Canonical report: `docs/qa/PINK_CAB_FGEAR_EVALUATION_LICENSE_REPORT.md`.

- Official Fab listing exposed packaged demos, documentation and an Example Project as the public zero-cost evaluation surface.
- No public seller/Fab source reviewed advertised an installable trial/evaluation plugin binary or temporary plugin license.
- Vendor-posted release evidence confirmed FGear v1.8.4 with Unreal Engine 5.7 support.
- Unreal Engine 5.8 compatibility was not publicly confirmed during evaluation.
- A vendor request was prepared as an unsent Gmail draft; it is no longer required for production progress and should remain unsent unless separately requested for research purposes.

## Provider boundary retained

The useful architectural result survives the pivot: `IPinkCabVehicleDynamicsProvider` remains the only PINK CAB-facing road-dynamics boundary. The native Chaos provider owns production vehicle physics; taxi, economy, persistence, passenger logic, Vehicle Health, UI and world generation stay outside the solver.

## Current authority

Canonical production plan: `docs/qa/PINK_CAB_NATIVE_UNREAL_CHAOS_ARCADE_SIM_PLAN.md`.
Canonical Jira program: `CD-785`.

FGear and VDS are fallback research only. This archived plan does not authorize purchase, installation or re-entry into the production critical path.
