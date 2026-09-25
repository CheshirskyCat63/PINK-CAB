# PINK-CAB CD-869 · R4 Road Visual v2 Implementation Plan

> **For agentic workers:** execute inline under TDD; R3 is immutable.

**Goal:** Replace the flat constant-only Level 1 asphalt with a project-owned worn/wet visual material while preserving accepted R1/R2/R3 geometry, markings, streaming, controls and physics.

**Architecture:** MetaRoad 3.2 remains editor/bake reference only. Runtime road content must resolve exclusively to /Game-owned material assets. R4 is presentation-only; R5/R6 physical surface and tire calibration remain frozen.

**Tech Stack:** Unreal Engine 5.8.2, C++ automation tests, MetaRoad Free 3.2.0 editor plugin, GitHub Actions self-hosted Windows runner.

**Spec:** docs/superpowers/specs/2026-09-24-l1-road-v2-human-gates.md

## Global Constraints

- R3 integration remains 55e9fd8481a114265fd43f1c6433d86df53ba991.
- 5 express lanes/direction × 4.50 m; 2 local lanes/direction × 4.00 m.
- 79.0 m design envelope; 79.15 m physical runtime assembly.
- 32 native curbs; 50 native RoadMarks; M_PC_L1_Mark remains exact.
- No geometry, collision, RoadGraph, controls, H-gate, physical material, friction or tire changes.
- No packaged runtime dependency on /MetaRoad/.
- R4 target: dark graphite worn asphalt, macro colour/roughness breakup, subtle wetness, no mirror road, no obvious kilometre tiling.

## Tasks

- [ ] T1 — RED contract + MetaRoad asphalt inventory: prove current M_PC_L1_Asphalt is constant-only and inspect MetaRoad M_Asphalt dependencies/graph on the runner.
- [ ] T2 — Implement project-owned Road Visual v2 material and regenerate only allowed road visual assets.
- [ ] T3 — GREEN R4 test plus frozen R2/R3/endless/materialization/H-gate regressions.
- [ ] T4 — Fresh Win64 package, packaged smoke/input, install desktop shortcut and foreground HUMAN launch.
- [ ] T5 — Independent branch diff review and HUMAN_PENDING evidence in Jira; no merge before owner acceptance.

## HUMAN R4 Gate

Check only: asphalt reads as worn asphalt; near grain is readable without shimmer; macro breakup hides repetitive tiling; wet response is subtle and non-mirror; no 1000 m seam/pop; R3 markings/curbs/access openings unchanged; controls/H-gate unchanged.
