/*
 * File: VoleGUI.h
 * Date: 6 Oct 1973
 * Package: GUI
 * Description: Clean, Simple & Modern OOP C++ GUI for the Vole Machine Simulator.
 */

#ifndef VOLE_GUI_H
#define VOLE_GUI_H

#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif

#include <windows.h>
#include <gdiplus.h>
#include <string>
#include <vector>
#include "Machine.h"

struct UIButton {
    int id;
    Gdiplus::RectF rect;
    std::wstring label;
    int style; // 0=neutral, 1=primary blue, 2=green, 3=purple, 4=red
};

class VoleGUI {
private:
    Machine machine;
    HWND hwnd;
    HWND hCodeEditor;
    HFONT hEditorFont;
    HBRUSH hEditorBgBrush;

    bool darkMode;
    bool showHelpModal;

    // Simple click-to-overwrite state
    enum EditTarget { TARGET_NONE = 0, TARGET_MEMORY = 1, TARGET_REGISTER = 2, TARGET_PC = 3, TARGET_START_ADDR = 4 };
    EditTarget selectedTarget;
    int selectedMemAddr;
    int selectedRegIdx;
    int startLoadAddr;
    std::string hexInputBuffer;
    std::string statusText;
    bool statusIsError;

    int hoveredButtonId;
    int hoveredMemCell;
    int hoveredRegIdx;

    std::vector<UIButton> buttons;
    Gdiplus::RectF memCellRects[256];
    Gdiplus::RectF regCardRects[16];
    Gdiplus::RectF pcBoxRect;
    Gdiplus::RectF startAddrBoxRect;

    void loadEditorProgramToRAM();
    void openProgramFileDialog();
    void applyHexInputDigit(char hexChar);
    void commitPendingHexEdit();

    void renderFrame(HDC hdc, int width, int height);
    void drawHeaderBar(Gdiplus::Graphics& g, int width);
    void drawCPUPanel(Gdiplus::Graphics& g, const Gdiplus::RectF& rect);
    void drawMemoryPanel(Gdiplus::Graphics& g, const Gdiplus::RectF& rect);
    void drawProgramAndScreenPanel(Gdiplus::Graphics& g, const Gdiplus::RectF& rect);
    void drawBottomControlBar(Gdiplus::Graphics& g, int width, int height);
    void drawHelpModal(Gdiplus::Graphics& g, int width, int height);

    void onButtonClick(int buttonId);
    void onMouseDown(int x, int y);
    void onMouseMove(int x, int y);
    void onCharInput(WCHAR ch);
    void onKeyDown(WPARAM key);
    void updateEditorLayout(int width, int height);

    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

public:
    VoleGUI();
    ~VoleGUI();

    bool create(HINSTANCE hInstance, int nCmdShow);
    int runMessageLoop();
};

#endif // VOLE_GUI_H
