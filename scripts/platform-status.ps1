[CmdletBinding()]
param([switch]$Strict)

$ErrorActionPreference = 'Stop'
$RepoRoot = Split-Path -Parent $PSScriptRoot
Set-Location $RepoRoot

$AcceptedTag = 'accepted/p4-rig06-20261007'
$ExpectedWorkflows = @('deliver.yml','verify.yml')
$EngineRoot = if($env:PINKCAB_UE_ROOT){$env:PINKCAB_UE_ROOT}else{'C:\Program Files\Epic Games\UE_5.8'}

$Head = (git rev-parse HEAD).Trim()
$Branch = (git branch --show-current).Trim()
$OriginMain = (git rev-parse origin/main).Trim()
$AcceptedHead = (git rev-list -n 1 $AcceptedTag 2>$null).Trim()
$Ahead = [int]((git rev-list --count "origin/main..HEAD").Trim())
$Behind = [int]((git rev-list --count "HEAD..origin/main").Trim())
$Dirty = @(git status --porcelain)
$Worktrees = @((git worktree list --porcelain) | Select-String '^worktree ' | ForEach-Object {$_.Line.Substring(9)})
$WorkflowNames = @(Get-ChildItem (Join-Path $RepoRoot '.github\workflows') -File | Select-Object -ExpandProperty Name | Sort-Object)
$WorkflowDrift = @($WorkflowNames | Where-Object {$_ -notin $ExpectedWorkflows}) + @($ExpectedWorkflows | Where-Object {$_ -notin $WorkflowNames})

$DeliveredHeadFile = Join-Path $env:USERPROFILE 'Desktop\PINCKCAB_BUILD\SOURCE_HEAD.txt'
$DeliveredHead = if(Test-Path $DeliveredHeadFile){(Get-Content $DeliveredHeadFile -Raw).Trim()}else{'MISSING'}

$BuildVersion = Join-Path $EngineRoot 'Engine\Build\Build.version'
$EngineVersion = 'UNKNOWN'
if(Test-Path $BuildVersion){
    $j = Get-Content $BuildVersion -Raw | ConvertFrom-Json
    $EngineVersion = "$($j.MajorVersion).$($j.MinorVersion).$($j.PatchVersion) CL $($j.Changelist)"
}

$MetaRoadVersion = 'UNKNOWN'
$MetaRoadUplugin = Join-Path $RepoRoot 'Plugins\MetaRoad\MetaRoad.uplugin'
if(Test-Path $MetaRoadUplugin){
    $m = Get-Content $MetaRoadUplugin -Raw | ConvertFrom-Json
    $MetaRoadVersion = $m.VersionName
}

$LfsVersion = (& git lfs version 2>$null)
$Runner = @(Get-Process -Name 'Runner.Listener','Runner.Worker' -ErrorAction SilentlyContinue)
$PinkCab = @(Get-Process PinkCab -ErrorAction SilentlyContinue)
$Unreal = @(Get-Process UnrealEditor,UnrealEditor-Cmd -ErrorAction SilentlyContinue)
$Zen = @(Get-Process zenserver -ErrorAction SilentlyContinue)
$DriveName = ([IO.Path]::GetPathRoot($RepoRoot)).Substring(0,1)
$Drive = Get-PSDrive -Name $DriveName

$VehicleCoreDrift = @()
if($AcceptedHead){
    $VehicleCoreDrift = @(git diff --name-only "$AcceptedTag..HEAD" -- Source/PinkCabVehicle Source/PinkCabInteraction)
}

$Checks = [ordered]@{
    RepoExists = Test-Path (Join-Path $RepoRoot 'PinkCab.uproject')
    AcceptedTagExists = -not [string]::IsNullOrWhiteSpace($AcceptedHead)
    UEExists = Test-Path (Join-Path $EngineRoot 'Engine\Binaries\Win64\UnrealEditor.exe')
    CanonicalWorkflowsOnly = $WorkflowDrift.Count -eq 0
    SingleWorktree = $Worktrees.Count -eq 1
    GitLfsAvailable = [bool]$LfsVersion
    DeliverScriptExists = Test-Path (Join-Path $RepoRoot 'scripts\deliver.ps1')
    VerifyScriptExists = Test-Path (Join-Path $RepoRoot 'scripts\verify-runtime.ps1')
}

[ordered]@{
    Repo = $RepoRoot
    Branch = $Branch
    Head = $Head
    OriginMain = $OriginMain
    AheadOfOriginMain = $Ahead
    BehindOriginMain = $Behind
    DirtyEntries = $Dirty.Count
    AcceptedTag = $AcceptedTag
    AcceptedHead = $AcceptedHead
    VehicleCoreFilesChangedSinceAcceptedTag = $VehicleCoreDrift.Count
    DeliveredHead = $DeliveredHead
    DeliveredMatchesHead = $DeliveredHead -eq $Head
    DeliveredMatchesAcceptedFallback = $DeliveredHead -eq $AcceptedHead
    Engine = $EngineVersion
    MetaRoad = $MetaRoadVersion
    GitLfs = $LfsVersion
    WorktreeCount = $Worktrees.Count
    WorkflowNames = $WorkflowNames
    RunningPinkCab = $PinkCab.Count
    RunningUnreal = $Unreal.Count
    RunningRunner = $Runner.Count
    RunningZen = $Zen.Count
    RepoDriveFreeGB = [math]::Round($Drive.Free / 1GB, 1)
    Checks = $Checks
} | ConvertTo-Json -Depth 5

$Failed = @($Checks.GetEnumerator() | Where-Object {-not $_.Value})
if($Failed.Count -gt 0){
    Write-Host "PINKCAB_PLATFORM_STATUS=FAIL failed=$($Failed.Name -join ',')" -ForegroundColor Red
    if($Strict){ exit 2 }
}else{
    Write-Host 'PINKCAB_PLATFORM_STATUS=PASS' -ForegroundColor Green
}
