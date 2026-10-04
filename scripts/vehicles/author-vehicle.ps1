param(
    [Parameter(Mandatory=$true)][string]$ManifestPath,
    [Parameter(Mandatory=$true)][string]$ReceiptPath,
    [string]$EngineRoot = 'C:\Program Files\Epic Games\UE_5.8',
    [string]$ProjectPath = (Join-Path (Split-Path $PSScriptRoot -Parent | Split-Path -Parent) 'PinkCab.uproject'),
    [string]$StudioRoot = ''
)

$ErrorActionPreference='Stop'
$editor=Join-Path $EngineRoot 'Engine\Binaries\Win64\UnrealEditor-Cmd.exe'
$author=Join-Path $PSScriptRoot 'author_vehicle.py'
if(-not (Test-Path -LiteralPath $editor)){ throw "PINKCAB_VEHICLE_EDITOR_MISSING=$editor" }
if(-not (Test-Path -LiteralPath $ProjectPath)){ throw "PINKCAB_VEHICLE_PROJECT_MISSING=$ProjectPath" }
if(-not (Test-Path -LiteralPath $ManifestPath)){ throw "PINKCAB_VEHICLE_MANIFEST_MISSING=$ManifestPath" }
if(-not (Test-Path -LiteralPath $author)){ throw "PINKCAB_VEHICLE_AUTHOR_MISSING=$author" }

if(-not $StudioRoot){
    $cursor=Split-Path -Parent (Resolve-Path -LiteralPath $ProjectPath).Path
    while($cursor -and (Split-Path $cursor -Leaf) -ne 'CHESHIRE_DIVISION'){
        $parent=Split-Path -Parent $cursor
        if($parent -eq $cursor){ break }
        $cursor=$parent
    }
    if($cursor -and (Split-Path $cursor -Leaf) -eq 'CHESHIRE_DIVISION'){
        $StudioRoot=$cursor
    }
}
$env:PINKCAB_VEHICLE_MANIFEST=(Resolve-Path -LiteralPath $ManifestPath).Path
$env:PINKCAB_VEHICLE_RECEIPT=[IO.Path]::GetFullPath($ReceiptPath)
$env:PINKCAB_STUDIO_ROOT=$StudioRoot
$env:TMP=$env:TEMP

& $editor $ProjectPath -run=pythonscript "-script=$author" -unattended -NullRHI -nosplash -NoSound
if($LASTEXITCODE -ne 0){ throw "PINKCAB_VEHICLE_AUTHOR_FAILED=$LASTEXITCODE" }
if(-not (Test-Path -LiteralPath $env:PINKCAB_VEHICLE_RECEIPT)){
    throw "PINKCAB_VEHICLE_RECEIPT_MISSING=$env:PINKCAB_VEHICLE_RECEIPT"
}
Write-Host "PINKCAB_VEHICLE_AUTHOR_WRAPPER=PASS manifest=$env:PINKCAB_VEHICLE_MANIFEST receipt=$env:PINKCAB_VEHICLE_RECEIPT"
