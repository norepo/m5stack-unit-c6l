# PowerShell Flashing Script for M5Stack Unit C6L - Companion Radio (BLE)
param (
    [string]$Port = ""
)

$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$ProjectRoot = Split-Path -Parent $ScriptDir
$FwDir = Join-Path $ProjectRoot "firmware\01-companion-radio-ble"
$Bootloader = Join-Path $FwDir "bootloader.bin"
$Partitions = Join-Path $FwDir "partitions.bin"
$Firmware = Join-Path $FwDir "firmware.bin"

Write-Host "==========================================================" -ForegroundColor Cyan
Write-Host "  M5Stack Unit C6L - Flashing Companion Radio (BLE)" -ForegroundColor Cyan
Write-Host "==========================================================" -ForegroundColor Cyan

# Find esptool
$EsptoolPath = "$env:USERPROFILE\.platformio\packages\tool-esptoolpy\esptool.py"
$PythonCmd = "python"

$PortArg = @()
if ($Port -ne "") {
    $PortArg = @("-p", $Port)
}

if (Test-Path $EsptoolPath) {
    Write-Host "Using PlatformIO esptool: $EsptoolPath" -ForegroundColor Green
    & $PythonCmd $EsptoolPath --chip esp32c6 @PortArg --baud 921600 --before default_reset --after hard_reset write_flash -z --flash_mode dio --flash_freq 80m --flash_size 4MB 0x0 $Bootloader 0x8000 $Partitions 0x10000 $Firmware
} else {
    Write-Host "Checking for pio fallback..." -ForegroundColor Yellow
    if (Test-Path (Join-Path $ProjectRoot "MeshCore")) {
        Set-Location (Join-Path $ProjectRoot "MeshCore")
        pio run -e M5Stack_Unit_C6L_companion_radio_ble -t upload
    } else {
        Write-Error "esptool not found. Please install via: pip install esptool"
    }
}
