# PINK CAB · Control Plane

## Canonical authority

- Repository: `CheshirskyCat63/PINK-CAB`
- Integration branch: `main`
- Accepted fallback: `accepted/p4-rig06-20261007` / `6edea77`
- Current execution lane: `Vehicle Feel 90` → then FIRST EURO R01
- Unreal Engine: UE 5.8
- Human game entry: `PINCKCAB.lnk`

There are only two active GitHub Actions entry points. Local environment state is inspected with `scripts/platform-status.ps1`; Vehicle Feel scope is enforced with `scripts/vehicle-feel-guard.ps1`:

1. `.github/workflows/verify.yml`
2. `.github/workflows/deliver.yml`

Everything named CD-648, CD-869, P00/P01/P02/P04 physics evidence, road R2/R3/R4/R5 authoring, G1 control-plane, or fast-delivery is historical evidence only.

## Verify

`verify.yml` runs:

- script/unit contracts;
- code-health;
- repository hygiene;
- exact-head `PinkCabEditor` build on the PINKCAB self-hosted Windows runner;
- focused playable runtime acceptance.

A runtime-changing PR is not mergeable by project policy until this workflow is green.

## Deliver

`deliver.yml` is manual.

It calls `scripts/deliver.ps1`, which requires a clean exact HEAD and performs:

1. editor build;
2. focused runtime acceptance;
3. BuildCookRun package;
4. packaged executable smoke;
5. atomic replacement of `PINCKCAB_BUILD`;
6. update of `PINCKCAB.lnk`;
7. `SOURCE_HEAD.txt` written into the delivered build.

Delivery is never proof of owner acceptance by itself.

## Vehicle authority

P4 uses the stock Chaos wheeled-vehicle mechanical simulation.

PINKCAB owns the gameplay/control layer above it:

- physical cockpit;
- H-pattern selection and validation;
- steering;
- pedals;
- parking brake;
- ignition/gameplay state;
- Tatra presentation;
- taxi/game systems.

The old custom Chaos simulation override, clutch causality laboratory, cadence probes, calibration fixtures and evidence stack are not active authority.

## Road authority

The active P4 world is the endless-straight map.

MetaRoad 3.2.0 is retained only as frozen free content needed by the baked road presentation. Its source/editor framework and StructUtils dependency are not part of the active P4 architecture.

## History

Research state was archived before simplification. Archive tags under `archive/20261006/*` preserve the old remote branch tips and dirty P04 research states. Historical documents remain evidence, not current operating instructions.
