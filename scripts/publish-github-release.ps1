param(
    [string]$Tag = "",
    [string]$Title = "",
    [string]$Notes = ""
)

$ErrorActionPreference = "Stop"

$repoRoot = (Resolve-Path "$PSScriptRoot\..").Path

if (-not $Tag) {
    $dateStamp = Get-Date -Format "yyyyMMdd"
    $Tag = "v1.18.0-$dateStamp"
}

if (-not $Title) {
    $Title = "MeshCore 1.18 — Release $Tag"
}

# Run packaging script first
& "$PSScriptRoot\package-release.ps1" -Tag $Tag

$releasesDir = Join-Path $repoRoot "releases"
$companionZip = Join-Path $releasesDir "m5stack-unit-c6l-companion-$Tag.zip"
$repeaterZip = Join-Path $releasesDir "m5stack-unit-c6l-repeater-$Tag.zip"

if (-not (Test-Path $companionZip) -or -not (Test-Path $repeaterZip)) {
    Write-Error "Release archives could not be found."
}

if (-not $Notes) {
    $Notes = @"
## MeshCore 1.18 Firmware Release ($Tag)

Precompiled binaries and standalone flashing packages for **M5Stack Unit C6L** (ESP32-C6 + SX1262 LoRa).

### Included Bundles / Dołączone Paczki
1. **Companion Radio (BLE + Wi-Fi)**:
   - Dynamic 6-digit random PIN pairing on OLED line 2.
   - 15s display auto-off with button wake & silent connection mode.
   - Flash partition: `huge_app.csv` (3.0 MB).
2. **Repeater (Relay)**:
   - Zero-UI headless packet relay (~30 KB RAM usage).
   - Startup boot reason and free heap diagnostics on Serial (115200 baud).
   - Safe default 20 dBm TX power to eliminate USB brownout resets.
   - Hardened buffer bounds for neighbor discovery (`discover.neighbors`).

### Quick Start / Szybki Start
Download the ZIP corresponding to your role, unzip, and double-click `flash-companion.bat` or `flash-repeater.bat`. No PlatformIO or Python installation required!
"@
}

Write-Host "Publishing GitHub Release: $Tag..." -ForegroundColor Cyan
gh release create $Tag $companionZip $repeaterZip --title "$Title" --notes "$Notes"

Write-Host "[SUCCESS] GitHub Release published successfully: $Tag" -ForegroundColor Green
