param(
    [string]$Tag = ""
)

$ErrorActionPreference = "Stop"

$repoRoot = (Resolve-Path "$PSScriptRoot\..").Path
$releasesDir = Join-Path $repoRoot "releases"

if (-not (Test-Path $releasesDir)) {
    New-Item -ItemType Directory -Path $releasesDir | Out-Null
}

if (-not $Tag) {
    $dateStamp = Get-Date -Format "yyyyMMdd"
    $Tag = "v1.18.0-$dateStamp"
}

Write-Host "==========================================" -ForegroundColor Cyan
Write-Host " Packaging MeshCore Releases: $Tag" -ForegroundColor Cyan
Write-Host "==========================================" -ForegroundColor Cyan

# Package Companion Radio
$companionZipName = "m5stack-unit-c6l-companion-$Tag.zip"
$companionZipPath = Join-Path $releasesDir $companionZipName
$companionTempDir = Join-Path $env:TEMP "meshcore-companion-$Tag"

if (Test-Path $companionTempDir) { Remove-Item -Recurse -Force $companionTempDir }
New-Item -ItemType Directory -Path $companionTempDir | Out-Null

Copy-Item "$repoRoot\firmware\01-companion-radio-ble\*" $companionTempDir -Force
Copy-Item "$repoRoot\scripts\flash-companion.bat" $companionTempDir -Force
Copy-Item "$repoRoot\scripts\flash-companion.ps1" $companionTempDir -Force
Copy-Item "$repoRoot\scripts\flash-companion-clean.bat" $companionTempDir -Force
Copy-Item "$repoRoot\scripts\erase-flash.bat" $companionTempDir -Force
Copy-Item "$repoRoot\scripts\erase-flash.ps1" $companionTempDir -Force
Copy-Item "$repoRoot\scripts\monitor-serial.bat" $companionTempDir -Force

if (Test-Path $companionZipPath) { Remove-Item -Force $companionZipPath }
Compress-Archive -Path "$companionTempDir\*" -DestinationPath $companionZipPath -CompressionLevel Optimal
Remove-Item -Recurse -Force $companionTempDir

Write-Host "[OK] Created Companion release bundle: $companionZipPath" -ForegroundColor Green

# Package Repeater
$repeaterZipName = "m5stack-unit-c6l-repeater-$Tag.zip"
$repeaterZipPath = Join-Path $releasesDir $repeaterZipName
$repeaterTempDir = Join-Path $env:TEMP "meshcore-repeater-$Tag"

if (Test-Path $repeaterTempDir) { Remove-Item -Recurse -Force $repeaterTempDir }
New-Item -ItemType Directory -Path $repeaterTempDir | Out-Null

Copy-Item "$repoRoot\firmware\02-repeater\*" $repeaterTempDir -Force
Copy-Item "$repoRoot\scripts\flash-repeater.bat" $repeaterTempDir -Force
Copy-Item "$repoRoot\scripts\flash-repeater.ps1" $repeaterTempDir -Force
Copy-Item "$repoRoot\scripts\flash-repeater-clean.bat" $repeaterTempDir -Force
Copy-Item "$repoRoot\scripts\erase-flash.bat" $repeaterTempDir -Force
Copy-Item "$repoRoot\scripts\erase-flash.ps1" $repeaterTempDir -Force
Copy-Item "$repoRoot\scripts\monitor-serial.bat" $repeaterTempDir -Force
Copy-Item "$repoRoot\docs\TROUBLESHOOTING.md" $repeaterTempDir -Force

if (Test-Path $repeaterZipPath) { Remove-Item -Force $repeaterZipPath }
Compress-Archive -Path "$repeaterTempDir\*" -DestinationPath $repeaterZipPath -CompressionLevel Optimal
Remove-Item -Recurse -Force $repeaterTempDir

Write-Host "[OK] Created Repeater release bundle: $repeaterZipPath" -ForegroundColor Green

Get-Item $companionZipPath, $repeaterZipPath | Select-Object Name, Length, LastWriteTime | Format-Table -AutoSize
