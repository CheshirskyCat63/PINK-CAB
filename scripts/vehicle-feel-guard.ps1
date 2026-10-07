[CmdletBinding()]
param([string]$Baseline = 'accepted/p4-rig06-20261007')

$ErrorActionPreference='Stop'
$RepoRoot=Split-Path -Parent $PSScriptRoot
Set-Location $RepoRoot

git rev-parse --verify "$Baseline^{commit}" *> $null
if($LASTEXITCODE -ne 0){ throw "PINKCAB_VF90_BASELINE_MISSING=$Baseline" }

$changed=@()
$changed += @(git diff --name-only "$Baseline..HEAD")
$changed += @(git diff --name-only)
$changed += @(git diff --name-only --cached)
$changed = @($changed | Where-Object {$_} | Sort-Object -Unique)

$forbiddenPrefixes=@(
    'Source/PinkCabWorld/',
    'Source/PinkCabTraffic/',
    'Source/PinkCabTaxi/',
    'Source/PinkCabEconomy/',
    'Source/PinkCabPersistence/',
    'Content/Dev/World/',
    'Content/Game/City/',
    'Content/Game/Traffic/',
    'Content/Game/Passenger/',
    'Content/Game/Services/'
)

$violations=@()
foreach($path in $changed){
    foreach($prefix in $forbiddenPrefixes){
        if($path.StartsWith($prefix,[StringComparison]::OrdinalIgnoreCase)){
            $violations += $path
            break
        }
    }
}

if($violations.Count -gt 0){
    $violations | ForEach-Object { Write-Host "FORBIDDEN_VF90_CHANGE=$_" -ForegroundColor Red }
    throw 'PINKCAB_VEHICLE_FEEL_90_SCOPE_FAIL'
}

Write-Host "PINKCAB_VEHICLE_FEEL_90_SCOPE=PASS baseline=$Baseline changed=$($changed.Count)"
