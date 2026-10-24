# PowerShell script to completely erase flash on ESP32-C6
param (
    [string]$Port = ""
)

$EsptoolPath = "$env:USERPROFILE\.platformio\packages\tool-esptoolpy\esptool.py"
$PythonCmd = "python"

$PortArg = @()
if ($Port -ne "") {
    $PortArg = @("-p", $Port)
}

Write-Host "==========================================================" -ForegroundColor Cyan
Write-Host "  M5Stack Unit C6L - Erasing Full Flash Memory" -ForegroundColor Cyan
Write-Host "==========================================================" -ForegroundColor Cyan

if (Test-Path $EsptoolPath) {
    & $PythonCmd $EsptoolPath --chip esp32c6 @PortArg --baud 921600 erase_flash
    Write-Host "Flash successfully erased!" -ForegroundColor Green
} else {
    Write-Error "esptool not found at $EsptoolPath"
}
