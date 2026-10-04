param(
    [Parameter(Mandatory=$true)][string]$EngineRoot,
    [Parameter(Mandatory=$true)][string]$ProjectPath,
    [Parameter(Mandatory=$true)][string]$TestName,
    [Parameter(Mandatory=$true)][string]$LogPath,
    [string]$StartupMap = '/Engine/Maps/Entry',
    [ValidateRange(30, 3600)][int]$TimeoutSeconds = 300,
    [int]$TailLines = 160,
    [switch]$AllowAssetAuthoring
)

$ErrorActionPreference='Stop'

$editor=Join-Path $EngineRoot 'Engine\Binaries\Win64\UnrealEditor-Cmd.exe'
if(-not (Test-Path -LiteralPath $editor)){
    throw "PINKCAB_AUTOMATION_EDITOR_MISSING=$editor"
}
if(-not (Test-Path -LiteralPath $ProjectPath)){
    throw "PINKCAB_AUTOMATION_PROJECT_MISSING=$ProjectPath"
}

$logDir=Split-Path -Parent $LogPath
if($logDir){
    New-Item -ItemType Directory -Force -Path $logDir | Out-Null
}
Remove-Item -LiteralPath $LogPath -Force -ErrorAction SilentlyContinue

function Quote-Arg([string]$Value){
    return '"' + ($Value -replace '"','\"') + '"'
}

$arguments=@(
    (Quote-Arg $ProjectPath),
    $StartupMap,
    '-unattended',
    '-NullRHI',
    '-nosplash',
    '-nopause',
    '-NoSound',
    '-SkipAssetScan'
)
if($AllowAssetAuthoring){
    $arguments += '-PinkCabAllowAssetAuthoring'
}
$arguments += @(
    (Quote-Arg "-abslog=$LogPath"),
    (Quote-Arg "-ExecCmds=Automation RunTests $TestName"),
    (Quote-Arg '-TestExit=Automation Test Queue Empty')
)
$argumentLine=$arguments -join ' '

$process=New-Object System.Diagnostics.Process
$process.StartInfo.FileName=$editor
$process.StartInfo.Arguments=$argumentLine
$process.StartInfo.UseShellExecute=$false
$process.StartInfo.CreateNoWindow=$true

Write-Host "PINKCAB_AUTOMATION_START test=$TestName timeout_s=$TimeoutSeconds map=$StartupMap asset_authoring=$([int]$AllowAssetAuthoring.IsPresent)"
if(-not $process.Start()){
    throw "PINKCAB_AUTOMATION_START_FAILED=$TestName"
}

$timer=[Diagnostics.Stopwatch]::StartNew()
$completed=$false
while(-not $completed -and $timer.Elapsed.TotalSeconds -lt $TimeoutSeconds){
    $remainingMs=[Math]::Max(1, [int](($TimeoutSeconds - $timer.Elapsed.TotalSeconds) * 1000))
    $completed=$process.WaitForExit([Math]::Min(30000, $remainingMs))
    if(-not $completed){
        $progress='waiting for automation log'
        if(Test-Path -LiteralPath $LogPath){
            $lastEvent=Get-Content -LiteralPath $LogPath -Tail 200 |
                Select-String -Pattern 'Test Started.|Test Completed.|P02_D3_MATRIX|Automation Test Queue Empty' |
                Select-Object -Last 1
            if($lastEvent){ $progress=$lastEvent.Line }
        }
        Write-Host "PINKCAB_AUTOMATION_PROGRESS test=$TestName elapsed_s=$([int]$timer.Elapsed.TotalSeconds) $progress"
    }
}
$timer.Stop()
if(-not $completed){
    try { $process.Kill() } catch {}
    try { $process.WaitForExit(10000) | Out-Null } catch {}
    if(Test-Path -LiteralPath $LogPath){
        Get-Content -LiteralPath $LogPath |
            Select-Object -Last $TailLines |
            ForEach-Object { Write-Host $_ }
    }
    throw "PINKCAB_AUTOMATION_TIMEOUT test=$TestName timeout_s=$TimeoutSeconds"
}

$exitCode=$process.ExitCode
if(Test-Path -LiteralPath $LogPath){
    Get-Content -LiteralPath $LogPath |
        Select-String -Pattern 'Automation Test Queue Empty|Test Completed. Result=|LogAutomationController: Error:|P01_IDLE|P01_SLOPE|P01_FLAT_COAST|P02_D3_MATRIX|P02_PHY009_' |
        Select-Object -Last $TailLines |
        ForEach-Object { Write-Host $_.Line }
}

if($exitCode -ne 0){
    throw "PINKCAB_AUTOMATION_PROCESS_FAIL test=$TestName exit=$exitCode"
}
if(-not (Test-Path -LiteralPath $LogPath)){
    throw "PINKCAB_AUTOMATION_LOG_MISSING=$TestName"
}

$queue=@(Select-String -LiteralPath $LogPath -Pattern 'Automation Test Queue Empty [0-9]+ tests performed')
$fail=@(Select-String -LiteralPath $LogPath -Pattern 'Test Completed\. Result=\{Fail\}|LogAutomationController: Error:')
$assetFail=@(Select-String -LiteralPath $LogPath -Pattern "Failed to load package|Can't find file '/Game|Unable to load package|Failed to load '/Game")

if($queue.Count -eq 0){
    throw "PINKCAB_AUTOMATION_QUEUE_MISSING=$TestName"
}
$queueCount=[regex]::Match($queue[-1].Line, 'Automation Test Queue Empty ([0-9]+) tests performed')
if(-not $queueCount.Success -or [int]$queueCount.Groups[1].Value -le 0){
    throw "PINKCAB_AUTOMATION_NO_TESTS=$TestName"
}
if($fail.Count -gt 0){
    Write-Host "PINKCAB_AUTOMATION_FAILURE_LINES_BEGIN test=$TestName count=$($fail.Count)"
    foreach($match in $fail){
        Write-Host "PINKCAB_AUTOMATION_FAILURE_LINE=$($match.Line)"
    }
    Write-Host "PINKCAB_AUTOMATION_FAILURE_LINES_END test=$TestName"
    throw "PINKCAB_AUTOMATION_TEST_FAIL test=$TestName fail=$($fail.Count)"
}
if($assetFail.Count -gt 0){
    Write-Host "PINKCAB_AUTOMATION_ASSET_FAILURE_LINES_BEGIN test=$TestName count=$($assetFail.Count)"
    foreach($match in $assetFail){
        Write-Host "PINKCAB_AUTOMATION_ASSET_FAILURE_LINE=$($match.Line)"
    }
    Write-Host "PINKCAB_AUTOMATION_ASSET_FAILURE_LINES_END test=$TestName"
    throw "PINKCAB_AUTOMATION_ASSET_FAIL test=$TestName asset_fail=$($assetFail.Count)"
}

Write-Host "PINKCAB_AUTOMATION_PASS test=$TestName $($queue[-1].Line)"
