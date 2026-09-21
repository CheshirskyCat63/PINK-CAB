# PINK CAB · GitHub Control Plane R1

**Authority date:** 2026-09-21  
**Owner umbrella:** CD-848  
**Canonical recovery branch:** `fix/CD-848-recovery-r1-input-contract`  
**Canonical workflow:** `.github/workflows/pinkcab-g1-github-control-plane.yml`

## Decision

PINK-CAB development execution no longer depends on Remote Desktop Commander.

GitHub is the technical control plane:
- code changes are committed to an exact Git SHA;
- GitHub Actions checks out `github.sha`, never an implicit moving branch for verification;
- the self-hosted Windows/UE 5.8 runner performs code-health, editor build, automation, packaging and packaged smoke;
- GitHub stores workflow results and exact-head evidence artifacts;
- a successful automated workflow may create an immutable runner-local build and `PINKCAB Latest.lnk` for owner testing;
- GitHub automation can report only **AUTO technical PASS**. It must always leave **HUMAN_PENDING** until the owner actually accepts the packaged build.

## Non-negotiable evidence rule

A green unit/integration/physics test proves only the layer named by that test.

Existing PlayerController automation is not described as OS/HID mouse validation. Existing direct-dynamics Chaos smoke is not described as end-to-end gameplay-input validation. The evidence artifact must retain this limitation.

No Jira, Confluence, PR, commit status or gate manifest may infer human acceptance from:
- compilation;
- code-health;
- Unreal Automation;
- direct provider/physics smoke;
- packaged launch smoke.

## Canonical execution order

1. Exact-head/LFS/clean preflight.
2. Strict code-health.
3. PinkCabEditor build.
4. Input/recovery automated matrix.
5. Full `PinkCab` regression.
6. Canonical Win64 package.
7. Packaged runtime smoke and critical log scan.
8. Immutable runner delivery.
9. GitHub evidence upload and package artifact upload.
10. Commit status `cheshire/pinkcab-g1-github`.
11. Owner human gate remains pending until explicit acceptance.

If any automated step fails, no technical PASS may be published for that SHA.

## Legacy workflows

The former owner-rejection/H-INP workflows are preserved only as manual, hard-failing tombstones so old links remain auditable but cannot silently become a second production lane.

The only active push-triggered G1 recovery workflow is `PINK CAB G1 GitHub Control Plane`.

## Tool boundary

Allowed execution/control path:

`ChatGPT GitHub connector → Git commit/PR → GitHub Actions → self-hosted UE runner → GitHub artifacts/status → owner human test`

Remote Desktop Commander is intentionally outside this path.
