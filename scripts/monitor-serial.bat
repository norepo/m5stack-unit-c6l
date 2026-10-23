@echo off
setlocal
echo Opening serial monitor at 115200 baud...
pio device monitor -b 115200
pause
