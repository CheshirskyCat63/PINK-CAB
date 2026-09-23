[CmdletBinding()]
param(
    [Parameter(Mandatory=$true)]
    [string[]]$Path,
    [string]$Subject = "CN=CHESHIRE DIVISION PINKCAB DEV",
    [int]$ValidityYears = 2
)

$ErrorActionPreference = "Stop"

$cert = Get-ChildItem Cert:\CurrentUser\My -CodeSigningCert -ErrorAction SilentlyContinue |
    Where-Object {
        $_.Subject -eq $Subject -and
        $_.HasPrivateKey -and
        $_.NotAfter -gt (Get-Date).AddDays(30)
    } |
    Sort-Object NotAfter -Descending |
    Select-Object -First 1

if (-not $cert) {
    $cert = New-SelfSignedCertificate `
        -Type CodeSigningCert `
        -Subject $Subject `
        -FriendlyName "CHESHIRE DIVISION PINKCAB runner dev signing" `
        -CertStoreLocation "Cert:\CurrentUser\My" `
        -KeyAlgorithm RSA `
        -KeyLength 3072 `
        -HashAlgorithm SHA256 `
        -KeyExportPolicy NonExportable `
        -NotAfter (Get-Date).AddYears($ValidityYears)
}

$cer = Join-Path ([System.IO.Path]::GetTempPath()) "pinkcab-dev-signing-$($cert.Thumbprint).cer"
Export-Certificate -Cert $cert -FilePath $cer -Force | Out-Null
try {
    foreach ($store in @("Cert:\CurrentUser\Root", "Cert:\CurrentUser\TrustedPublisher")) {
        $present = Get-ChildItem $store -ErrorAction SilentlyContinue |
            Where-Object { $_.Thumbprint -eq $cert.Thumbprint } |
            Select-Object -First 1
        if (-not $present) {
            Import-Certificate -FilePath $cer -CertStoreLocation $store | Out-Null
        }
    }
}
finally {
    Remove-Item $cer -Force -ErrorAction SilentlyContinue
}

$files = @()
foreach ($item in $Path) {
    $resolved = @(Get-ChildItem -Path $item -File -ErrorAction SilentlyContinue)
    if ($resolved.Count -eq 0 -and (Test-Path -LiteralPath $item -PathType Leaf)) {
        $resolved = @(Get-Item -LiteralPath $item)
    }
    $files += $resolved
}
$files = @($files | Sort-Object FullName -Unique)
if ($files.Count -eq 0) {
    throw "No files matched PINK-CAB dev-signing input."
}

foreach ($file in $files) {
    $signature = Set-AuthenticodeSignature `
        -FilePath $file.FullName `
        -Certificate $cert `
        -HashAlgorithm SHA256
    if ($signature.Status -ne [System.Management.Automation.SignatureStatus]::Valid) {
        throw "PINK-CAB dev-signing failed for $($file.FullName): $($signature.Status)"
    }
    Write-Host ("PINKCAB_DEV_SIGNED File={0} Status={1}" -f $file.Name, $signature.Status)
}

Write-Host ("PINKCAB_DEV_SIGNING_PASS Count={0} Subject={1} Thumbprint={2} NonExportableIdentity=YES" -f $files.Count, $cert.Subject, $cert.Thumbprint)
