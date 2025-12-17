@echo off
echo Cleaning build files...

if exist obj rmdir /s /q obj
if exist rapid-texter.exe del /q rapid-texter.exe

echo Clean complete!
pause