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

int main(int argc, char* argv[]) {
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
