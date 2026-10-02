param([string]$RunnerRoot='E:\CHESHIRE_DIVISION\GitHubRunner\PINK-CAB\actions-runner')
$ErrorActionPreference='Stop'
$registration=Get-Content -LiteralPath (Join-Path $RunnerRoot '.runner') -Raw | ConvertFrom-Json
if($registration.gitHubUrl -ne 'https://github.com/CheshirskyCat63/PINK-CAB' -or $registration.agentName -ne 'DESKTOP-C7VAU4V-PINKCAB'){
    throw 'PINKCAB_RUNNER_IDENTITY_MISMATCH'
}
$listener=Join-Path $RunnerRoot 'bin\Runner.Listener.exe'
$active=@(Get-Process -Name Runner.Listener -ErrorAction SilentlyContinue | Where-Object { $_.Path -eq $listener })
if($active.Count){Write-Output 'PINKCAB_RUNNER_ALREADY_RUNNING'; exit 0}
$process=Start-Process -FilePath $listener -ArgumentList 'run' -WorkingDirectory $RunnerRoot -WindowStyle Hidden -PassThru
Write-Output "PINKCAB_RUNNER_STARTED pid=$($process.Id)"
