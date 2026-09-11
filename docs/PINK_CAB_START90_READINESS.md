# PINK CAB · START-90 / BASE-100 Readiness Snapshot

**Status:** CURRENT CODE-ONLY SPECIFICATION READINESS
**Snapshot:** 2026-09-10
**Jira:** `CD-660`, governance `CD-661`, owner register `CD-673`
**Confluence:** `6553617`
**Scope:** `CD-753`
**Source census:** `docs/PINK_CAB_BASE100_TECH_OWNER_PACK_01.md`

## Meaning

This score measures implementation-facing specification readiness only. It does not measure art completeness, code implementation or runtime verification.

`CANON → SPECIFIED → IMPLEMENTED → VERIFIED`

FIRST EURO is the first 12 months / PC / single-player / full L1+L2 product. Post-year multiplayer, L3 gameplay, lifestyle ServiceNodes, full Taxi Regulator and daily insurance are excluded except minimal extension boundaries.

## Reproducible row scoring

| Status | Value |
| --- | ---: |
| LOCKED | 1.0 |
| CALIBRATION | 1.0 |
| PROPOSED DEFAULT | 0.5 |
| OPEN | 0.0 |

Reason: a proposed default has implementation shape but is not owner authority; it therefore cannot count as a fully closed decision.

## Fresh owner-pack census

- LOCKED: **44**
- CALIBRATION: **3**
- PROPOSED DEFAULT: **88**
- OPEN: **61**
- TOTAL: **196**

Current score: `44 + 3 + 88×0.5 = 91`; `91 / 196 = 46.4%`.

**START-90: FAIL. Gap: 43.6 percentage points.**

## Domain snapshot

| Domain | Pack sections | Rows | Score |
| --- | --- | ---: | ---: |
| CORE | A/B/C/Q/R/S | 55 | **48.2%** |
| VEHICLE | D/E/M/N | 35 | **68.6%** |
| TAXI | F/G | 30 | **38.3%** |
| STATE | H/I | 21 | **23.8%** |
| WORLD | J/K/L | 32 | **34.4%** |
| SERVICE | O/P | 23 | **56.5%** |
| SCOPE | `CD-753` | separate lock | **100% LOCKED** |

SCOPE is reported separately so a locked delivery boundary cannot hide weak runtime domains.

## Exact path to START-90

If all 88 PROPOSED DEFAULT rows are owner-accepted, score becomes `135 / 196 = 68.9%`. Closing at least 42 of the remaining 61 OPEN rows then yields `177 / 196 = 90.31%` and crosses START-90. This is a mathematical minimum, not permission to ignore structural priority.

## Current genuine OPEN owner rows

`A01 A03 A07 A09 A10 A13 A14 A15 C08 F04 F05 F06 F08 F11 F12 F13 F14 F15 F16 F17 F18 F19 G02 G04 H04 H05 I01 I02 I03 I04 I06 I08 I09 I11 I12 I13 J05 J07 J09 J11 J12 K04 K06 K08 K11 K12 K13 L05 L06 L07 M05 N05 N06 N07 N09 O02 O03 P03 P05 P09 P11`

Highest structural priority: runtime/version/build/platform (`A`), session/save/recovery (`I`), CityCode/streaming (`J`), traffic/geometry (`K`), rule numerics (`L`), then remaining taxi/service/transit owner rows.

## Administrative vs design closure

Administrative contradiction cleanup may be complete while START-90 remains failed. An OPEN row is not an administrative defect; it is a legitimate unresolved owner decision. No OPEN or PROPOSED DEFAULT status may be silently upgraded to LOCKED to improve the score.

Historical ≈59% and 38.2% readiness figures are retired because they used older denominators.

## Runtime truth

Broad production remains HOLD while START-90 fails. Narrow bootstrap/specification/calibration work is allowed where it closes known contracts without inventing owner decisions.

No documentation score establishes runtime implementation. PINK CAB remains **NOT IMPLEMENTED / NOT VERIFIED** until exact executable build/commit/plugin/config/seed evidence exists.
