# PF-00 Verification · 2026-09-11

Status: fresh executable evidence for CD-755 before closure.

## Toolchain

- Unreal Engine: 5.8.2, CL 56702186
- Canonical build entrypoint: `.\scripts\build.ps1`
- Canonical package entrypoint: `.\scripts\build.ps1 -Package`
- Compiler: MSVC 14.44.35228
- Windows SDK: 10.0.22621.0

## Fresh verification

- `PinkCabEditor Win64 Development`: PASS, exit 0.
- Automation `PinkCab.Bootstrap.ProjectIdentity`: Success, exit 0.
- Startup map `/Game/Dev/Maps/L_PinkCab_ContractZero`: loaded.
- MapCheck: 0 errors / 0 warnings.
- `scripts/check-authority.sh`: PASS.
- `git diff --check`: PASS before commit.
- Secret scan over Config/Source/uproject: 0 hits.
- Paid-vendor runtime symbol scan: 0 hits.
- Temporary vehicle-solver scan: 0 hits.
- Project `Plugins/` directory: absent.
- AndroidFileServer: explicitly disabled; no SecurityToken regeneration in fresh automation run.

## Packaging

- `BuildCookRun`: BUILD SUCCESSFUL.
- AutomationTool: exit 0.
- Cook / stage / pak / archive: completed.
- Archive contains packaged `PinkCab.exe` launcher and Win64 game executable.

No FGear/VDS purchase or runtime integration is present in PF-00.
