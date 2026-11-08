========================================================================
VOLE MACHINE SIMULATOR - MODERN OOP C++ GUI EDITION
CS213: Object Oriented Programming - Assignment 2 (Part 2 & Bonus Part 3)
========================================================================

1. ARCHITECTURE OVERVIEW (OOP C++ DESIGN)
-----------------------------------------
The project is divided into separate header (.h) and implementation (.cpp)
files following the 3-package Object-Oriented Architecture:

  [Package: Storage]
    - Storage.h / Storage.cpp   : Abstract base class (size, cells, normalizeHex)
    - Memory.h  / Memory.cpp    : 256-cell main memory inheriting from Storage
    - Register.h / Register.cpp : 16 general-purpose registers (R0-RF) inheriting from Storage

  [Package: Processing]
    - ALU.h / ALU.cpp           : Stateless Arithmetic Logic Unit (2's complement,
                                  Brookshear 8-bit excess-4 float, OR, AND, XOR, Rotate)
    - CPU.h / CPU.cpp           : Central Processing Unit (PC, IR, fetch, decodeAndExecute,
                                  runNextStep, instruction validation & disassembler)

  [Package: Orchestration & GUI]
    - Machine.h / Machine.cpp   : Top-level Machine simulator orchestrator
    - VoleGUI.h / VoleGUI.cpp   : Modern double-buffered GDI+ / Win32 GUI studio
    - main.cpp                  : Application entry point (launches GUI, or CLI with --cli)


2. HOW TO COMPILE & RUN
-----------------------
Option A: Double-click `build_and_run.bat`

Option B: Compile manually using MinGW-w64 (g++):
  g++ -std=c++17 Storage.cpp Memory.cpp Register.cpp ALU.cpp CPU.cpp Machine.cpp VoleGUI.cpp main.cpp -o VoleMachineGUI.exe -lgdiplus -lgdi32 -lcomdlg32 -ldwmapi -mwindows

To run the GUI:
  .\VoleMachineGUI.exe

To run in Console Menu (CLI) mode:
  .\VoleMachineGUI.exe --cli


3. GUI FEATURES & CONTROLS
--------------------------
- [Load File...]         : Load any Vole program (.txt) from disk.
- [Program Input]        : Type or paste Vole instructions, set an even Start Addr
                           (00 .. FE), and click 'Load Program to Memory'.
- [Run All]              : Execute the entire program at once.
- [Single Step]          : Execute one Fetch-Decode-Execute cycle.
- [Halt / Clear / Reset] : Stop execution or reset Memory, Registers, PC, IR, and Screen.
- [Direct Overwrite]     : Click any Memory Cell (00-FF), Register (R0-RF), or PC
                           and type 2 hex digits (0-9, A-F) on your keyboard.
- [Screen Display (FF)]  : Shows output written to memory-mapped screen address FF.
- [Help / Light-Dark UI] : Quick instruction reference and Dark/Light theme toggle.
