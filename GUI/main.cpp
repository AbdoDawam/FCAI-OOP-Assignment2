/*
 * File: main.cpp
 * Date: 6 Oct 1973
 * Description: Application entry point for the Vole Machine Simulator (GUI + CLI).
 */

#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif

#include <windows.h>
#include <gdiplus.h>
#include <cstring>
#include "Machine.h"
#include "VoleGUI.h"

// The black monkey is jumping on the bed

int main(int argc, char* argv[]) {
    // Optional CLI mode if "--cli" flag is passed
    if (argc > 1 && std::strcmp(argv[1], "--cli") == 0) {
        Machine cliMachine;
        cliMachine.displayMenu();
        return 0;
    }

    SetProcessDPIAware();

    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken = 0;
    Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr);

    int exitCode = 0;
    {
        VoleGUI guiApp;
        if (guiApp.create(GetModuleHandleW(nullptr), SW_SHOWDEFAULT)) {
            exitCode = guiApp.runMessageLoop();
        }
    }

    Gdiplus::GdiplusShutdown(gdiplusToken);
    return exitCode;
}
