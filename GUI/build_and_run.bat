@echo off
echo Compiling Vole Machine Simulator GUI...
g++ -std=c++17 Storage.cpp Memory.cpp Register.cpp ALU.cpp CPU.cpp Machine.cpp VoleGUI.cpp main.cpp -o VoleMachineGUI.exe -lgdiplus -lgdi32 -lcomdlg32 -ldwmapi -mwindows
if %ERRORLEVEL% EQU 0 (
    echo Build successful! Launching VoleMachineGUI.exe...
    start "" VoleMachineGUI.exe
) else (
    echo Build failed!
    pause
)
