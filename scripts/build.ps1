[CmdletBinding()]
param(
    [switch]$Package,
    [switch]$GenerateProjectFiles,
    [string]$EngineRoot = $env:PINKCAB_UE_ROOT
)

$ErrorActionPreference = "Stop"

# UE 5.8 Build.bat uses %TMP% for its process lock. Some agent shells only expose TEMP.
if ([string]::IsNullOrWhiteSpace($env:TMP)) {
    if ([string]::IsNullOrWhiteSpace($env:TEMP)) {
        throw "Neither TMP nor TEMP is configured; Unreal Build.bat cannot create its lock file."
    }
    $env:TMP = $env:TEMP
}
$RepoRoot = Split-Path -Parent $PSScriptRoot
$Project = Join-Path $RepoRoot "PinkCab.uproject"

if ([string]::IsNullOrWhiteSpace($EngineRoot)) {
    $EngineRoot = "C:\Program Files\Epic Games\UE_5.8"
}

$BuildVersion = Join-Path $EngineRoot "Engine\Build\Build.version"
$BuildBat = Join-Path $EngineRoot "Engine\Build\BatchFiles\Build.bat"
$ProjectFilesBat = Join-Path $EngineRoot "Engine\Build\BatchFiles\GenerateProjectFiles.bat"
$RunUAT = Join-Path $EngineRoot "Engine\Build\BatchFiles\RunUAT.bat"

foreach ($Required in @($Project, $BuildVersion, $BuildBat, $RunUAT)) {
    if (-not (Test-Path $Required)) { throw "Required path missing: $Required" }
}

$Version = Get-Content $BuildVersion -Raw | ConvertFrom-Json
if ($Version.MajorVersion -ne 5 -or $Version.MinorVersion -ne 8) {
    throw "A01 requires Unreal Engine 5.8; found $($Version.MajorVersion).$($Version.MinorVersion).$($Version.PatchVersion) at $EngineRoot"
}

Write-Host "PINK CAB build authority: UE $($Version.MajorVersion).$($Version.MinorVersion).$($Version.PatchVersion) CL $($Version.Changelist)"
Write-Host "Project: $Project"

if ($GenerateProjectFiles) {
    if (-not (Test-Path $ProjectFilesBat)) { throw "GenerateProjectFiles.bat missing: $ProjectFilesBat" }
    & $ProjectFilesBat "-project=$Project" -game -engine
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
}

if ($Package) {
    $ArchiveDir = Join-Path $RepoRoot "Artifacts\Package"
    & $RunUAT -WaitForUATMutex BuildCookRun "-project=$Project" -noP4 -platform=Win64 -clientconfig=Development -build -cook -stage -pak -archive "-archivedirectory=$ArchiveDir"
    exit $LASTEXITCODE
}

& $BuildBat PinkCabEditor Win64 Development $Project -WaitMutex -NoHotReloadFromIDE
exit $LASTEXITCODE
