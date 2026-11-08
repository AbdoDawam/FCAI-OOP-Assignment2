#include "VoleGUI.h"
#include <commdlg.h>
#include <dwmapi.h>
#include <windowsx.h>
#include <cctype>
#include <sstream>

using namespace Gdiplus;

namespace {
    constexpr int IDC_CODE_EDITOR = 2001;

    enum ButtonIds {
        BTN_OPEN_FILE = 101,
        BTN_LOAD_TO_RAM = 102,
        BTN_RUN_ALL = 103,
        BTN_SINGLE_STEP = 104,
        BTN_HALT = 105,
        BTN_RESET = 106,
        BTN_HELP = 107,
        BTN_THEME = 108
    };

    std::wstring toWide(const std::string& str) {
        if (str.empty()) return L"";
        int n = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), nullptr, 0);
        std::wstring w(n, 0);
        MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &w[0], n);
        return w;
    }

    std::string toNarrow(const std::wstring& wstr) {
        if (wstr.empty()) return "";
        int n = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), nullptr, 0, nullptr, nullptr);
        std::string s(n, 0);
        WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &s[0], n, nullptr, nullptr);
        return s;
    }

    void addRoundedRect(GraphicsPath& path, const RectF& r, REAL radius) {
        REAL maxR = (r.Width < r.Height ? r.Width : r.Height) * 0.5f;
        if (radius > maxR) radius = maxR;
        if (radius <= 0.5f) {
            path.AddRectangle(r);
            return;
        }
        REAL d = radius * 2.0f;
        path.AddArc(r.X, r.Y, d, d, 180.0f, 90.0f);
        path.AddArc(r.X + r.Width - d, r.Y, d, d, 270.0f, 90.0f);
        path.AddArc(r.X + r.Width - d, r.Y + r.Height - d, d, d, 0.0f, 90.0f);
        path.AddArc(r.X, r.Y + r.Height - d, d, d, 90.0f, 90.0f);
        path.CloseFigure();
    }

    void fillRoundRect(Graphics& g, const Brush* brush, const RectF& r, REAL radius) {
        GraphicsPath path;
        addRoundedRect(path, r, radius);
        g.FillPath(brush, &path);
    }

    void drawRoundRect(Graphics& g, const Pen* pen, const RectF& r, REAL radius) {
        GraphicsPath path;
        addRoundedRect(path, r, radius);
        g.DrawPath(pen, &path);
    }

    void drawSoftCard(Graphics& g, const RectF& r, REAL radius, bool darkMode) {
        for (int i = 3; i >= 1; --i) {
            REAL spread = static_cast<REAL>(i) * 1.8f;
            RectF sr(r.X - spread * 0.4f, r.Y + spread * 0.9f, r.Width + spread * 0.8f, r.Height + spread * 0.8f);
            int alpha = darkMode ? (18 - i * 4) : (12 - i * 3);
            SolidBrush shBrush(Color(alpha, 0, 0, 0));
            fillRoundRect(g, &shBrush, sr, radius + spread);
        }

        Color cardBg = darkMode ? Color(255, 25, 30, 43) : Color(255, 255, 255, 255);
        SolidBrush bgBrush(cardBg);
        fillRoundRect(g, &bgBrush, r, radius);

        Color edgeCol = darkMode ? Color(35, 255, 255, 255) : Color(110, 226, 232, 240);
        Pen edgePen(edgeCol, 1.0f);
        drawRoundRect(g, &edgePen, r, radius);
    }
}

VoleGUI::VoleGUI()
    : hwnd(nullptr),
      hCodeEditor(nullptr),
      hEditorFont(nullptr),
      hEditorBgBrush(nullptr),
      darkMode(true),
      showHelpModal(false),
      selectedTarget(TARGET_NONE),
      selectedMemAddr(0),
      selectedRegIdx(0),
      startLoadAddr(0),
      hexInputBuffer(""),
      statusText("Ready"),
      statusIsError(false),
      hoveredButtonId(-1),
      hoveredMemCell(-1),
      hoveredRegIdx(-1) {}

VoleGUI::~VoleGUI() {
    if (hEditorFont) DeleteObject(hEditorFont);
    if (hEditorBgBrush) DeleteObject(hEditorBgBrush);
}

bool VoleGUI::create(HINSTANCE hInstance, int nCmdShow) {
    const wchar_t CLASS_NAME[] = L"VoleMachineSmoothModernGUI";

    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = VoleGUI::WindowProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    wc.lpszClassName = CLASS_NAME;

    RegisterClassExW(&wc);

    hwnd = CreateWindowExW(
        0, CLASS_NAME,
        L"Vole Machine Simulator",
        WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
        CW_USEDEFAULT, CW_USEDEFAULT, 1280, 800,
        nullptr, nullptr, hInstance, this);

    if (!hwnd) return false;

    BOOL useDark = TRUE;
    DwmSetWindowAttribute(hwnd, 20, &useDark, sizeof(useDark));

    hCodeEditor = CreateWindowExW(
        0, L"EDIT", L"",
        WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_WANTRETURN,
        880, 175, 340, 270,
        hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(IDC_CODE_EDITOR)), hInstance, nullptr);

    hEditorFont = CreateFontW(
        -16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, FIXED_PITCH | FF_MODERN, L"Consolas");

    SendMessageW(hCodeEditor, WM_SETFONT, reinterpret_cast<WPARAM>(hEditorFont), TRUE);
    hEditorBgBrush = CreateSolidBrush(RGB(34, 40, 56));

    RECT rc;
    GetClientRect(hwnd, &rc);
    updateEditorLayout(rc.right - rc.left, rc.bottom - rc.top);

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);
    return true;
}

int VoleGUI::runMessageLoop() {
    MSG msg = {};
    while (GetMessageW(&msg, nullptr, 0, 0)) {
        if (msg.message == WM_KEYDOWN && msg.wParam == VK_ESCAPE && showHelpModal) {
            showHelpModal = false;
            if (hCodeEditor) ShowWindow(hCodeEditor, SW_SHOW);
            InvalidateRect(hwnd, nullptr, FALSE);
            continue;
        }
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return static_cast<int>(msg.wParam);
}

void VoleGUI::loadEditorProgramToRAM() {
    if (!hCodeEditor) return;
    int len = GetWindowTextLengthW(hCodeEditor);
    std::wstring wtext(len + 1, L'\0');
    GetWindowTextW(hCodeEditor, &wtext[0], len + 1);
    wtext.resize(len);

    machine.resetMachine();
    std::string msg;
    int count = 0;
    bool ok = machine.loadProgramFromText(toNarrow(wtext), startLoadAddr, msg, count);
    statusText = msg;
    statusIsError = !ok;
    InvalidateRect(hwnd, nullptr, FALSE);
}

void VoleGUI::openProgramFileDialog() {
    wchar_t filePath[MAX_PATH] = {0};
    OPENFILENAMEW ofn = {};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFilter = L"Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
    ofn.lpstrFile = filePath;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
    ofn.lpstrTitle = L"Load Program File";

    if (GetOpenFileNameW(&ofn)) {
        std::string filename = toNarrow(filePath);
        std::string msg, src;
        int count = 0;
        machine.resetMachine();
        bool ok = machine.loadProgramFileWithReport(filename, startLoadAddr, msg, count, src);
        if (!src.empty() && hCodeEditor) {
            std::string norm;
            for (size_t i = 0; i < src.size(); ++i) {
                if (src[i] == '\n' && (i == 0 || src[i - 1] != '\r')) norm += "\r\n";
                else norm.push_back(src[i]);
            }
            SetWindowTextW(hCodeEditor, toWide(norm).c_str());
        }
        statusText = msg;
        statusIsError = !ok;
        InvalidateRect(hwnd, nullptr, FALSE);
    }
}

void VoleGUI::commitPendingHexEdit() {
    if (hexInputBuffer.empty()) return;
    std::string padded = (hexInputBuffer.size() == 1) ? ("0" + hexInputBuffer) : hexInputBuffer;

    if (selectedTarget == TARGET_MEMORY) {
        machine.getMemory().setCell(selectedMemAddr, padded);
        statusText = "Memory [" + ALU::decToHex(selectedMemAddr, 2) + "] = " + padded;
    } else if (selectedTarget == TARGET_REGISTER) {
        machine.getCPU().setRegister(selectedRegIdx, padded);
        statusText = "Register R" + ALU::decToHex(selectedRegIdx, 1) + " = " + padded;
    } else if (selectedTarget == TARGET_PC) {
        int evenAddr = ALU::hexToDec(padded) & 0xFE;
        machine.getCPU().setPC(evenAddr);
        statusText = "PC = " + ALU::decToHex(evenAddr, 2);
    } else if (selectedTarget == TARGET_START_ADDR) {
        startLoadAddr = ALU::hexToDec(padded) & 0xFE;
        if (startLoadAddr > 254) startLoadAddr = 0;
        statusText = "Start Address = " + ALU::decToHex(startLoadAddr, 2);
    }
    statusIsError = false;
    hexInputBuffer.clear();
}

void VoleGUI::applyHexInputDigit(char hexChar) {
    if (selectedTarget == TARGET_NONE) return;
    char u = static_cast<char>(std::toupper(static_cast<unsigned char>(hexChar)));
    if (!((u >= '0' && u <= '9') || (u >= 'A' && u <= 'F'))) return;

    hexInputBuffer.push_back(u);
    if (hexInputBuffer.size() >= 2) {
        commitPendingHexEdit();
    }
    InvalidateRect(hwnd, nullptr, FALSE);
}

void VoleGUI::updateEditorLayout(int width, int height) {
    if (!hCodeEditor) return;
    float rightX = width * 0.68f;
    float rightW = width - rightX - 22.0f;
    float contentH = height - 166.0f;

    int edX = static_cast<int>(rightX + 26.0f);
    int edY = 166;
    int edW = static_cast<int>(rightW - 52.0f);
    int edH = static_cast<int>(contentH * 0.48f - 20.0f);
    if (edH < 130) edH = 130;

    MoveWindow(hCodeEditor, edX, edY, edW, edH, TRUE);
}

void VoleGUI::renderFrame(HDC hdc, int width, int height) {
    if (width <= 0 || height <= 0) return;

    Bitmap backBuffer(width, height, PixelFormat32bppPARGB);
    Graphics g(&backBuffer);

    g.SetSmoothingMode(SmoothingModeHighQuality);
    g.SetPixelOffsetMode(PixelOffsetModeHighQuality);
    g.SetCompositingQuality(CompositingQualityHighQuality);
    g.SetTextRenderingHint(TextRenderingHintClearTypeGridFit);

    Color bg = darkMode ? Color(255, 17, 20, 28) : Color(255, 240, 244, 249);
    g.Clear(bg);
    buttons.clear();

    drawHeaderBar(g, width);

    float topY = 76.0f;
    float bottomBarH = 76.0f;
    float contentH = height - topY - bottomBarH - 14.0f;
    if (contentH < 420.0f) contentH = 420.0f;

    float leftW = width * 0.22f;
    if (leftW < 250.0f) leftW = 250.0f;
    float rightX = width * 0.68f;
    float centerW = rightX - leftW - 40.0f;
    float rightW = width - rightX - 22.0f;

    RectF cpuRect(22.0f, topY, leftW, contentH);
    RectF memRect(22.0f + leftW + 18.0f, topY, centerW, contentH);
    RectF rightRect(rightX, topY, rightW, contentH);

    drawCPUPanel(g, cpuRect);
    drawMemoryPanel(g, memRect);
    drawProgramAndScreenPanel(g, rightRect);
    drawBottomControlBar(g, width, height);

    if (showHelpModal) {
        drawHelpModal(g, width, height);
    }

    Graphics screenG(hdc);
    screenG.DrawImage(&backBuffer, 0, 0);
}

void VoleGUI::drawHeaderBar(Graphics& g, int width) {
    RectF headerCard(22.0f, 14.0f, width - 44.0f, 50.0f);
    drawSoftCard(g, headerCard, 16.0f, darkMode);

    Color textMain = darkMode ? Color(255, 241, 245, 249) : Color(255, 30, 41, 59);
    Color pillBg = darkMode ? Color(255, 34, 40, 56) : Color(255, 238, 242, 247);

    FontFamily ffSans(L"Segoe UI");
    Font titleFont(&ffSans, 16.5f, FontStyleBold, UnitPixel);
    Font statusFont(&ffSans, 13.0f, FontStyleRegular, UnitPixel);
    Font btnFont(&ffSans, 12.5f, FontStyleBold, UnitPixel);

    SolidBrush mainBrush(textMain);
    g.DrawString(L"Vole Machine Simulator", -1, &titleFont, PointF(42.0f, 27.0f), &mainBrush);

    Color stCol = statusIsError
                      ? Color(255, 251, 113, 133)
                      : (darkMode ? Color(255, 129, 140, 248) : Color(255, 79, 70, 229));
    SolidBrush stBrush(stCol);
    g.DrawString(toWide(statusText).c_str(), -1, &statusFont, PointF(275.0f, 29.0f), &stBrush);

    StringFormat centerFmt;
    centerFmt.SetAlignment(StringAlignmentCenter);
    centerFmt.SetLineAlignment(StringAlignmentCenter);

    RectF helpBtn(headerCard.X + headerCard.Width - 186.0f, headerCard.Y + 9.0f, 80.0f, 32.0f);
    RectF themeBtn(headerCard.X + headerCard.Width - 98.0f, headerCard.Y + 9.0f, 86.0f, 32.0f);
    buttons.push_back({BTN_HELP, helpBtn, L"Help", 0});
    buttons.push_back({BTN_THEME, themeBtn, darkMode ? L"Light UI" : L"Dark UI", 0});

    SolidBrush bBrush(pillBg);
    fillRoundRect(g, &bBrush, helpBtn, 16.0f);
    fillRoundRect(g, &bBrush, themeBtn, 16.0f);
    g.DrawString(L"Help", -1, &btnFont, helpBtn, &centerFmt, &mainBrush);
    g.DrawString(darkMode ? L"Light UI" : L"Dark UI", -1, &btnFont, themeBtn, &centerFmt, &mainBrush);
}

void VoleGUI::drawCPUPanel(Graphics& g, const RectF& rect) {
    drawSoftCard(g, rect, 20.0f, darkMode);

    Color pillBg = darkMode ? Color(255, 34, 40, 56) : Color(255, 241, 245, 249);
    Color textMain = darkMode ? Color(255, 241, 245, 249) : Color(255, 30, 41, 59);
    Color textSub = darkMode ? Color(255, 148, 163, 184) : Color(255, 100, 116, 139);

    SolidBrush itemBrush(pillBg);
    FontFamily ffSans(L"Segoe UI");
    FontFamily ffMono(L"Consolas");
    Font headerFont(&ffSans, 14.5f, FontStyleBold, UnitPixel);
    Font labelFont(&ffSans, 12.0f, FontStyleBold, UnitPixel);
    Font regFont(&ffMono, 14.5f, FontStyleBold, UnitPixel);
    Font bigMono(&ffMono, 18.0f, FontStyleBold, UnitPixel);

    SolidBrush mainBrush(textMain);
    SolidBrush subBrush(textSub);

    g.DrawString(L"CPU", -1, &headerFont, PointF(rect.X + 20.0f, rect.Y + 16.0f), &mainBrush);

    const CPU& cpu = machine.getCPU();

    float boxW = (rect.Width - 48.0f) * 0.5f;
    pcBoxRect = RectF(rect.X + 20.0f, rect.Y + 46.0f, boxW, 54.0f);
    RectF irBoxRect(pcBoxRect.X + boxW + 8.0f, rect.Y + 46.0f, boxW, 54.0f);

    fillRoundRect(g, &itemBrush, pcBoxRect, 14.0f);
    fillRoundRect(g, &itemBrush, irBoxRect, 14.0f);

    if (selectedTarget == TARGET_PC) {
        Pen selPen(Color(255, 99, 102, 241), 2.0f);
        drawRoundRect(g, &selPen, pcBoxRect, 14.0f);
    }

    g.DrawString(L"PC", -1, &labelFont, PointF(pcBoxRect.X + 14.0f, pcBoxRect.Y + 7.0f), &subBrush);
    g.DrawString(L"IR", -1, &labelFont, PointF(irBoxRect.X + 14.0f, irBoxRect.Y + 7.0f), &subBrush);

    std::string pcVal = (selectedTarget == TARGET_PC && !hexInputBuffer.empty())
                            ? (hexInputBuffer + "_")
                            : ALU::decToHex(cpu.getPC(), 2);
    SolidBrush indigoBrush(darkMode ? Color(255, 129, 140, 248) : Color(255, 79, 70, 229));
    SolidBrush amberBrush(darkMode ? Color(255, 251, 191, 36) : Color(255, 217, 119, 6));
    g.DrawString(toWide(pcVal).c_str(), -1, &bigMono, PointF(pcBoxRect.X + 14.0f, pcBoxRect.Y + 25.0f), &indigoBrush);
    g.DrawString(toWide(cpu.getIR()).c_str(), -1, &bigMono, PointF(irBoxRect.X + 14.0f, irBoxRect.Y + 25.0f), &amberBrush);

    g.DrawString(L"Registers", -1, &labelFont, PointF(rect.X + 20.0f, rect.Y + 112.0f), &subBrush);

    float gridTop = rect.Y + 136.0f;
    float availH = rect.Height - 154.0f;
    float gap = 10.0f;
    float colW = (rect.Width - 40.0f - gap) * 0.5f;
    float rowH = (availH - 7.0f * 7.0f) / 8.0f;
    if (rowH < 28.0f) rowH = 28.0f;

    const Register& regs = cpu.getRegisters();
    int lastMod = cpu.getLastModifiedReg();

    for (int i = 0; i < 16; ++i) {
        int col = i / 8;
        int row = i % 8;
        float rx = rect.X + 20.0f + col * (colW + gap);
        float ry = gridTop + row * (rowH + 7.0f);
        RectF rc(rx, ry, colW, rowH);
        regCardRects[i] = rc;

        bool isSel = (selectedTarget == TARGET_REGISTER && selectedRegIdx == i);
        bool isMod = (lastMod == i);
        bool isHov = (hoveredRegIdx == i);

        Color bg = pillBg;
        if (isMod) {
            bg = darkMode ? Color(55, 52, 211, 153) : Color(40, 16, 185, 129);
        } else if (isSel) {
            bg = darkMode ? Color(75, 99, 102, 241) : Color(45, 99, 102, 241);
        } else if (isHov) {
            bg = darkMode ? Color(255, 44, 52, 72) : Color(255, 226, 232, 240);
        }

        SolidBrush rb(bg);
        fillRoundRect(g, &rb, rc, 11.0f);

        if (isSel) {
            Pen sp(Color(255, 129, 140, 248), 1.8f);
            drawRoundRect(g, &sp, rc, 11.0f);
        }

        std::wstring rLabel = L"R" + toWide(ALU::decToHex(i, 1));
        std::string rVal = (isSel && !hexInputBuffer.empty()) ? (hexInputBuffer + "_") : regs.getReg(i);

        g.DrawString(rLabel.c_str(), -1, &labelFont,
                     PointF(rx + 14.0f, ry + (rowH - 18.0f) * 0.5f), &subBrush);

        SolidBrush valBrush(rVal != "00" ? textMain : textSub);
        g.DrawString(toWide(rVal).c_str(), -1, &regFont,
                     PointF(rx + colW - 40.0f, ry + (rowH - 18.0f) * 0.5f), &valBrush);
    }
}

void VoleGUI::drawMemoryPanel(Graphics& g, const RectF& rect) {
    drawSoftCard(g, rect, 20.0f, darkMode);

    Color cellBg = darkMode ? Color(255, 32, 38, 54) : Color(255, 243, 246, 251);
    Color textMain = darkMode ? Color(255, 241, 245, 249) : Color(255, 30, 41, 59);
    Color textSub = darkMode ? Color(255, 148, 163, 184) : Color(255, 100, 116, 139);

    FontFamily ffSans(L"Segoe UI");
    FontFamily ffMono(L"Consolas");
    Font headerFont(&ffSans, 14.5f, FontStyleBold, UnitPixel);
    Font smallFont(&ffSans, 12.0f, FontStyleRegular, UnitPixel);
    Font hdrMono(&ffMono, 12.5f, FontStyleBold, UnitPixel);
    Font cellMono(&ffMono, 12.5f, FontStyleBold, UnitPixel);

    SolidBrush mainBrush(textMain);
    SolidBrush subBrush(textSub);
    StringFormat centerFmt;
    centerFmt.SetAlignment(StringAlignmentCenter);
    centerFmt.SetLineAlignment(StringAlignmentCenter);

    g.DrawString(L"Main Memory (16 × 16)", -1, &headerFont, PointF(rect.X + 20.0f, rect.Y + 16.0f), &mainBrush);
    g.DrawString(L"Click any cell to edit", -1, &smallFont,
                 PointF(rect.X + rect.Width - 145.0f, rect.Y + 18.0f), &subBrush);

    float gridTop = rect.Y + 48.0f;
    float gridH = rect.Height - 66.0f;
    float gridW = rect.Width - 40.0f;

    float hdrSize = 24.0f;
    float gap = 4.5f;
    float cellW = (gridW - hdrSize - gap * 15.0f) / 16.0f;
    float cellH = (gridH - hdrSize - gap * 15.0f) / 16.0f;

    SolidBrush hdrBrush(darkMode ? Color(255, 129, 140, 248) : Color(255, 99, 102, 241));
    for (int c = 0; c < 16; ++c) {
        RectF chr(rect.X + 20.0f + hdrSize + c * (cellW + gap), gridTop, cellW, hdrSize);
        g.DrawString(toWide(ALU::decToHex(c, 1)).c_str(), -1, &hdrMono, chr, &centerFmt, &hdrBrush);
    }

    const Memory& mem = machine.getMemory();
    const CPU& cpu = machine.getCPU();
    int pc = cpu.getPC();
    int lastWr = cpu.getLastWrittenMem();

    for (int r = 0; r < 16; ++r) {
        float ry = gridTop + hdrSize + r * (cellH + gap);
        RectF rhr(rect.X + 16.0f, ry, hdrSize, cellH);
        g.DrawString(toWide(ALU::decToHex(r, 1)).c_str(), -1, &hdrMono, rhr, &centerFmt, &hdrBrush);

        for (int c = 0; c < 16; ++c) {
            int addr = r * 16 + c;
            float cx = rect.X + 20.0f + hdrSize + c * (cellW + gap);
            RectF cr(cx, ry, cellW, cellH);
            memCellRects[addr] = cr;

            std::string val = mem.getCell(addr);
            bool isPC = (addr == pc || addr == pc + 1);
            bool isWr = (addr == lastWr);
            bool isSel = (selectedTarget == TARGET_MEMORY && selectedMemAddr == addr);
            bool isHov = (hoveredMemCell == addr);

            Color bg = cellBg;
            if (val != "00") {
                bg = darkMode ? Color(255, 45, 55, 78) : Color(255, 224, 231, 255);
            }
            if (isWr) {
                bg = darkMode ? Color(70, 52, 211, 153) : Color(45, 16, 185, 129);
            }
            if (isPC) {
                bg = darkMode ? Color(85, 99, 102, 241) : Color(55, 99, 102, 241);
            }
            if (isSel) {
                bg = darkMode ? Color(120, 99, 102, 241) : Color(80, 99, 102, 241);
            } else if (isHov) {
                bg = darkMode ? Color(255, 48, 57, 79) : Color(255, 226, 232, 240);
            }

            SolidBrush cb(bg);
            fillRoundRect(g, &cb, cr, 7.5f);

            if (isSel || isPC) {
                Color ringCol = isSel ? Color(255, 165, 180, 252) : Color(180, 129, 140, 248);
                Pen ringPen(ringCol, 1.5f);
                drawRoundRect(g, &ringPen, cr, 7.5f);
            }

            std::string disp = (isSel && !hexInputBuffer.empty()) ? (hexInputBuffer + "_") : val;
            Color txtCol = (val != "00" || isSel || isPC)
                               ? textMain
                               : (darkMode ? Color(255, 90, 103, 128) : Color(255, 160, 174, 192));
            SolidBrush tb(txtCol);
            g.DrawString(toWide(disp).c_str(), -1, &cellMono, cr, &centerFmt, &tb);
        }
    }
}

void VoleGUI::drawProgramAndScreenPanel(Graphics& g, const RectF& rect) {
    drawSoftCard(g, rect, 20.0f, darkMode);

    Color pillBg = darkMode ? Color(255, 34, 40, 56) : Color(255, 241, 245, 249);
    Color textMain = darkMode ? Color(255, 241, 245, 249) : Color(255, 30, 41, 59);
    Color textSub = darkMode ? Color(255, 148, 163, 184) : Color(255, 100, 116, 139);

    SolidBrush itemBrush(pillBg);
    FontFamily ffSans(L"Segoe UI");
    FontFamily ffMono(L"Consolas");
    Font headerFont(&ffSans, 14.5f, FontStyleBold, UnitPixel);
    Font smallBold(&ffSans, 12.5f, FontStyleBold, UnitPixel);
    Font monoFont(&ffMono, 14.0f, FontStyleBold, UnitPixel);

    SolidBrush mainBrush(textMain);
    SolidBrush subBrush(textSub);
    StringFormat centerFmt;
    centerFmt.SetAlignment(StringAlignmentCenter);
    centerFmt.SetLineAlignment(StringAlignmentCenter);

    g.DrawString(L"Program Input", -1, &headerFont, PointF(rect.X + 20.0f, rect.Y + 16.0f), &mainBrush);

    float rowY = rect.Y + 46.0f;
    g.DrawString(L"Start Addr:", -1, &smallBold, PointF(rect.X + 20.0f, rowY + 7.0f), &subBrush);

    startAddrBoxRect = RectF(rect.X + 98.0f, rowY, 52.0f, 34.0f);
    fillRoundRect(g, &itemBrush, startAddrBoxRect, 12.0f);
    if (selectedTarget == TARGET_START_ADDR) {
        Pen saPen(Color(255, 129, 140, 248), 1.8f);
        drawRoundRect(g, &saPen, startAddrBoxRect, 12.0f);
    }

    std::string saText = (selectedTarget == TARGET_START_ADDR && !hexInputBuffer.empty())
                             ? (hexInputBuffer + "_")
                             : ALU::decToHex(startLoadAddr, 2);
    SolidBrush indigoBrush(darkMode ? Color(255, 129, 140, 248) : Color(255, 79, 70, 229));
    g.DrawString(toWide(saText).c_str(), -1, &monoFont, startAddrBoxRect, &centerFmt, &indigoBrush);

    RectF loadBtn(rect.X + 158.0f, rowY, rect.Width - 178.0f, 34.0f);
    buttons.push_back({BTN_LOAD_TO_RAM, loadBtn, L"Load Program to Memory", 1});
    LinearGradientBrush loadGrad(loadBtn, Color(255, 99, 102, 241), Color(255, 79, 70, 229), LinearGradientModeHorizontal);
    SolidBrush whiteBrush(Color(255, 255, 255, 255));
    fillRoundRect(g, &loadGrad, loadBtn, 14.0f);
    g.DrawString(L"Load Program to Memory", -1, &smallBold, loadBtn, &centerFmt, &whiteBrush);

    int edH = static_cast<int>(rect.Height * 0.48f);
    if (edH < 150) edH = 150;
    RectF edContainer(rect.X + 18.0f, 156.0f, rect.Width - 36.0f, static_cast<float>(edH));
    fillRoundRect(g, &itemBrush, edContainer, 16.0f);

    float scrTitleY = edContainer.Y + edContainer.Height + 20.0f;
    g.DrawString(L"Screen Display (Address FF)", -1, &headerFont,
                 PointF(rect.X + 20.0f, scrTitleY), &mainBrush);

    float scrBoxY = scrTitleY + 30.0f;
    float scrBoxH = rect.Y + rect.Height - scrBoxY - 20.0f;
    if (scrBoxH < 60.0f) scrBoxH = 60.0f;

    RectF scrBox(rect.X + 18.0f, scrBoxY, rect.Width - 36.0f, scrBoxH);
    fillRoundRect(g, &itemBrush, scrBox, 16.0f);

    const std::string& scrText = machine.getScreenBuffer();
    std::string cellFF = machine.getMemory().getCell(0xFF);
    Font scrFont(&ffMono, 16.0f, FontStyleBold, UnitPixel);
    SolidBrush mintBrush(darkMode ? Color(255, 52, 211, 153) : Color(255, 5, 150, 105));

    if (scrText.empty()) {
        g.DrawString(L"Screen is empty", -1, &smallBold,
                     PointF(scrBox.X + 16.0f, scrBox.Y + 16.0f), &subBrush);
    } else {
        g.DrawString(toWide(scrText).c_str(), -1, &scrFont,
                     PointF(scrBox.X + 16.0f, scrBox.Y + 16.0f), &mintBrush);
        std::wstring hexInfo = L"Last Byte [FF]: 0x" + toWide(cellFF) +
                               L"  (" + std::to_wstring(ALU::hexToSignedTwosComplement(cellFF)) + L")";
        g.DrawString(hexInfo.c_str(), -1, &smallBold,
                     PointF(scrBox.X + 16.0f, scrBox.Y + scrBox.Height - 30.0f), &subBrush);
    }
}

void VoleGUI::drawBottomControlBar(Graphics& g, int width, int height) {
    RectF barCard(22.0f, height - 74.0f, width - 44.0f, 60.0f);
    drawSoftCard(g, barCard, 18.0f, darkMode);

    struct BottomBtn {
        int id;
        const wchar_t* label;
        float w;
        int style;
    };
    BottomBtn bSpecs[] = {
        {BTN_OPEN_FILE, L"📂 Load File...", 152.0f, 0},
        {BTN_RUN_ALL, L"▶ Run All", 152.0f, 2},
        {BTN_SINGLE_STEP, L"⏭ Single Step", 152.0f, 1},
        {BTN_HALT, L"⏹ Halt", 132.0f, 4},
        {BTN_RESET, L"🔄 Clear / Reset", 162.0f, 0}
    };

    float totalW = 0;
    for (const auto& b : bSpecs) totalW += b.w + 16.0f;
    totalW -= 16.0f;

    float startX = (width - totalW) * 0.5f;
    FontFamily ffSans(L"Segoe UI");
    Font btnFont(&ffSans, 13.5f, FontStyleBold, UnitPixel);
    StringFormat centerFmt;
    centerFmt.SetAlignment(StringAlignmentCenter);
    centerFmt.SetLineAlignment(StringAlignmentCenter);

    for (const auto& b : bSpecs) {
        RectF r(startX, barCard.Y + 10.0f, b.w, 40.0f);
        buttons.push_back({b.id, r, b.label, b.style});

        bool hov = (hoveredButtonId == b.id);
        Color bg1, bg2, fg;
        if (b.style == 2) {
            bg1 = hov ? Color(255, 16, 185, 129) : Color(255, 52, 211, 153);
            bg2 = hov ? Color(255, 5, 150, 105) : Color(255, 16, 185, 129);
            fg = Color(255, 255, 255, 255);
        } else if (b.style == 1) {
            bg1 = hov ? Color(255, 99, 102, 241) : Color(255, 129, 140, 248);
            bg2 = hov ? Color(255, 79, 70, 229) : Color(255, 99, 102, 241);
            fg = Color(255, 255, 255, 255);
        } else if (b.style == 4) {
            bg1 = hov ? Color(255, 244, 63, 94) : Color(255, 251, 113, 133);
            bg2 = hov ? Color(255, 225, 29, 72) : Color(255, 244, 63, 94);
            fg = Color(255, 255, 255, 255);
        } else {
            Color c = darkMode
                          ? (hov ? Color(255, 45, 53, 74) : Color(255, 34, 40, 56))
                          : (hov ? Color(255, 226, 232, 240) : Color(255, 238, 242, 247));
            bg1 = c;
            bg2 = c;
            fg = darkMode ? Color(255, 241, 245, 249) : Color(255, 30, 41, 59);
        }

        LinearGradientBrush bb(r, bg1, bg2, LinearGradientModeVertical);
        SolidBrush fb(fg);
        fillRoundRect(g, &bb, r, 20.0f);
        g.DrawString(b.label, -1, &btnFont, r, &centerFmt, &fb);

        startX += b.w + 16.0f;
    }
}

void VoleGUI::drawHelpModal(Graphics& g, int width, int height) {
    SolidBrush dim(Color(185, 10, 12, 18));
    g.FillRectangle(&dim, 0, 0, width, height);

    float mw = 740.0f;
    float mh = 500.0f;
    RectF mr((width - mw) * 0.5f, (height - mh) * 0.5f, mw, mh);

    drawSoftCard(g, mr, 24.0f, true);

    FontFamily ffSans(L"Segoe UI");
    FontFamily ffMono(L"Consolas");
    Font titleF(&ffSans, 17.0f, FontStyleBold, UnitPixel);
    Font rowF(&ffSans, 13.0f, FontStyleRegular, UnitPixel);
    Font monoF(&ffMono, 13.5f, FontStyleBold, UnitPixel);

    SolidBrush whiteB(Color(255, 241, 245, 249));
    SolidBrush indigoB(Color(255, 129, 140, 248));
    SolidBrush subB(Color(255, 148, 163, 184));

    g.DrawString(L"Vole Machine Instructions (Click anywhere to close)",
                 -1, &titleF, PointF(mr.X + 28.0f, mr.Y + 24.0f), &whiteB);

    const wchar_t* lines[][2] = {
        {L"1 RXY", L"Load register R from memory cell XY"},
        {L"2 RXY", L"Load register R with immediate value XY"},
        {L"3 RXY", L"Store register R into memory cell XY"},
        {L"3 RFF", L"Store register R to Screen Display (address FF)"},
        {L"4 0RS", L"Move (copy) register R into register S"},
        {L"5 RST", L"Add registers S and T (Two's Complement) -> R"},
        {L"6 RST", L"Add registers S and T (Floating-Point) -> R"},
        {L"7 RST", L"Bitwise OR registers S and T -> R"},
        {L"8 RST", L"Bitwise AND registers S and T -> R"},
        {L"9 RST", L"Bitwise XOR registers S and T -> R"},
        {L"A R0X", L"Rotate register R right by X bits"},
        {L"B RXY", L"Jump to address XY if register R == R0"},
        {L"C 000", L"Halt execution"}
    };

    float y = mr.Y + 68.0f;
    for (int i = 0; i < 13; ++i) {
        g.DrawString(lines[i][0], -1, &monoF, PointF(mr.X + 34.0f, y), &indigoB);
        g.DrawString(lines[i][1], -1, &rowF, PointF(mr.X + 124.0f, y), &whiteB);
        y += 30.0f;
    }

    g.DrawString(L"Click any Register (R0-RF), Memory Cell (00-FF), or PC to type a new hex value.",
                 -1, &rowF, PointF(mr.X + 28.0f, mr.Y + mh - 38.0f), &subB);
}

void VoleGUI::onButtonClick(int buttonId) {
    commitPendingHexEdit();

    switch (buttonId) {
        case BTN_OPEN_FILE:
            openProgramFileDialog();
            break;
        case BTN_LOAD_TO_RAM:
            loadEditorProgramToRAM();
            break;
        case BTN_RUN_ALL: {
            if (machine.getCPU().isHalted()) {
                machine.getCPU().setPC(startLoadAddr);
            }
            int steps = machine.executeAllAtOnce(1000);
            statusText = "Executed " + std::to_string(steps) + " steps — " +
                         machine.getCPU().getLastActionDescription();
            statusIsError = false;
            InvalidateRect(hwnd, nullptr, FALSE);
            break;
        }
        case BTN_SINGLE_STEP: {
            if (machine.getCPU().isHalted()) {
                statusText = "Halted.";
                statusIsError = true;
            } else {
                machine.executeSingleStep();
                statusText = machine.getCPU().getLastActionDescription();
                statusIsError = false;
            }
            InvalidateRect(hwnd, nullptr, FALSE);
            break;
        }
        case BTN_HALT:
            machine.haltExecution();
            statusText = "Halted.";
            statusIsError = false;
            InvalidateRect(hwnd, nullptr, FALSE);
            break;
        case BTN_RESET:
            machine.resetMachine();
            statusText = "Reset.";
            statusIsError = false;
            InvalidateRect(hwnd, nullptr, FALSE);
            break;
        case BTN_HELP:
            showHelpModal = !showHelpModal;
            if (hCodeEditor) ShowWindow(hCodeEditor, showHelpModal ? SW_HIDE : SW_SHOW);
            InvalidateRect(hwnd, nullptr, FALSE);
            break;
        case BTN_THEME:
            darkMode = !darkMode;
            if (hEditorBgBrush) DeleteObject(hEditorBgBrush);
            hEditorBgBrush = CreateSolidBrush(darkMode ? RGB(34, 40, 56) : RGB(241, 245, 249));
            if (hCodeEditor) InvalidateRect(hCodeEditor, nullptr, TRUE);
            InvalidateRect(hwnd, nullptr, FALSE);
            break;
    }
}

void VoleGUI::onMouseDown(int x, int y) {
    PointF pt(static_cast<REAL>(x), static_cast<REAL>(y));

    if (showHelpModal) {
        showHelpModal = false;
        if (hCodeEditor) ShowWindow(hCodeEditor, SW_SHOW);
        InvalidateRect(hwnd, nullptr, FALSE);
        return;
    }

    for (const auto& b : buttons) {
        if (b.rect.Contains(pt)) {
            SetFocus(hwnd);
            onButtonClick(b.id);
            return;
        }
    }

    for (int i = 0; i < 256; ++i) {
        if (memCellRects[i].Contains(pt)) {
            commitPendingHexEdit();
            selectedTarget = TARGET_MEMORY;
            selectedMemAddr = i;
            SetFocus(hwnd);
            statusText = "Selected Memory [" + ALU::decToHex(i, 2) + "] — Type 2 hex digits.";
            statusIsError = false;
            InvalidateRect(hwnd, nullptr, FALSE);
            return;
        }
    }

    for (int i = 0; i < 16; ++i) {
        if (regCardRects[i].Contains(pt)) {
            commitPendingHexEdit();
            selectedTarget = TARGET_REGISTER;
            selectedRegIdx = i;
            SetFocus(hwnd);
            statusText = "Selected Register R" + ALU::decToHex(i, 1) + " — Type 2 hex digits.";
            statusIsError = false;
            InvalidateRect(hwnd, nullptr, FALSE);
            return;
        }
    }

    if (pcBoxRect.Contains(pt)) {
        commitPendingHexEdit();
        selectedTarget = TARGET_PC;
        SetFocus(hwnd);
        statusText = "Selected PC — Type an even hex address.";
        statusIsError = false;
        InvalidateRect(hwnd, nullptr, FALSE);
        return;
    }

    if (startAddrBoxRect.Contains(pt)) {
        commitPendingHexEdit();
        selectedTarget = TARGET_START_ADDR;
        SetFocus(hwnd);
        statusText = "Selected Start Address — Type an even hex address.";
        statusIsError = false;
        InvalidateRect(hwnd, nullptr, FALSE);
        return;
    }
}

void VoleGUI::onMouseMove(int x, int y) {
    PointF pt(static_cast<REAL>(x), static_cast<REAL>(y));
    int newBtn = -1;
    int newMem = -1;
    int newReg = -1;

    for (const auto& b : buttons) {
        if (b.rect.Contains(pt)) {
            newBtn = b.id;
            break;
        }
    }
    if (newBtn == -1) {
        for (int i = 0; i < 256; ++i) {
            if (memCellRects[i].Contains(pt)) {
                newMem = i;
                break;
            }
        }
    }
    if (newBtn == -1 && newMem == -1) {
        for (int i = 0; i < 16; ++i) {
            if (regCardRects[i].Contains(pt)) {
                newReg = i;
                break;
            }
        }
    }

    if (newBtn != hoveredButtonId || newMem != hoveredMemCell || newReg != hoveredRegIdx) {
        hoveredButtonId = newBtn;
        hoveredMemCell = newMem;
        hoveredRegIdx = newReg;
        InvalidateRect(hwnd, nullptr, FALSE);
    }
}

void VoleGUI::onCharInput(WCHAR ch) {
    if (showHelpModal) return;
    if ((ch >= L'0' && ch <= L'9') || (ch >= L'a' && ch <= L'f') || (ch >= L'A' && ch <= L'F')) {
        applyHexInputDigit(static_cast<char>(ch));
    }
}

void VoleGUI::onKeyDown(WPARAM key) {
    if (key == VK_RETURN) {
        commitPendingHexEdit();
        InvalidateRect(hwnd, nullptr, FALSE);
    } else if (key == VK_BACK && !hexInputBuffer.empty()) {
        hexInputBuffer.pop_back();
        InvalidateRect(hwnd, nullptr, FALSE);
    }
}

LRESULT CALLBACK VoleGUI::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    VoleGUI* self = nullptr;
    if (uMsg == WM_NCCREATE) {
        CREATESTRUCTW* cs = reinterpret_cast<CREATESTRUCTW*>(lParam);
        self = reinterpret_cast<VoleGUI*>(cs->lpCreateParams);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
    } else {
        self = reinterpret_cast<VoleGUI*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    }

    if (!self) return DefWindowProcW(hwnd, uMsg, wParam, lParam);

    switch (uMsg) {
        case WM_ERASEBKGND:
            return 1;
        case WM_SIZE:
            self->updateEditorLayout(LOWORD(lParam), HIWORD(lParam));
            InvalidateRect(hwnd, nullptr, FALSE);
            return 0;
        case WM_GETMINMAXINFO: {
            MINMAXINFO* mmi = reinterpret_cast<MINMAXINFO*>(lParam);
            mmi->ptMinTrackSize.x = 1050;
            mmi->ptMinTrackSize.y = 680;
            return 0;
        }
        case WM_CTLCOLOREDIT: {
            HDC hdcEdit = reinterpret_cast<HDC>(wParam);
            if (self->darkMode) {
                SetTextColor(hdcEdit, RGB(226, 232, 240));
                SetBkColor(hdcEdit, RGB(34, 40, 56));
            } else {
                SetTextColor(hdcEdit, RGB(30, 41, 59));
                SetBkColor(hdcEdit, RGB(241, 245, 249));
            }
            return reinterpret_cast<INT_PTR>(self->hEditorBgBrush);
        }
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            RECT rc;
            GetClientRect(hwnd, &rc);
            self->renderFrame(hdc, rc.right - rc.left, rc.bottom - rc.top);
            EndPaint(hwnd, &ps);
            return 0;
        }
        case WM_LBUTTONDOWN:
            self->onMouseDown(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
            return 0;
        case WM_MOUSEMOVE:
            self->onMouseMove(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
            return 0;
        case WM_CHAR:
            self->onCharInput(static_cast<WCHAR>(wParam));
            return 0;
        case WM_KEYDOWN:
            self->onKeyDown(wParam);
            return 0;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }

    return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}
