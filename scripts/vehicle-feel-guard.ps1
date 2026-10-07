[CmdletBinding()]
param([string]$Baseline = 'accepted/p4-rig06-20261007')

$ErrorActionPreference='Stop'
$RepoRoot=Split-Path -Parent $PSScriptRoot
Set-Location $RepoRoot

git rev-parse --verify --quiet "$Baseline^{commit}" *> $null
if($LASTEXITCODE -ne 0){ throw "PINKCAB_VF90_BASELINE_MISSING=$Baseline" }

function Get-ChangedPaths([string[]]$GitArguments){
    $paths = @(& git -c core.quotepath=false @GitArguments)
    if($LASTEXITCODE -ne 0){ throw "PINKCAB_VF90_GIT_FAILED=$($GitArguments -join ' ')" }
    return $paths
}

$changed=@()
$changed += @(Get-ChangedPaths @('diff','--name-only','--no-renames',"$Baseline..HEAD"))
$changed += @(Get-ChangedPaths @('diff','--name-only','--no-renames'))
$changed += @(Get-ChangedPaths @('diff','--name-only','--no-renames','--cached'))
$changed += @(Get-ChangedPaths @('ls-files','--others','--exclude-standard'))
$changed = @($changed | Where-Object {$_} | Sort-Object -Unique)

$forbiddenPrefixes=@(
    'Source/PinkCabWorld/',
    'Source/PinkCabTraffic/',
    'Source/PinkCabTaxi/',
    'Source/PinkCabEconomy/',
    'Source/PinkCabPersistence/',
    'Source/PinkCab/Private/World/',
    'Source/PinkCab/Public/World/',
    'Source/PinkCab/Private/Service/',
    'Source/PinkCab/Public/Service/',
    'Source/PinkCab/Private/Persistence/',
    'Source/PinkCab/Public/Persistence/',
    'Source/PinkCab/Private/Taxi/',
    'Source/PinkCab/Public/Taxi/',
    'Source/PinkCab/Private/Traffic/',
    'Source/PinkCab/Public/Traffic/',
    'Source/PinkCab/Private/Economy/',
    'Source/PinkCab/Public/Economy/',
    'Content/World/',
    'Content/Dev/Maps/',
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
