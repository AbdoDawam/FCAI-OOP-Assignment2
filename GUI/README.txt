========================================================================
VOLE MACHINE SIMULATOR
CS213: Object Oriented Programming - Assignment 2 (Part 2 & Bonus Part 3)
========================================================================

1. HOW TO COMPILE & RUN
-----------------------
Option A: Double-click `build_and_run.bat`

Option B: Compile manually using MinGW-w64 (g++):
  g++ -std=c++17 Storage.cpp Memory.cpp Register.cpp ALU.cpp CPU.cpp Machine.cpp VoleGUI.cpp main.cpp -o VoleMachineGUI.exe -lgdiplus -lgdi32 -lcomdlg32 -ldwmapi -mwindows

To run the GUI:
  .\VoleMachineGUI.exe

To run in Console Menu (CLI) mode:
  .\VoleMachineGUI.exe --cli
