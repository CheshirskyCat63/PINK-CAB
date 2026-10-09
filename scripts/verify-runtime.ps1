[CmdletBinding()]
param([string]$EngineRoot = $env:PINKCAB_UE_ROOT)

$ErrorActionPreference='Stop'
$RepoRoot=Split-Path -Parent $PSScriptRoot
if([string]::IsNullOrWhiteSpace($EngineRoot)){ $EngineRoot='C:\Program Files\Epic Games\UE_5.8' }
$Project=Join-Path $RepoRoot 'PinkCab.uproject'
$LogDir=Join-Path $RepoRoot 'Saved\FocusedAcceptance'

$tests=@(
    'PinkCab.Vehicle.AssetContract.Tatra613V12',
    'PinkCab.Vehicle.Visual.Tatra613ScenePreservedProfile',
    'PinkCab.Vehicle.ChaosBaseline.PhysicsOnly.DriveSmoke',
    'PinkCab.Vehicle.ChaosBaseline.PhysicsOnly.ReverseDriveSmoke',
    'PinkCab.Vehicle.LiveState.RuntimeRoundTrip',
    'PinkCab.Vehicle.HGate.DeliberateCenterEntry',
    'PinkCab.Cockpit.Playable.Runtime',
    'PinkCab.World.L1EndlessRoad.Runtime.ChunkBinding',
    'PinkCab.World.L1EndlessRoad.Physics.SeamCrossing',
    # Task5: physical output and migration proof must travel with every future candidate.
    'PinkCab.Vehicle.PhysicalFoundation.AuthoredRigContract',
    'PinkCab.Vehicle.PhysicalFoundation.BaselineAudit',
    'PinkCab.Vehicle.PhysicalFoundation.ReferenceMassMoment',
    'PinkCab.Vehicle.PhysicalFoundation.CrewMassMoment',
    'PinkCab.Vehicle.PhysicalFoundation.ThreeAxisMassMoment',
    'PinkCab.Vehicle.PhysicalFoundation.MassInertiaCombination',
    'PinkCab.Vehicle.PhysicalFoundation.MassCoordinatePersistence',
    'PinkCab.Vehicle.PhysicalFoundation.LegacyCoordinateMigration',
    'PinkCab.Vehicle.PhysicalFoundation.InvalidMassCoordinates',
    'PinkCab.Vehicle.PhysicalFoundation.LiveLoadEnvelope',
    'PinkCab.Vehicle.Visual.TatraWheelContacts',
    # Task6 bounded disconnect/handbrake proof; does not certify partial-clutch actuation.
    'PinkCab.Vehicle.Actuation.DrivelineDisconnect',
    'PinkCab.Vehicle.Actuation.DisconnectedCoast',
    'PinkCab.Vehicle.Actuation.AnalogHandbrakeTorque'
)

$filter=$tests -join '+'
$log=Join-Path $LogDir 'focused-runtime.log'
& pwsh -NoProfile -File (Join-Path $RepoRoot 'scripts\ci\run-unreal-automation.ps1') -EngineRoot $EngineRoot -ProjectPath $Project -TestName $filter -LogPath $log -TimeoutSeconds 900
if($LASTEXITCODE -ne 0){ exit $LASTEXITCODE }

$queue=Select-String -LiteralPath $log -Pattern 'Automation Test Queue Empty ([0-9]+) tests performed' | Select-Object -Last 1
if(-not $queue){ throw 'PINKCAB_FOCUSED_RUNTIME_QUEUE_MISSING' }
$match=[regex]::Match($queue.Line, 'Automation Test Queue Empty ([0-9]+) tests performed')
if(-not $match.Success -or [int]$match.Groups[1].Value -ne $tests.Count){
    throw "PINKCAB_FOCUSED_RUNTIME_COUNT_FAIL expected=$($tests.Count) actual=$($match.Groups[1].Value)"
}
Write-Host "PINKCAB_FOCUSED_RUNTIME=PASS tests=$($tests.Count)"
