# PF-00 · Local Development Environment Inventory

**Date:** 2026-09-11  
**Jira:** `CD-755`  
**Branch:** `feat/CD-755-pf00-bootstrap`  
**Status:** OBSERVED ENVIRONMENT ONLY — NO A01/A09 OWNER DECISION IMPLIED

## Unreal Engine installations

Observed on authorized Windows development machine:

- `C:\Program Files\Epic Games\UE_5.3`
- `C:\Program Files\Epic Games\UE_5.7`
- `C:\Program Files\Epic Games\UE_5.8`

This inventory does **not** choose the production engine version. BASE-100 owner row `A01` remains OPEN.

## Visual Studio / compiler environment

Observed:

- Visual Studio Build Tools 2022 — `17.14.29`, installation `C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools`
- Visual Studio Community 2026 — `18.9.2`, installation `C:\Program Files\Microsoft Visual Studio\18\Community`

Compiler/toolchain compatibility with the eventually owner-locked Unreal version must be verified before generating the production project.

## Windows SDK

Installed SDK library versions:

- `10.0.22621.0`
- `10.0.26100.0`

## Git / Git LFS

- Git: `2.55.0.windows.5`
- Git LFS: `3.7.1`

Repository `.gitattributes` already routes Unreal binary assets and large source media through Git LFS.

## .NET

Observed SDKs:

- `.NET SDK 8.0.101`
- `.NET SDK 10.0.400`
- MSBuild reported by current `dotnet`: `18.9.6`

## PF-00 decision boundary

The machine has multiple plausible Unreal/toolchain combinations. No production choice is made by this inventory.

Before committing `PinkCab.uproject` engine association or declaring a canonical build/package command:

1. resolve/pin `A01` exact Unreal production-version policy;
2. resolve `A09` canonical local build/package command;
3. verify the selected Unreal version against the selected MSVC/Windows SDK combination;
4. record exact commands and first executable smoke evidence.

No paid dependency was introduced by this inventory.
