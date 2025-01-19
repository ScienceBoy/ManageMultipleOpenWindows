// Sicherheit bei der verarbeitung der suchtexte, stichwort sql-injection und co.
// Signieren
#define UNICODE
#define _UNICODE
#include <windows.h> // Include Windows API functions
#include <psapi.h> // Include functions for process management
#include <vector> // Include STL vector library
#include <string> // Include STL string library
#include <unordered_map> // Include STL hashmap library
#include <algorithm> // Include STL algorithms library
#include <iostream> // Include input/output functions
#include "resource.h" // Include resource header file
#include <cmath> // Include mathematical functions
#include <windowsx.h> // Include Windowsx header file, which defines macros like GET_X_LPARAM and GET_Y_LPARAM
#include <cctype>
#include <commctrl.h>
#include <map>
#include <shellapi.h>

#pragma comment(lib, "psapi.lib") // Link psapi.lib library

#define WM_TRAYICON (WM_USER + 1) // Define a custom message for the tray icon
#define WM_UPDATE_LIST (WM_USER + 2) // Define a custom message to update the window list
#define ID_MINIMIZE 2000
#define ID_RESTORE  2001
#define ID_CLOSE    2002
#define ID_ARRANGE  2003 // Needs 2003-200x for enumerate all screens
#define ID_MAXIMIZE 2050
#define ID_MOVE_TO_SCREEN_BASE 2104
#define IDC_SEARCHBOX 101
#define IDC_ERASEBUTTON 201


// Structure to store window information
struct WindowInfo {
    HWND hwnd; // Handle of the window
    std::wstring title; // Title of the window
    std::wstring processName; // Name of the associated process
    std::wstring exePath;
    bool checked; // Status indicating if the window is selected
    bool arranged; 
    bool visible; 
};

struct MonitorInfo {
    int index;
    RECT rect;
    std::wstring position;
};


// Function declarations
BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam); // Callback function to list open windows
std::vector<WindowInfo> getOpenWindows(); // Function to return open windows
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam); // Window procedure to handle messages
void ShowLastError(LPCSTR message); // Function to display the last error
void CreateTrayIcon(HWND hwnd); // Function to create the tray icon
void RemoveTrayIcon(HWND hwnd); // Function to remove the tray icon
void ShowTrayMenu(HWND hwnd); // Function to display the tray menu
void UpdateWindowList(HWND hwnd); // Function to update the window list
void AdjustWindowSize(HWND hwnd); // Function to adjust the window size
void MinimizeToTray(HWND hwnd); // Function to minimize the window to the tray
void ProcessMessages(); // Function to process messages
void SaveCurrentWindows(); // Function to save the current windows
bool HasWindowsChanged(); // Function to check if the windows have changed
bool ConfirmClose(HWND hwnd); // Function to confirm closing windows
RECT GetScreenRect(int screenIndex);
BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData);

// Global variables
static std::unordered_map<std::wstring, std::vector<WindowInfo>> processWindowsMap; // Hashmap to store windows by processes
static std::unordered_map<std::wstring, bool> expandedState; // Hashmap to store the expanded state of processes
static std::vector<std::wstring> processNames; // Vector to store process names
static std::unordered_map<std::wstring, bool> checkboxState; // Hashmap to store the checkbox state
//static HWND closeButton; // Handle of the close button
//static HWND arrangeButton; // Handle of the arrange button
//static HWND minimizeButton; // Handle of the minimize button
//static HWND restoreButton; // Handle of the restore button
std::vector<WindowInfo> currentWindows; // Vector to store current windows
static bool initialized = false; // Status indicating if the application is initialized
//static std::unordered_map<std::wstring, HWND> expandButtons; // Hashmap to store expand buttons
static HWND whiteBar; // Handle of the white bar
static bool isScrolling = false; // Variable to track if scrolling is in progress
static POINT lastMousePos = {0, 0}; // Variable to store the last mouse position
static std::map<std::wstring, HICON> processIcons; // Map for prozess icons
std::unordered_map<std::wstring, int> buttonPositions;
int highlightedRow = -1; // Globale Variable zur Speicherung der hervorgehobenen Zeile
int highlightedWindowRow = -1; // Globale Variable zur Speicherung der hervorgehobenen Fensterzeile
bool isRedrawPending = false;
int screenCount = 1;
int textWidth = 700;
HWND hwndTT;
HWND hSearchBox;
HWND hEraseButton;
HMENU hMenu;

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam) {
    wchar_t title[256]; // Buffer für den Fenstertitel
    DWORD processId; // Prozess-ID
    GetWindowThreadProcessId(hwnd, &processId); // Prozess-ID des Fensters abrufen
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processId); // Prozess öffnen
    if (hProcess) {
        wchar_t exePath[MAX_PATH]; // Buffer für den Pfad zur ausführbaren Datei
        if (GetModuleFileNameExW(hProcess, NULL, exePath, sizeof(exePath) / sizeof(wchar_t))) { // Pfad zur ausführbaren Datei abrufen
            wchar_t processName[MAX_PATH]; // Buffer für den Prozessnamen
            if (GetModuleBaseNameW(hProcess, NULL, processName, sizeof(processName) / sizeof(wchar_t))) { // Prozessnamen abrufen
                // Liste der auszuschließenden Prozesse, die von Windows erstellt werden und für den Benutzer nicht nützlich sind
                std::vector<std::wstring> excludedProcesses = {
                    L"TextInputHost.exe",
                    L"SearchUI.exe",
                    L"ShellExperienceHost.exe",
                    L"SystemSettings.exe",
                    L"ApplicationFrameHost.exe",
                    L"RuntimeBroker.exe",
                    L"sihost.exe",
                    L"dwm.exe",
                    L"smartscreen.exe",
                    L"window_minimizer.exe"
                };
                std::wstring processNameStr(processName);
                if (processNameStr.find(L"CodeSetup") == 0 && processNameStr.rfind(L"tmp") == processNameStr.length() - 3) {
                    CloseHandle(hProcess); // Prozesshandle schließen
                    return TRUE; // Prozess ausschließen und Enumeration fortsetzen
                }
                if (IsWindowVisible(hwnd) && std::find(excludedProcesses.begin(), excludedProcesses.end(), processName) == excludedProcesses.end()) { // Überprüfen, ob das Fenster sichtbar und nicht ausgeschlossen ist
                    int length = GetWindowTextW(hwnd, title, sizeof(title) / sizeof(wchar_t)); // Fenstertitel abrufen
                    if (length > 0 && wcscmp(title, L"Program Manager") != 0) { // "Program Manager" ausschließen
                        if (processNameStr.length() > 4 && 
                            (processNameStr.substr(processNameStr.length() - 4) == L".exe" || 
                             processNameStr.substr(processNameStr.length() - 4) == L".EXE")) {
                            processNameStr = processNameStr.substr(0, processNameStr.length() - 4); // ".exe" oder ".EXE" entfernen
                        }
                        std::vector<WindowInfo>* windows = reinterpret_cast<std::vector<WindowInfo>*>(lParam); // lParam in einen Vektor von WindowInfo umwandeln
                        windows->emplace_back(WindowInfo{ hwnd, title, processNameStr, exePath, false }); // Fenster zur Liste hinzufügen
                    }
                }
            }
        }
        CloseHandle(hProcess); // Prozesshandle schließen
    }
    return TRUE; // Enumeration fortsetzen
}

// Function to return open windows
std::vector<WindowInfo> getOpenWindows() {
    std::vector<WindowInfo> windows; // Vector to store windows
    EnumWindows(EnumWindowsProc, reinterpret_cast<LPARAM>(&windows)); // Enumerate windows
    return windows; // Return the list of windows
}

// Function to minimize the window to the tray
void MinimizeToTray(HWND hwnd) {
    ShowWindow(hwnd, SW_HIDE); // Hide the window
}

// Function to create the tray icon
void CreateTrayIcon(HWND hwnd) {
    NOTIFYICONDATAW nid = {}; // Structure for the tray icon
    nid.cbSize = sizeof(nid); // Size of the structure
    nid.hWnd = hwnd; // Handle of the window
    nid.uID = 1; // ID of the tray icon
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP; // Flags for the tray icon
    nid.uCallbackMessage = WM_TRAYICON; // Message for the tray icon
    nid.hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_MYICON)); // Load the icon
    wcscpy_s(nid.szTip, sizeof(nid.szTip) / sizeof(wchar_t), L"Minimize, Maximize, Restore, Close, Arrange or Move Many Windows at Once"); // Tooltip for the tray icon
    Shell_NotifyIconW(NIM_ADD, &nid); // Add the tray icon
}

// Function to remove the tray icon
void RemoveTrayIcon(HWND hwnd) {
    NOTIFYICONDATAW nid = {}; // Structure for the tray icon
    nid.cbSize = sizeof(nid); // Size of the structure
    nid.hWnd = hwnd; // Handle of the window
    nid.uID = 1; // ID of the tray icon
    Shell_NotifyIconW(NIM_DELETE, &nid); // Remove the tray icon
}

// Function to display the tray menu
void ShowTrayMenu(HWND hwnd) {
    POINT pt; // Structure for the cursor position
    GetCursorPos(&pt); // Get the cursor position
    SetForegroundWindow(hwnd); // Bring the window to the foreground
    HMENU hMenu = CreatePopupMenu(); // Create the popup menu
    AppendMenuW(hMenu, MF_STRING, ID_TRAY_EXIT, L"Exit"); // Add a menu item
    TrackPopupMenu(hMenu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hwnd, NULL); // Display the menu
    DestroyMenu(hMenu); // Destroy the menu
}

// Function to process messages
void ProcessMessages() {
    MSG msg; // Structure for the message
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) { // Retrieve messages
        TranslateMessage(&msg); // Translate the message
        DispatchMessage(&msg); // Dispatch the message
    }
}

// Function to save the current windows
void SaveCurrentWindows() {
    currentWindows = getOpenWindows(); // Save the current windows
}

bool CompareMonitors(const MonitorInfo& a, const MonitorInfo& b) {
    return a.rect.left < b.rect.left;
}

RECT GetScreenRect(int screenIndex) {
    std::vector<MonitorInfo> monitors;
    EnumDisplayMonitors(NULL, NULL, MonitorEnumProc, (LPARAM)&monitors);

    if (screenIndex < 0 || screenIndex >= monitors.size()) {
        //MessageBox(NULL, "Ungueltiger Bildschirm-Index", "Fehler", MB_OK | MB_ICONERROR);
        return {0, 0, 0, 0};
    }

    return monitors[screenIndex].rect;
}

BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData) {
    std::vector<MonitorInfo>* monitors = reinterpret_cast<std::vector<MonitorInfo>*>(dwData);
    
    MONITORINFOEX mi;
    mi.cbSize = sizeof(mi);
    if (GetMonitorInfo(hMonitor, &mi)) {
        monitors->push_back({static_cast<int>(monitors->size()), mi.rcMonitor, L""});
    }
    return TRUE;
}

void MoveWindowToScreen(HWND hwnd, int screenIndex) {
    if (!IsWindow(hwnd)) {
        //MessageBox(NULL, "Ungueltiges Fenster-Handle", "Fehler", MB_OK | MB_ICONERROR);
        return;
    }

    WINDOWPLACEMENT wp;
    wp.length = sizeof(WINDOWPLACEMENT);
    if (!GetWindowPlacement(hwnd, &wp)) {
        //MessageBox(NULL, "Fehler beim Abrufen der Fensterplatzierung", "Fehler", MB_OK | MB_ICONERROR);
        return;
    }

    bool wasMaximized = (wp.showCmd == SW_SHOWMAXIMIZED);

    if (wasMaximized) {
        ShowWindow(hwnd, SW_RESTORE);
    }

    RECT screenRect = GetScreenRect(screenIndex);
    if (screenRect.left == 0 && screenRect.top == 0 && screenRect.right == 0 && screenRect.bottom == 0) {
        //MessageBox(NULL, "Ungueltiger Bildschirm-Index", "Fehler", MB_OK | MB_ICONERROR);
        return;
    }

    if (!SetWindowPos(hwnd, NULL, screenRect.left, screenRect.top, screenRect.right - screenRect.left, screenRect.bottom - screenRect.top, SWP_NOZORDER | SWP_SHOWWINDOW)) {
        //MessageBox(NULL, "Fehler beim Verschieben des Fensters", "Fehler", MB_OK | MB_ICONERROR);
        return;
    }

    if (wasMaximized) {
        //ShowWindow(hwnd, SW_MAXIMIZE);
    }
}


HBITMAP CaptureAndResizeScreen(HWND hwnd, RECT rect, int width, int height) {
    HDC hdcScreen = GetDC(NULL);
    HDC hdcMem = CreateCompatibleDC(hdcScreen);
    HBITMAP hbmScreen = CreateCompatibleBitmap(hdcScreen, rect.right - rect.left, rect.bottom - rect.top);
    SelectObject(hdcMem, hbmScreen);
    BitBlt(hdcMem, 0, 0, rect.right - rect.left, rect.bottom - rect.top, hdcScreen, rect.left, rect.top, SRCCOPY);

    HBITMAP hbmResized = CreateCompatibleBitmap(hdcScreen, width, height);
    HDC hdcResized = CreateCompatibleDC(hdcScreen);
    SelectObject(hdcResized, hbmResized);
    SetStretchBltMode(hdcResized, HALFTONE);
    StretchBlt(hdcResized, 0, 0, width, height, hdcMem, 0, 0, rect.right - rect.left, rect.bottom - rect.top, SRCCOPY);

    // Draw a blue border around the resized image
    HPEN hPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 255)); // Create a blue pen with a width of 1
    HGDIOBJ hOldPen = SelectObject(hdcResized, hPen);
    HGDIOBJ hOldBrush = SelectObject(hdcResized, GetStockObject(NULL_BRUSH));
    Rectangle(hdcResized, 0, 0, width, height); // Draw the rectangle
    SelectObject(hdcResized, hOldPen);
    SelectObject(hdcResized, hOldBrush);
    DeleteObject(hPen);

    DeleteDC(hdcMem);
    DeleteDC(hdcResized);
    ReleaseDC(NULL, hdcScreen);
    DeleteObject(hbmScreen);

    return hbmResized;
}

void AddMenuItemWithImage(HMENU hMenu, UINT uIDNewItem, HBITMAP hBitmap, const std::wstring& text) {
    MENUITEMINFOW mii = { sizeof(MENUITEMINFOW) };
    mii.fMask = MIIM_BITMAP | MIIM_STRING | MIIM_ID;
    mii.wID = uIDNewItem;
    mii.dwTypeData = const_cast<LPWSTR>(text.c_str());
    mii.cch = static_cast<UINT>(text.size());
    mii.hbmpItem = hBitmap;
    InsertMenuItemW(hMenu, uIDNewItem, FALSE, &mii);
}

void CreateMoveToScreenMenu(HMENU hMenu) {
    HMENU hMoveToScreenMenu = CreateMenu();
    std::vector<MonitorInfo> monitors;
    EnumDisplayMonitors(NULL, NULL, MonitorEnumProc, reinterpret_cast<LPARAM>(&monitors));
    if (monitors.size() > 1) { // Check if more than one screen is present
        screenCount = -1;
        for (const auto& monitor : monitors) {
            screenCount++;
            int width = monitor.rect.right - monitor.rect.left;
            int height = monitor.rect.bottom - monitor.rect.top;
            std::wstring menuText = L"Screen " + std::to_wstring(monitor.index) + L" (" + 
                                    std::to_wstring(width) + L"x" + 
                                    std::to_wstring(height) + L")";

            // Berechne das neue Seitenverhältnis mit fester Höhe von 25 Pixel
            int newHeight = 25;
            int newWidth = static_cast<int>(25.0 * width / height);

            HBITMAP hBitmap = CaptureAndResizeScreen(NULL, monitor.rect, newWidth, newHeight);
            AddMenuItemWithImage(hMoveToScreenMenu, ID_MOVE_TO_SCREEN_BASE + monitor.index, hBitmap, menuText);
        }
        AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hMoveToScreenMenu, L"Mo&ve Window(s)");
    } else {
        //Move to the same (and only) screen does not make sense, so no menu
        //AppendMenu(hMenu, MF_STRING, ID_MOVE_TO_SCREEN_BASE, L"Mo&ve Window(s)");
    }
}

// Funktion zum Trimmen von Leerzeichen
std::wstring trim(const std::wstring& str) {
    size_t first = str.find_first_not_of(L' ');
    if (first == std::wstring::npos) return L"";
    size_t last = str.find_last_not_of(L' ');
    return str.substr(first, last - first + 1);
}

// Funktion zum Umwandeln von Großbuchstaben in nur den ersten Großbuchstaben
std::wstring capitalizeIfAllCaps(const std::wstring& str) {
    bool allCaps = true;
    for (wchar_t c : str) {
        if (!std::iswupper(c)) {
            allCaps = false;
            break;
        }
    }
    if (allCaps) {
        std::wstring result = str;
        for (wchar_t& c : result) {
            c = std::towlower(c);
        }
        result[0] = std::towupper(result[0]);
        return result;
    }
    return str;
}

std::wstring toLower(const std::wstring& str) {
    std::wstring lowerStr = str;
    std::transform(lowerStr.begin(), lowerStr.end(), lowerStr.begin(), ::towlower);
    return lowerStr;
}
void CreateArrangeOnScreenMenu(HMENU hMenu) {
    std::vector<MonitorInfo> monitors;
    EnumDisplayMonitors(NULL, NULL, MonitorEnumProc, reinterpret_cast<LPARAM>(&monitors));

    HMENU hArrangeOnScreenMenu = CreateMenu();
    if (monitors.size() > 1) { // Check if more than one screen is present
        screenCount = -1;

        for (const auto& monitor : monitors) {
            screenCount++;
            int width = monitor.rect.right - monitor.rect.left;
            int height = monitor.rect.bottom - monitor.rect.top;
            std::wstring menuText = L"Screen " + std::to_wstring(monitor.index) + L" (" + 
                                    std::to_wstring(width) + L"x" + 
                                    std::to_wstring(height) + L")";

            // Berechne das neue Seitenverhältnis
            int newWidth, newHeight;
            if (width > height) {
                newWidth = 25;
                newHeight = static_cast<int>(30.0 * height / width);
            } else {
                newHeight = 25;
                newWidth = static_cast<int>(30.0 * width / height);
            }

            HBITMAP hBitmap = CaptureAndResizeScreen(NULL, monitor.rect, newWidth, newHeight);
            AddMenuItemWithImage(hArrangeOnScreenMenu, ID_ARRANGE + monitor.index, hBitmap, menuText);
        }
        AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hArrangeOnScreenMenu, L"&Arrange Window(s)");
    } else {
        AppendMenu(hMenu, MF_STRING, ID_ARRANGE, L"&Arrange Window(s)");
    }
}

// Function to check if the windows have changed
bool HasWindowsChanged() {
    // Retrieve the current open windows
    auto newWindows = getOpenWindows();
    
    // Check if the number of windows is different
    if (newWindows.size() != currentWindows.size()) {
        return true; // Number of windows has changed
    }
    
    // Check if any of the windows have changed
    for (size_t i = 0; i < newWindows.size(); ++i) {
        // Compare window handles, titles, and process names
        if (newWindows[i].hwnd != currentWindows[i].hwnd || 
            newWindows[i].title != currentWindows[i].title || 
            newWindows[i].processName != currentWindows[i].processName) {
            return true; // A window has changed
        }
    }
    
    return false; // No changes detected
}

// Function to move the window to the main monitor
void MoveWindowToMainMonitor(HWND hwnd) {
    //MessageBoxW(hwnd, L"aha1", L"Debug Info", MB_OK);
    WINDOWPLACEMENT wp;     // Überprüfe, ob das Fenster maximiert ist
    wp.length = sizeof(WINDOWPLACEMENT);
    GetWindowPlacement(hwnd, &wp);

    bool wasMaximized = (wp.showCmd == SW_SHOWMAXIMIZED);

    if (wasMaximized) {
        ShowWindow(hwnd, SW_RESTORE); // Fenster wiederherstellen, wenn es maximiert ist
    }

    // Get the handle to the primary monitor
    HMONITOR hMonitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTOPRIMARY);

    // Get the monitor info
    MONITORINFO mi = { sizeof(mi) };
    if (GetMonitorInfo(hMonitor, &mi)) {
        // Calculate the new position for the window
        int newX = mi.rcWork.left;
        int newY = mi.rcWork.top;

        // Move the window to the new position
        SetWindowPos(hwnd, HWND_TOP, newX, newY, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW);
        //MessageBoxW(hwnd, L"aha", L"Debug Info", MB_OK);
    }

    
    if (wasMaximized) {
        // Fenster wieder maximieren
        ShowWindow(hwnd, SW_MAXIMIZE);
    }
}

// Function to allow sorting case-insensitive
bool caseInsensitiveCompare(const std::wstring& a, const std::wstring& b) {
    std::wstring lowerA = a;
    std::wstring lowerB = b;
    std::transform(lowerA.begin(), lowerA.end(), lowerA.begin(), ::towlower);
    std::transform(lowerB.begin(), lowerB.end(), lowerB.begin(), ::towlower);
    return lowerA < lowerB;
}

void SetEditPlaceholder(HWND hwndEdit, const std::wstring& placeholder) {
    SetWindowText(hwndEdit, placeholder.c_str());
    SendMessage(hwndEdit, EM_SETSEL, 0, -1); // Markiere den gesamten Text
    SendMessage(hwndEdit, EM_SETSEL, -1, -1); // Entferne die Markierung
}

// Function to allow on-mouse-over tooltip
HWND CreateTooltip(HWND hwndParent) {
    HWND hwndTT = CreateWindowEx(WS_EX_TOPMOST, TOOLTIPS_CLASS, NULL,
        WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP,        
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        hwndParent, NULL, GetModuleHandle(NULL), NULL);

    SetWindowPos(hwndTT, HWND_TOPMOST, 0, 0, 0, 0,
        SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

    return hwndTT;
}

// Function to update the window list
void UpdateWindowList(HWND hwnd) {
    // Fensterinhalt löschen
    RECT rect;
    GetClientRect(hwnd, &rect);
    HDC hdc = GetDC(hwnd);
    HBRUSH hBrush = (HBRUSH)GetStockObject(WHITE_BRUSH);
    FillRect(hdc, &rect, hBrush);
    ReleaseDC(hwnd, hdc);

    // Alte Steuerelemente zerstören
    /*for (auto& button : expandButtons) {
        DestroyWindow(button.second);
    }*/
    //expandButtons.clear();

    /*if (whiteBar) {
        DestroyWindow(whiteBar);
        whiteBar = NULL;
    }
    if (minimizeButton) {
        DestroyWindow(minimizeButton);
        minimizeButton = NULL;
    }
    if (restoreButton) {
        DestroyWindow(restoreButton);
        restoreButton = NULL;
    }
    if (closeButton) {
        DestroyWindow(closeButton);
        closeButton = NULL;
    }
    if (arrangeButton) {
        DestroyWindow(arrangeButton);
        arrangeButton = NULL;
    }*/

    // Fenster- und Zustandskarten löschen
    processWindowsMap.clear();
    processNames.clear();
    checkboxState.clear();
    expandedState.clear();

    // Fenster abrufen
    auto windows = getOpenWindows();
    if (windows.empty()) {
        // MessageBoxW(hwnd, L"No windows found", L"Debug Info", MB_OK);
    }

    // Fenster den jeweiligen Prozessen zuordnen
    for (auto& window : windows) {
        processWindowsMap[window.processName].push_back(window);
        expandedState[window.processName] = false;
        checkboxState[window.processName] = false;
    }

    // Prozessnamen in eine Liste einfügen
    for (const auto& entry : processWindowsMap) {
        processNames.push_back(entry.first);
    }

    // Prozessnamen alphabetisch sortieren
    std::sort(processNames.begin(), std::end(processNames), caseInsensitiveCompare);

    // Scrollinformationen setzen
    SCROLLINFO si = {};
    si.cbSize = sizeof(si);
    si.fMask = SIF_RANGE | SIF_PAGE;
    si.nMin = 0;
    si.nMax = processNames.size() * 30;
    si.nPage = 100;
    SetScrollInfo(hwnd, SB_VERT, &si, TRUE);

    // Schaltflächen für die Prozesse erstellen
    int yPos = 60;
    /*for (size_t i = 0; i < processNames.size(); ++i) {
        HWND expandButton = CreateWindowExW(
            0,
            L"BUTTON",
            L">",
            WS_VISIBLE | WS_CHILD | BS_OWNERDRAW,
            10, yPos, 30, 30,
            hwnd,
            (HMENU)(3000 + i),
            GetModuleHandle(NULL),
            NULL
        );
        expandButtons[processNames[i]] = expandButton;
        yPos += 30;
    }*/

    /*// Weiße Leiste am unteren Rand des Fensters erstellen
    GetClientRect(hwnd, &rect);
    int scrollbarWidth = GetSystemMetrics(SM_CXVSCROLL);
    int width = rect.right - rect.left - scrollbarWidth;
    whiteBar = CreateWindowExW(
        0,
        L"STATIC",
        NULL,
        WS_VISIBLE | WS_CHILD | SS_WHITERECT,
        0, rect.bottom - 55, width + scrollbarWidth, 55,
        hwnd,
        NULL,
        GetModuleHandle(NULL),
        NULL
    );

    // Schaltflächen für Minimieren, Wiederherstellen, Schließen und Anordnen erstellen
    int buttonCount = 4;
    int buttonWidth = (width - 20) / buttonCount;

    minimizeButton = CreateWindowW(
        L"BUTTON",
        L"Minimize\nWindow(s)",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON | BS_MULTILINE,
        10, rect.bottom - 50, buttonWidth, 40,
        hwnd,
        (HMENU)2000,
        GetModuleHandle(NULL),
        NULL
    );

    restoreButton = CreateWindowW(
        L"BUTTON",
        L"Restore\nWindow(s)",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON | BS_MULTILINE,
        10 + buttonWidth + 5, rect.bottom - 50, buttonWidth, 40,
        hwnd,
        (HMENU)2001,
        GetModuleHandle(NULL),
        NULL
    );

    closeButton = CreateWindowW(
        L"BUTTON",
        L"Close\nWindow(s)",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON | BS_MULTILINE,
        10 + 2 * (buttonWidth + 5), rect.bottom - 50, buttonWidth, 40,
        hwnd,
        (HMENU)2002,
        GetModuleHandle(NULL),
        NULL
    );

    arrangeButton = CreateWindowW(
        L"BUTTON",
        L"Arrange\nWindow(s)",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON | BS_MULTILINE,
        10 + 3 * (buttonWidth + 5), rect.bottom - 50, buttonWidth, 40,
        hwnd,
        (HMENU)2003,
        GetModuleHandle(NULL),
        NULL
    );*/

    // Aktuelle Fenster speichern und Fenstergröße anpassen
    SaveCurrentWindows();
    AdjustWindowSize(hwnd);

    // Icons für die Prozesse laden und in der Map speichern
    HINSTANCE hInstance = (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE);
    for (const auto& processName : processNames) {
        auto it = windows.end();
        for (auto winIt = windows.begin(); winIt != windows.end(); ++winIt) {
            if (winIt->processName == processName) {
                it = winIt;
                break;
            }
        }
        if (it != windows.end()) {
            HICON hIcon = ExtractIconW(hInstance, it->exePath.c_str(), 0);
            if (hIcon == NULL) {
                hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MYICON)); // Verwende das Standard-Icon, wenn kein spezifisches Icon gefunden wird
            }
            processIcons[processName] = hIcon;
        }
    }
}

// Function to adjust the window size
void AdjustWindowSize(HWND hwnd) {
    RECT rect; // Declaration of a RECT structure to store the window size
    GetWindowRect(hwnd, &rect);
    int xPos = rect.left;
    int yPos = rect.top;
    //GetClientRect(hwnd, &rect); // Retrieve the client rectangles of the window
    int width = rect.right - rect.left; // Calculate the window width
    int height = rect.bottom - rect.top; // Calculate the window height
    SCROLLINFO si = {}; // Initialize a SCROLLINFO structure
    si.cbSize = sizeof(si); // Set the size of the SCROLLINFO structure
    si.fMask = SIF_RANGE | SIF_PAGE; // Specify the masks to use
    GetScrollInfo(hwnd, SB_VERT, &si); // Retrieve the scroll information of the window
    int screenHeight = GetSystemMetrics(SM_CYSCREEN); // Retrieve the screen height
    int titleBarHeight = GetSystemMetrics(SM_CYCAPTION); // Retrieve the title bar height
    int usableScreenHeight = screenHeight - titleBarHeight - 25 - 50; // Calculate the usable screen height (minus 50px to not be straight up to the bottom of the screen)
    int contentHeight = si.nMax + 30 + 40; // Calculate the content height
    int contentWidth = textWidth; // Set the content width
    int newHeight = std::min(contentHeight, usableScreenHeight); // Calculate the new window height

    // Calculate the new y-position
    int newYPos = yPos - (newHeight - height);
    if (newYPos < 0) {
        newYPos = 0; // Ensure the window is not moved above the top of the screen
        newHeight = screenHeight - titleBarHeight - 25 - 50; // Adjust the height to fit the screen
    }

    SetWindowPos(hwnd, NULL, xPos, newYPos, contentWidth, newHeight, SWP_NOZORDER); // Set the new window position and size
}

void InitializeMenu(HWND hwnd) {
    hMenu = CreateMenu();
    AppendMenu(hMenu, MF_STRING, ID_MINIMIZE, L"&Minimize Window(s)");
    AppendMenu(hMenu, MF_STRING, ID_MAXIMIZE, L"Ma&ximize Window(s)");
    AppendMenu(hMenu, MF_STRING, ID_RESTORE, L"&Restore Window(s)");

    // Füge die dynamischen Einträge direkt hinzu
    CreateArrangeOnScreenMenu(hMenu);
    CreateMoveToScreenMenu(hMenu);

    AppendMenu(hMenu, MF_STRING, ID_CLOSE, L"&Close Window(s)");
    SetMenu(hwnd, hMenu);
}

void UpdateDynamicMenus(HWND hwnd) {
    // Entferne die alten dynamischen Einträge
    // Hier musst du die Positionen der dynamischen Einträge kennen
    // Angenommen, sie sind an den Positionen 3 und 4
    /*RemoveMenu(hMenu, 3, MF_BYPOSITION);
    RemoveMenu(hMenu, 3, MF_BYPOSITION); // Da die Positionen sich verschieben
    RemoveMenu(hMenu, 3, MF_BYPOSITION);
    RemoveMenu(hMenu, 3, MF_BYPOSITION);
    RemoveMenu(hMenu, 3, MF_BYPOSITION);*/

    HMENU hMenu = GetMenu(hwnd);
    DestroyMenu(hMenu);
    SetMenu(hwnd, NULL);
    InitializeMenu(hwnd);
}

void InvalidateWindow(HWND hwnd) {
    /*if (!isRedrawPending) {
        isRedrawPending = true;
        InvalidateWindow(hwnd);
    }*/
   InvalidateRect(hwnd, NULL, TRUE);
}

void RefreshWindowList(HWND hwnd) {
    // Invalidate and redraw the window immediately
    RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE); // Invalidate and redraw

    // Process messages
    ProcessMessages(); // Process messages

    // Update the window list
    UpdateWindowList(hwnd); // Update the window list
}

bool compareWindowsByName(const WindowInfo& a, const WindowInfo& b) {
    std::wstring nameA = a.title;
    std::wstring nameB = b.title;
    std::transform(nameA.begin(), nameA.end(), nameA.begin(), ::tolower);
    std::transform(nameB.begin(), nameB.end(), nameB.begin(), ::tolower);
    return nameA < nameB; // Sort windows by window name case-insensitive
}

// Function to confirm closing windows
bool ConfirmClose(HWND hwnd) {
    int result = MessageBoxW(hwnd, L"Do you really want to close the selected windows?", L"Yes", MB_YESNO | MB_ICONQUESTION | MB_TOPMOST); // Display confirmation dialog
    return (result == IDYES); // Return whether the user chose "Yes"
}

void SearchAndCheck(const std::wstring& searchString, HWND hwnd) {
    std::wstring lowerSearchString = toLower(searchString);

    // Setze alle Checkboxen auf "unchecked" und klappe alle Prozesse zu
    for (auto& processName : processNames) {
        //checkboxState[processName] = false;
        expandedState[processName] = false; // Klappe alle Prozesse zu
        for (auto& window : processWindowsMap[processName]) {
            //window.checked = false;
            window.visible = false; // Setze das sichtbare Attribut auf false
        }
    }

    // Führe die Suche durch und setze passende Einträge auf "checked"
    if (lowerSearchString.length() >= 1) {
        for (auto& processName : processNames) {
            std::wstring lowerProcessName = toLower(processName);
            bool processMatch = lowerProcessName.find(lowerSearchString) != std::wstring::npos;
            if (processMatch) {
                //checkboxState[processName] = true;
            }

            for (auto& window : processWindowsMap[processName]) {
                std::wstring lowerWindowTitle = toLower(window.title);
                bool windowMatch = lowerWindowTitle.find(lowerSearchString) != std::wstring::npos;
                if (windowMatch) {
                    //window.checked = true;
                    window.visible = true; // Setze das sichtbare Attribut auf true, wenn es übereinstimmt
                    expandedState[processName] = true; // Klappe den Prozess auf, wenn ein Fenster übereinstimmt
                }
                if (window.checked)
                    expandedState[processName] = true;
            }
        }
    }

    // Scroll-Informationen aktualisieren
    SCROLLINFO si = {}; // Initialize a SCROLLINFO structure
    si.cbSize = sizeof(si); // Set the size of the SCROLLINFO structure
    si.fMask = SIF_RANGE | SIF_PAGE; // Specify the masks to use
    si.nMin = 0; // Set the minimum scroll range
    si.nMax = 0; // Initialize the maximum scroll range
    for (const auto& processName : processNames) { // Iterate through all process names
        si.nMax += 30; // Increment the maximum scroll range for each process
        if (expandedState[processName]) { // Check if the process is expanded
            for (const auto& window : processWindowsMap[processName]) {
                if (window.visible) {
                    si.nMax += 30; // Increment the maximum scroll range based on the number of visible windows
                }
            }
        }
    }
    si.nPage = 100; // Set the page length for scrolling
    si.nPos = 0;
    SetScrollInfo(hwnd, SB_VERT, &si, TRUE); // Set the scroll information for the vertical scrollbar
    InvalidateRect(hwnd, NULL, TRUE); // Invalidate and redraw the window
    AdjustWindowSize(hwnd); // Adjust the window size
    InvalidateWindow(hwnd); // Invalidate and redraw the window
    //InvalidateRect(hwnd, NULL, TRUE);
}

void SearchAndCheckErase(HWND hwnd) {
    for (size_t i = 0; i < processNames.size(); ++i) {
        const auto& processName = processNames[i];
        bool AlreadyOneChecked = false;
        for (auto& window : processWindowsMap[processName]) { // Iterate through all windows of the process
            if (window.checked == true) AlreadyOneChecked = true;
        }
        if (AlreadyOneChecked) expandedState[processName] = true;
    }

    // Scroll-Informationen aktualisieren
    SCROLLINFO si = {}; // Initialize a SCROLLINFO structure
    si.cbSize = sizeof(si); // Set the size of the SCROLLINFO structure
    si.fMask = SIF_RANGE | SIF_PAGE; // Specify the masks to use
    si.nMin = 0; // Set the minimum scroll range
    si.nMax = 0; // Initialize the maximum scroll range
    for (const auto& processName : processNames) { // Iterate through all process names
        si.nMax += 30; 
        if (expandedState[processName]) { // Check if the process is expanded
            si.nMax += 30; // Increment the maximum scroll range for each process
            for (const auto& window : processWindowsMap[processName]) {
                if (window.visible) {
                    si.nMax += 30; // Increment the maximum scroll range based on the number of visible windows
                }
            }
        }
    }
    si.nPage = 100; // Set the page length for scrolling
    si.nPos = 0;
    SetScrollInfo(hwnd, SB_VERT, &si, TRUE); // Set the scroll information for the vertical scrollbar
    InvalidateRect(hwnd, NULL, TRUE); // Invalidate and redraw the window
    AdjustWindowSize(hwnd); // Adjust the window size
    InvalidateWindow(hwnd); // Invalidate and redraw the window
    //InvalidateRect(hwnd, NULL, TRUE);
}

// Window procedure
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static int scrollPos = 0; // Static variable to store the scroll position
    int id; // Variable to store the ID
    static HIMAGELIST hImageList;
    static HICON hIcon1;
    static HMENU hMenu = NULL;

    switch (uMsg) { // Check the messages
        case WM_CREATE: {
            CreateTrayIcon(hwnd);
            if (!initialized) {
                UpdateWindowList(hwnd);
                initialized = true;
                MinimizeToTray(hwnd);
            }
            RECT clientRect;
            GetClientRect(hwnd, &clientRect);
            int searchBoxWidth = 150; // Breite des Suchfelds
            int searchBoxHeight = 20; // Höhe des Suchfelds
            int searchBoxX = clientRect.right - searchBoxWidth - 30; // Position X des Suchfelds
            int searchBoxY = 5; // Position Y des Suchfelds

            // Erstellen Sie das Suchfeld
            hSearchBox = CreateWindowEx(0, TEXT("EDIT"), NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL, 
                                        searchBoxX, searchBoxY, searchBoxWidth, searchBoxHeight, hwnd, (HMENU)IDC_SEARCHBOX, 
                                        ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            SendMessage(hSearchBox, EM_SETLIMITTEXT, 10, 0); // Begrenze die Eingabe auf 10 Zeichen
            SetEditPlaceholder(hSearchBox, L"Search name"); // Setze den Hint

            // Erstellen Sie den "Erase"-Button
            hEraseButton = CreateWindowEx(0, TEXT("BUTTON"), TEXT("X"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 
                                            searchBoxX + searchBoxWidth, searchBoxY, 20, searchBoxHeight, hwnd, (HMENU)IDC_ERASEBUTTON, 
                                            ((LPCREATESTRUCT)lParam)->hInstance, NULL);

            InitializeMenu(hwnd);
            /*HMENU hMenu = CreateMenu();

            AppendMenu(hMenu, MF_STRING, ID_MINIMIZE, L"&Minimize Window(s)");
            AppendMenu(hMenu, MF_STRING, ID_MAXIMIZE, L"Ma&ximize Window(s)");
            AppendMenu(hMenu, MF_STRING, ID_RESTORE, L"&Restore Window(s)");
            AppendMenu(hMenu, MF_STRING, ID_CLOSE, L"&Close Window(s)");
            CreateArrangeOnScreenMenu(hMenu); // Arrange
            CreateMoveToScreenMenu(hMenu); // Move

            SetMenu(hwnd, hMenu);*/

            SCROLLINFO si = {};
            si.cbSize = sizeof(si);
            si.fMask = SIF_RANGE | SIF_PAGE;
            si.nMin = 0;
            si.nMax = processNames.size() * 30;
            si.nPage = 100;
            SetScrollInfo(hwnd, SB_VERT, &si, TRUE);
            scrollPos = 0;

            HINSTANCE hInstance = (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE);
            hIcon1 = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MYICON));

            auto windows = getOpenWindows();
            // Lade Icons für jeden Prozess und speichere sie in der Map
            for (const auto& processName : processNames) {
                auto it = windows.end();
                for (auto winIt = windows.begin(); winIt != windows.end(); ++winIt) {
                    if (winIt->processName == processName) {
                        it = winIt;
                        break;
                    }
                }
                if (it != windows.end()) {
                    HICON hIcon = ExtractIconW(hInstance, it->exePath.c_str(), 0);
                    if (hIcon == NULL) {
                        hIcon = hIcon1;
                    }
                    processIcons[processName] = hIcon;
                }
            }
        }
        //InvalidateWindow(hwnd);
        break;

        case WM_TIMER: {
            if (wParam == 1) {
                DestroyWindow(hwnd);
            }
        }
        //InvalidateWindow(hwnd);
        break;

        case WM_DRAWITEM: { // Message when drawing an item
            /*LPDRAWITEMSTRUCT lpDrawItem = (LPDRAWITEMSTRUCT)lParam; // Draw item structure
            if (lpDrawItem->CtlID >= 3000 && lpDrawItem->CtlID < 3000 + processNames.size()) { // Check the ID
                std::wstring processName = processNames[lpDrawItem->CtlID - 3000]; // Retrieve the process name
                wchar_t buttonText = expandedState[processName] ? 'v' : '>'; // Set the button text

                // Create a brush and fill the rectangle only if necessary
                HBRUSH hBrush = CreateSolidBrush(GetSysColor(COLOR_WINDOW)); // Create a brush
                FillRect(lpDrawItem->hDC, &lpDrawItem->rcItem, hBrush); // Fill the rectangle
                DeleteObject(hBrush); // Delete the brush

                SetBkMode(lpDrawItem->hDC, TRANSPARENT); // Set the background mode
                SetTextColor(lpDrawItem->hDC, RGB(0, 0, 0)); // Set the text color
                DrawTextW(lpDrawItem->hDC, &buttonText, 1, &lpDrawItem->rcItem, DT_CENTER | DT_VCENTER | DT_SINGLELINE); // Draw the text
                
                InvalidateWindow(hwnd);*/
                InvalidateRect(hwnd, NULL, TRUE);

                return TRUE; // Return TRUE
            //}
        }
        break;

        case WM_UPDATE_LIST: { // Message to update the list
            processWindowsMap.clear(); // Clear the process window map
            processNames.clear(); // Clear the process names
            checkboxState.clear(); // Clear the checkbox state
            expandedState.clear(); // Clear the expanded state
            auto windows = getOpenWindows(); // Retrieve the windows

            for (auto& window : windows) { // Iterate through the windows
                processWindowsMap[window.processName].push_back(window); // Add the window to the map
                expandedState[window.processName] = false; // Set the expanded state
                checkboxState[window.processName] = false; // Set the checkbox state
            }

            for (auto& entry : processWindowsMap) { // Iterate through the process window map
                std::sort(entry.second.begin(), entry.second.end(), compareWindowsByName); // Sort windows by window name
                processNames.push_back(entry.first); // Add the process name
            }
            std::sort(processNames.begin(), std::end(processNames), caseInsensitiveCompare); // Sort the process names
            SCROLLINFO si = {}; // Initialize scroll info
            si.cbSize = sizeof(si); // Set the size of the scroll info
            si.fMask = SIF_RANGE | SIF_PAGE; // Set the mask
            si.nMin = 0; // Set the minimum
            si.nMax = processNames.size() * 30; // Set the maximum
            si.nPage = 100; // Set the page
            SetScrollInfo(hwnd, SB_VERT, &si, TRUE); // Set the scroll info
            InvalidateRect(hwnd, NULL, TRUE); // Invalidate the rectangle

            AdjustWindowSize(hwnd); // Adjust the window size
            //InvalidateWindow(hwnd);
        }
        break;

        case WM_VSCROLL: {
            SCROLLINFO si = {};
            si.cbSize = sizeof(si);
            si.fMask = SIF_ALL;
            GetScrollInfo(hwnd, SB_VERT, &si);
            int yPos = si.nPos;
            switch (LOWORD(wParam)) {
                case SB_LINEUP: yPos -= 30; break;
                case SB_LINEDOWN: yPos += 30; break;
                case SB_PAGEUP: yPos -= si.nPage; break;
                case SB_PAGEDOWN: yPos += si.nPage; break;
                case SB_THUMBTRACK: yPos = HIWORD(wParam); break;
            }
            yPos = std::max(0, std::min(yPos, si.nMax - (int)si.nPage + 1));
            if (yPos != si.nPos) {
                si.fMask = SIF_POS;
                si.nPos = yPos;
                SetScrollInfo(hwnd, SB_VERT, &si, TRUE);
                scrollPos = yPos;
                //InvalidateWindow(hwnd);
                InvalidateRect(hwnd, NULL, TRUE);
            }
            break;
        }

        case WM_ERASEBKGND:
            return 1; // Hintergrund nicht löschen

        case WM_COMMAND: { // Message when a command is executed (e.g., button click)
            id = LOWORD(wParam); // Extract the command ID from wParam
            //std::wstring message = L"" + std::to_wstring(id);
            //MessageBoxW(hwnd, message.c_str(), L"Debug Info", MB_OK);
            if (id == IDC_ERASEBUTTON) {
                SetWindowText(hSearchBox, L"");
                ShowWindow(hEraseButton, SW_SHOW); 
                SearchAndCheckErase(hwnd);
                InvalidateRect(hEraseButton, NULL, TRUE);
                UpdateWindow(hEraseButton);
                //InvalidateWindow(hwnd);                               
            }
            if (HIWORD(wParam) == EN_CHANGE && LOWORD(wParam) == IDC_SEARCHBOX) {
                wchar_t searchString[256];
                GetWindowTextW(hSearchBox, searchString, 256);
                SearchAndCheck(searchString, hwnd);
                //InvalidateWindow(hwnd);
            }
            if (HIWORD(wParam) == EN_UPDATE && LOWORD(wParam) == IDC_SEARCHBOX) {
                if (GetKeyState(VK_RETURN) & 0x8000) {
                    wchar_t searchString[256];
                    GetWindowTextW(hSearchBox, searchString, 256);
                    SearchAndCheck(searchString, hwnd);
                    //InvalidateWindow(hwnd);
                }
            }
            // Check if the command ID is one of the specified IDs
            if (id == ID_MINIMIZE || id == ID_MAXIMIZE || id == ID_RESTORE || id == ID_CLOSE || 
                (id >= ID_ARRANGE && id <= ID_ARRANGE + screenCount) || 
                (id >= ID_MOVE_TO_SCREEN_BASE && id <= ID_MOVE_TO_SCREEN_BASE + screenCount)) {

                // Check if at least one window is checked
                bool isChecked = false;
                for (const auto& entry : processWindowsMap) { // Iterate through all processes
                    for (const auto& window : entry.second) { // Iterate through all windows of a process
                        if (window.checked) {
                            isChecked = true;
                            break; // Exit the loop as soon as one checked window is found
                        }
                    }
                    if (isChecked) break; // Exit the outer loop as well
                }

                if (!isChecked) {
                    MessageBoxW(hwnd, L"Please select one or more window(s).", L"Warning", MB_OK | MB_ICONWARNING);
                    break; // Exit the WM_COMMAND handler if no window is checked
                }
            }


            if (id == ID_MINIMIZE) { // Check if the ID is 2000 (Minimize)
                int screenWidth = GetSystemMetrics(SM_CXSCREEN);
                int screenHeight = GetSystemMetrics(SM_CYSCREEN);
                for (auto& entry : processWindowsMap) { // Iterate through all processes
                    for (auto& window : entry.second) { // Iterate through all windows of a process
                        if (window.checked) { // Check if the window is selected
                            SetWindowPos(window.hwnd, NULL, 0, 0, screenWidth, screenHeight, SWP_NOZORDER);
                            //ProcessMessages(); // Process messages
                            window.arranged = false; // Setze arranged auf false, nachdem das Fenster vergrößert wurde
                            ShowWindow(window.hwnd, SW_MINIMIZE); // Minimize the window
                            ProcessMessages(); // Process messages
                        }
                    }
                }
                for (auto& entry : processWindowsMap) { // Iterate through all processes
                    for (auto& window : entry.second) { // Iterate through all windows of a process
                        window.checked = false; // Deselect the window
                    }
                }
                for (auto& state : checkboxState) { // Iterate through all checkbox states
                    state.second = false; // Deselect the checkbox
                }
                for (auto& state : expandedState) { // Iterate through all expanded states
                    state.second = false; // Collapse the expanded state
                }
                InvalidateRect(hwnd, NULL, TRUE); // Invalidate and redraw the window
                UpdateWindowList(hwnd); // Update the window list
                AdjustWindowSize(hwnd); // Adjust the window size
                ProcessMessages(); // Process messages
                //InvalidateWindow(hwnd);
                Sleep(100); // Short pause
                MinimizeToTray(hwnd); // Minimize the window to the tray
            } else if (id == ID_MAXIMIZE) { // Check if the ID is 2000 (Minimize)
                int screenWidth = GetSystemMetrics(SM_CXSCREEN);
                int screenHeight = GetSystemMetrics(SM_CYSCREEN);
                for (auto& entry : processWindowsMap) { // Iterate through all processes
                    for (auto& window : entry.second) { // Iterate through all windows of a process
                        if (window.checked) { // Check if the window is selected
                            SetWindowPos(window.hwnd, NULL, 0, 0, screenWidth, screenHeight, SWP_NOZORDER);
                            //ProcessMessages(); // Process messages
                            window.arranged = false; // Setze arranged auf false, nachdem das Fenster vergrößert wurde
                            ShowWindow(window.hwnd, SW_MAXIMIZE); // Maximize the window
                            ProcessMessages(); // Process messages
                        }
                    }
                }
                for (auto& entry : processWindowsMap) { // Iterate through all processes
                    for (auto& window : entry.second) { // Iterate through all windows of a process
                        window.checked = false; // Deselect the window
                    }
                }
                for (auto& state : checkboxState) { // Iterate through all checkbox states
                    state.second = false; // Deselect the checkbox
                }
                for (auto& state : expandedState) { // Iterate through all expanded states
                    state.second = false; // Collapse the expanded state
                }
                InvalidateRect(hwnd, NULL, TRUE); // Invalidate and redraw the window
                UpdateWindowList(hwnd); // Update the window list
                AdjustWindowSize(hwnd); // Adjust the window size
                ProcessMessages(); // Process messages
                //InvalidateWindow(hwnd);
                Sleep(100); // Short pause
                MinimizeToTray(hwnd); // Minimize the window to the tray
            } else if (id == ID_RESTORE) { // Check if the ID is 2001 (Restore)
                for (const auto& entry : processWindowsMap) { // Iterate through all processes
                    for (const auto& window : entry.second) { // Iterate through all windows of a process
                        if (window.checked) { // Check if the window is selected
                            //MoveWindowToScreen(window.hwnd, 1);
                            //MoveWindowToMainMonitor(window.hwnd); // Move the window to the main monitor
                            ShowWindow(window.hwnd, SW_MINIMIZE); // Minimize the window
                            ShowWindow(window.hwnd, SW_RESTORE); // Restore the window
                            SetForegroundWindow(window.hwnd); // Bring the window to the foreground
			                //SetWindowPos(window.hwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE); // Ensure the window is brought to the top of the Z-order
                            ProcessMessages(); // Process messages
                        }
                    }
                }
                for (auto& entry : processWindowsMap) { // Iterate through all processes
                    for (auto& window : entry.second) { // Iterate through all windows of a process
                        window.checked = false; // Deselect the window
                    }
                }
                for (auto& state : checkboxState) { // Iterate through all checkbox states
                    state.second = false; // Deselect the checkbox
                }
                for (auto& state : expandedState) { // Iterate through all expanded states
                    state.second = false; // Collapse the expanded state
                }
                InvalidateRect(hwnd, NULL, TRUE); // Invalidate and redraw the window
                UpdateWindowList(hwnd); // Update the window list
                AdjustWindowSize(hwnd); // Adjust the window size
                ProcessMessages(); // Process messages
                //InvalidateWindow(hwnd);
                Sleep(100); // Short pause
                MinimizeToTray(hwnd); // Minimize the window to the tray
            } else if (id == ID_CLOSE) { // Check if the ID is 2002 (Close)
                if (ConfirmClose(hwnd)) { // Display confirmation dialog
                    for (const auto& entry : processWindowsMap) { // Iterate through all processes
                        for (const auto& window : entry.second) { // Iterate through all windows of a process
                            if (window.checked) { // Check if the window is selected
                                PostMessage(window.hwnd, WM_CLOSE, 0, 0); // Close the window
                                ProcessMessages(); // Process messages
                            }
                        }
                    }
                    for (auto& entry : processWindowsMap) { // Iterate through all processes
                        for (auto& window : entry.second) { // Iterate through all windows of a process
                            window.checked = false; // Deselect the window
                        }
                    }
                    for (auto& state : checkboxState) { // Iterate through all checkbox states
                        state.second = false; // Deselect the checkbox
                    }
                    for (auto& state : expandedState) { // Iterate through all expanded states
                        state.second = false; // Collapse the expanded state
                    }
                    InvalidateRect(hwnd, NULL, TRUE); // Invalidate and redraw the window
                    UpdateWindowList(hwnd); // Update the window list
                    AdjustWindowSize(hwnd); // Adjust the window size
                    ProcessMessages(); // Process messages
                    //InvalidateWindow(hwnd);
                    Sleep(100); // Short pause
                    MinimizeToTray(hwnd); // Minimize the window to the tray
                }
            } else if (id >= ID_ARRANGE && id <= ID_ARRANGE + screenCount) {//else if (id == 2003) { Check if the ID is 2003 (Arrange)
                //MessageBoxW(hwnd, L"aha2", L"Debug Info", MB_OK);
                int screenIndex = (id - ID_ARRANGE) ; // Handle move to screen action
                RECT screenRect = GetScreenRect(screenIndex);
                //RECT rect; // Declaration of a RECT structure
                //GetClientRect(hwnd, &rect); // Retrieve the client rectangles of the window
                //int screenWidth = GetSystemMetrics(SM_CXSCREEN); // Retrieve the screen width
                int screenWidth = screenRect.right - screenRect.left;
                int screenHeight = screenRect.bottom - screenRect.top;
                //int screenHeight = GetSystemMetrics(SM_CYSCREEN); // Retrieve the screen height

                int numWindows = 0; // Counter for the number of windows
                for (auto& entry : processWindowsMap) { // Iterate through all processes
                    for (auto& window : entry.second) { // Iterate through all windows of a process
                        if (window.checked) { // Check if the window is selected
                            window.arranged = true; 
                            numWindows++; // Increment the counter
                        }
                    }
                }

                int cols = static_cast<int>(ceil(sqrt(numWindows))); // Calculate the number of columns
                int rows = (numWindows + cols - 1) / cols; // Calculate the number of rows

                int windowWidth = screenWidth / cols; // Calculate the window width
                int windowHeight = screenHeight / rows; // Calculate the window height

                int x = 0, y = 0; // Initialize the X and Y positions
                for (const auto& entry : processWindowsMap) { // Iterate through all processes
                    for (const auto& window : entry.second) { // Iterate through all windows of a process
                        if (window.checked) { // Check if the window is selected
                            ShowWindow(window.hwnd, SW_RESTORE); // Restore the window
                            SetWindowPos(window.hwnd, NULL, 0, 0, screenWidth, screenHeight, SWP_NOZORDER);
                            MoveWindowToScreen(window.hwnd, screenIndex);
			                //MoveWindowToMainMonitor(window.hwnd); // Move the window to the main monitor
                            ProcessMessages(); // Process messages
                            ShowWindow(window.hwnd, SW_MINIMIZE); // Minimize the window
                            ShowWindow(window.hwnd, SW_RESTORE); // Restore the window
                            SetForegroundWindow(window.hwnd); // Bring the window to the foreground
                            //SetWindowPos(window.hwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE); // Ensure the window is brought to the top of the Z-order
                            ProcessMessages(); // Process messages
                            //Sleep(150); // Short pause
                            MoveWindow(window.hwnd, screenRect.left + x, screenRect.top + y, windowWidth, windowHeight, TRUE); // Move and resize the window
                            ProcessMessages(); // Process messages
                            x += windowWidth; // Increment the X position
                            if (x >= screenWidth) { // Check if the X position exceeds the screen width
                                x = 0; // Reset the X position
                                y += windowHeight; // Increment the Y position
                            }
                        }
                    }
                }
                for (auto& entry : processWindowsMap) { // Iterate through all processes
                    for (auto& window : entry.second) { // Iterate through all windows of a process
                        window.checked = false; // Deselect the window
                    }
                }
                for (auto& state : checkboxState) { // Iterate through all checkbox states
                    state.second = false; // Deselect the checkbox
                }
                for (auto& state : expandedState) { // Iterate through all expanded states
                    state.second = false; // Collapse the expanded state
                }
                InvalidateRect(hwnd, NULL, TRUE); // Invalidate and redraw the window
                UpdateWindowList(hwnd); // Update the window list
                AdjustWindowSize(hwnd); // Adjust the window size
                ProcessMessages(); // Process messages
                Sleep(100); // Short pause
                //InvalidateWindow(hwnd);
                MinimizeToTray(hwnd); // Minimize the window to the tray
                ProcessMessages(); // Process messages
            } else if (id == ID_TRAY_EXIT) { // Check if the ID is for tray exit
                PostQuitMessage(0); // Quit the application
            } else if (id >= ID_MOVE_TO_SCREEN_BASE && id <= ID_MOVE_TO_SCREEN_BASE + screenCount) { // Check if the ID is 2004ff (Move to Screen x)
                int screenIndex = (id - ID_MOVE_TO_SCREEN_BASE) ; // Handle move to screen action
                RECT screenRect = GetScreenRect(screenIndex);
                 for (const auto& entry : processWindowsMap) { // Iterate through all processes
                    for (const auto& window : entry.second) { // Iterate through all windows of a process
                        if (window.checked) { // Check if the window is selected
                            MoveWindowToScreen(window.hwnd, screenIndex);
                        }
                    }
                }
                InvalidateRect(hwnd, NULL, TRUE); // Invalidate and redraw the window
                UpdateWindowList(hwnd); // Update the window list
                AdjustWindowSize(hwnd); // Adjust the window size
                ProcessMessages(); // Process messages
                //InvalidateWindow(hwnd);
                Sleep(100); // Short pause
                MinimizeToTray(hwnd); // Minimize the window to the tray
            } else if (id >= 3000 && id < 3000 + processNames.size()) { // Check if the ID is within the range of process names and thus collapse/expand needed
                //MessageBoxW(hwnd, L"aha3", L"Debug Info", MB_OK);
                std::wstring processName = processNames[id - 3000]; // Retrieve the process name based on the ID
                //checkboxState[processName] = !checkboxState[processName]; // Toggle the checkbox state for the process
                expandedState[processName] = !expandedState[processName]; // Toggle the expanded state for the process

                /*bool alreadyOneOpened = false;
                for (auto& window : processWindowsMap[processName]) 
                { // Iterate through all windows of the process
                    if (expandedState[processName]) 
                    { // Check if the process is expanded
                        alreadyOneOpened = false;
                        for (auto& window : processWindowsMap[processName]) 
                        { // Iterate through all windows of the process
                            if (window.visible == true) alreadyOneOpened = true;
                        }
                        if (!alreadyOneOpened)
                        {
                            for (auto& window : processWindowsMap[processName]) 
                            { // Iterate through all windows of the process
                                window.visible = true; 
                            }
                        }
                    }              
                }*/
                SCROLLINFO si = {}; // Initialize a SCROLLINFO structure
                si.cbSize = sizeof(si); // Set the size of the SCROLLINFO structure
                si.fMask = SIF_RANGE | SIF_PAGE; // Specify the masks to use
                si.nMin = 0; // Set the minimum scroll range
                si.nMax = 0; // Initialize the maximum scroll range
                for (const auto& processName : processNames) { // Iterate through all process names
                    si.nMax += 30; // Increment the maximum scroll range for each process
                    if (expandedState[processName]) { // Check if the process is expanded
                        //si.nMax += processWindowsMap[processName].size() * 30; // Increment the maximum scroll range based on the number of windows
                        bool alreadyOneOpened = false;
                        for (auto& window : processWindowsMap[processName]) { // Iterate through all windows of the process
                            if (window.visible == true) alreadyOneOpened = true;
                        }
                        if (!alreadyOneOpened) // No window yet opened
                            for (auto& window : processWindowsMap[processName]) { // Iterate through all windows of the process
                                window.visible = true; // make all visible
                            }
                        for (auto& window : processWindowsMap[processName]) { // Iterate through all windows of the process
                            if (window.visible == true) 
                                si.nMax += 30; // make the scroll area as long as required
                        }
                    }
                    else
                    {
                         for (auto& window : processWindowsMap[processName]) { // Iterate through all windows of the process
                            window.visible = false; 
                        }
                    }
                }
                si.nPage = 100; // Set the page length for scrolling
                SetScrollInfo(hwnd, SB_VERT, &si, TRUE); // Set the scroll information for the vertical scrollbar
                InvalidateRect(hwnd, NULL, TRUE); // Invalidate and redraw the window
                AdjustWindowSize(hwnd); // Adjust the window size
                //InvalidateRect(hwnd, NULL, TRUE);
                //InvalidateWindow(hwnd);
            } else if (HIWORD(wParam) == EN_SETFOCUS && LOWORD(wParam) == IDC_SEARCHBOX) {
                wchar_t text[256];
                GetWindowText(hSearchBox, text, 256);
                if (wcscmp(text, L"Search name") == 0) {
                    SetWindowText(hSearchBox, L"");
                }
            } else if (HIWORD(wParam) == EN_KILLFOCUS && LOWORD(wParam) == IDC_SEARCHBOX) {
                wchar_t text[256];
                GetWindowText(hSearchBox, text, 256);
                if (wcslen(text) == 0) {
                    SetEditPlaceholder(hSearchBox, L"Search name");
                }
            } else if (HIWORD(wParam) == EN_CHANGE && LOWORD(wParam) == IDC_SEARCHBOX) {
                wchar_t searchString[256];
                GetWindowTextW(hSearchBox, searchString, 256);
                SearchAndCheck(searchString, hwnd);
            }
            }
            break;

        case WM_LBUTTONDOWN: { // Message when the left mouse button is pressed
            isScrolling = true; // Start scrolling
            SetCapture(hwnd); // Capture the mouse to receive all mouse events
            lastMousePos.x = GET_X_LPARAM(lParam); // Store the X position of the mouse
            lastMousePos.y = GET_Y_LPARAM(lParam); // Store the Y position of the mouse

            POINT pt; // Declaration of a POINT structure to store the cursor position
            GetCursorPos(&pt); // Retrieve the current cursor position
            ScreenToClient(hwnd, &pt); // Convert screen coordinates to client coordinates
            int yPos = 0 - scrollPos; // Initialize the y-position based on the scroll position
            //bool found = false; // Flag to check if an item was found
            HFONT hFont = CreateFont(24, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, TEXT("Segoe UI")); // Create a font
            HDC hdc = GetDC(hwnd); // Retrieve the device context
            HFONT hOldFont = (HFONT)SelectObject(hdc, hFont); // Select the new font and save the old font
            for (size_t i = 0; i < processNames.size(); ++i) {
                const auto& processName = processNames[i];
                RECT rect = { 50, yPos, textWidth, yPos + 30 };
                //MessageBoxW(hwnd, (L"yPos: " + std::to_wstring(yPos)).c_str(), L"Info", MB_OK | MB_ICONINFORMATION);
                if (PtInRect(&rect, pt)) {
                    checkboxState[processName] = !checkboxState[processName];
                    for (auto& window : processWindowsMap[processName]) {
                        window.checked = checkboxState[processName];
                    }
                    InvalidateRect(hwnd, NULL, TRUE);
                    //found = true;
                    //break;
                }
                rect = { 0, yPos, 30, yPos + 30 };
                //MessageBoxW(hwnd, (L"yPos: " + std::to_wstring(yPos)).c_str(), L"Info", MB_OK | MB_ICONINFORMATION);
                if (PtInRect(&rect, pt)) {
                    expandedState[processName] = !expandedState[processName];
                    for (size_t j = 0; j < processWindowsMap[processName].size(); ++j) { // Iterate through all windows of the process
                        auto& window = processWindowsMap[processName][j]; // Retrieve the current window
                        window.visible = true;
                        yPos += 30;
                    }
                    InvalidateWindow(hwnd);
                    //found = true;
                    //break;
                }
                yPos += 30;// Increase the y-position

                /*bool AlreadyOneOpened = false;
                for (auto& window : processWindowsMap[processName]) 
                { // Iterate through all windows of the process
                    if (window.visible == true) AlreadyOneOpened = true;
                }*/
 
                /*if (NbrAlreadyOpened == 0) 
                    NbrAlreadyOpened = processWindowsMap[processName].size();*/
                
                if (expandedState[processName]) { // Check if the process is expanded
                    //MessageBoxW(hwnd, L"aha3", L"Debug Info", MB_OK);
                    for (size_t j = 0; j < processWindowsMap[processName].size(); ++j) { // Iterate through all windows of the process
                        auto& window = processWindowsMap[processName][j]; // Retrieve the current window
                        RECT windowRect = { 50, yPos, textWidth, yPos + 30 }; // Define a rectangle for the window
                        //MessageBoxW(hwnd, (L"yPos: " + std::to_wstring(yPos)).c_str(), L"Info", MB_OK | MB_ICONINFORMATION);
                        if (PtInRect(&windowRect, pt) && window.visible) { // Check if the cursor is in the rectangle
                            window.checked = !window.checked; // Toggle the checkbox state for the window
                            InvalidateRect(hwnd, NULL, TRUE); // Invalidate and redraw the window
                            //found = true; // Set the flag that an item was found
                            //break; // Exit the loop
                        }
                        if (window.visible || window.checked) 
                            yPos += 30; // Increase the y-position 
                    }
                }
            }
            /*    SCROLLINFO si = {};
                si.cbSize = sizeof(si);
                si.fMask = SIF_RANGE | SIF_PAGE;
                si.nMin = 0;
                si.nMax = processNames.size() * 30;
                si.nPage = 10;
                SetScrollInfo(hwnd, SB_VERT, &si, TRUE);
                scrollPos = 0;*/
            SelectObject(hdc, hOldFont); // Restore the old font
            DeleteObject(hFont); // Delete the new font
            ReleaseDC(hwnd, hdc); // Release the device context
            //InvalidateWindow(hwnd);
        }
        break;

        case WM_MOUSEMOVE: { // Message when the mouse is moved
            if (isScrolling) { // Check if scrolling
                POINT currentMousePos; // Variable to store the current mouse position
                currentMousePos.x = GET_X_LPARAM(lParam); // Retrieve the X position of the mouse
                currentMousePos.y = GET_Y_LPARAM(lParam); // Retrieve the Y position of the mouse

                int deltaY = currentMousePos.y - lastMousePos.y; // Calculate the Y displacement

                // Scroll the window
                SCROLLINFO si = {}; // Initialize a SCROLLINFO structure
                si.cbSize = sizeof(si); // Set the size of the structure
                si.fMask = SIF_ALL; // Specify that all information is set
                GetScrollInfo(hwnd, SB_VERT, &si); // Retrieve the current scroll information
                int yPos = si.nPos - deltaY; // Calculate the new Y position

                // Limit the Y position
                yPos = std::max(0, std::min(yPos, si.nMax - (int)si.nPage + 1));
                if (yPos != si.nPos) { // Check if the position has changed
                    si.fMask = SIF_POS; // Specify that the position is set
                    si.nPos = yPos; // Set the new position
                    SetScrollInfo(hwnd, SB_VERT, &si, TRUE); // Set the scroll info
                    RECT scrollRect;
                    GetClientRect(hwnd, &scrollRect);
                    //scrollRect.bottom -= 55; // Area without white bar
                    ScrollWindowEx(hwnd, 0, deltaY, &scrollRect, &scrollRect, NULL, NULL, SW_INVALIDATE | SW_ERASE); 
                    InvalidateRect(hwnd, &scrollRect, TRUE); // Invalidate and redraw the rectangle
                }
                InvalidateRect(hwnd, NULL, TRUE); // Invalidate and redraw the rectangle
                lastMousePos = currentMousePos; // Update the last mouse position
            } else {
                POINT pt; 
                GetCursorPos(&pt); 
                ScreenToClient(hwnd, &pt); 

                int yPos = 0 - scrollPos + 0;
                int newHighlightedRow = -1;
                int newHighlightedWindowRow = -1;

                for (size_t i = 0; i < processNames.size(); ++i) {
                    const auto& processName = processNames[i];
                    RECT rect = {-30, yPos, textWidth, yPos + 30 };
                    if (PtInRect(&rect, pt)) {
                        newHighlightedRow = i;
                        TOOLINFOW ti = { 0 }; 
                        ti.cbSize = sizeof(TOOLINFOW);
                        ti.uFlags = TTF_SUBCLASS;
                        ti.hwnd = hwnd;
                        ti.hinst = GetModuleHandle(NULL);
                        ti.lpszText = const_cast<LPWSTR>(processName.c_str()); 
                        ti.rect = rect;
                        //SendMessage(hwndTT, TTM_ADDTOOL, 0, (LPARAM)&ti);  // tooltip does not yet work correctly. Shows only first letter of the name
                        //SendMessage(hwndTT, TTM_SETMAXTIPWIDTH, 0, 300);  // tooltip does not yet work correctly. Shows only first letter of the name
                        //SendMessage(hwndTT, TTM_UPDATETIPTEXT, 0, (LPARAM)&ti);  // tooltip does not yet work correctly. Shows only first letter of the name
                        break;
                    }
                    yPos += 30;
                    if (expandedState[processName]) {
                        auto& windows = processWindowsMap[processName];
                        for (size_t j = 0; j < windows.size(); ++j) {
                            const auto& window = windows[j];
                            if (window.visible || window.checked) {
                                RECT windowRect = {-30, yPos, textWidth, yPos + 30 };
                                if (PtInRect(&windowRect, pt)) {
                                    newHighlightedWindowRow = i*100000+j;
                                    newHighlightedRow = i;
                                    break;
                                }
                                yPos += 30;
                            }
                        }
                    }
                }

                if (newHighlightedRow != highlightedRow || newHighlightedWindowRow != highlightedWindowRow) {
                    highlightedRow = newHighlightedRow;
                    highlightedWindowRow = newHighlightedWindowRow;
                    InvalidateRect(hwnd, NULL, TRUE);
                }
            }
        }
        break;
    
        case WM_LBUTTONUP: { // Message when the left mouse button is released
            isScrolling = false; // Stop scrolling
            ReleaseCapture(); // Release the mouse
        }
        break;
        
        case WM_MOUSEWHEEL: { // Nachricht, wenn das Mausrad gedreht wird
            int delta = GET_WHEEL_DELTA_WPARAM(wParam); // Abrufen der Scroll-Richtung und -Menge
            SCROLLINFO si = {}; // Initialisieren einer SCROLLINFO-Struktur
            si.cbSize = sizeof(si); // Setzen der Größe der Struktur
            si.fMask = SIF_ALL; // Festlegen, dass alle Informationen gesetzt werden
            GetScrollInfo(hwnd, SB_VERT, &si); // Abrufen der aktuellen Scroll-Informationen
            int yPos = si.nPos - delta / WHEEL_DELTA * 30; // Neue Y-Position berechnen

            // Begrenzen der Y-Position
            yPos = std::max(0, std::min(yPos, si.nMax - (int)si.nPage + 1));
            if (yPos != si.nPos) { // Überprüfen, ob sich die Position geändert hat
                si.fMask = SIF_POS; // Festlegen, dass die Position gesetzt wird
                si.nPos = yPos; // Setzen der neuen Position
                SetScrollInfo(hwnd, SB_VERT, &si, TRUE); // Scroll-Info setzen
                scrollPos = yPos; // Scroll-Position setzen
                RECT scrollRect;
                GetClientRect(hwnd, &scrollRect);
                //scrollRect.bottom -= 55; // Bereich ohne grauen Balken
                ScrollWindowEx(hwnd, 0, delta / WHEEL_DELTA * 30, &scrollRect, &scrollRect, NULL, NULL, SW_INVALIDATE | SW_ERASE); 
                InvalidateRect(hwnd, &scrollRect, TRUE); // Rechteck ungültig machen und neu zeichnen
            }
        }
        break;
                
        case WM_PAINT: {
            isRedrawPending = false; // Reset the flag
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            // Erstelle einen Offscreen-Puffer
            HDC hdcMem = CreateCompatibleDC(hdc);
            RECT clientRect;
            GetClientRect(hwnd, &clientRect);
            HBITMAP hbmMem = CreateCompatibleBitmap(hdc, clientRect.right, clientRect.bottom);
            HBITMAP hbmOld = (HBITMAP)SelectObject(hdcMem, hbmMem);

            // Hintergrund des Offscreen-Puffers füllen
            HBRUSH hBrush = CreateSolidBrush(RGB(255, 255, 255)); // Weißer Hintergrund
            FillRect(hdcMem, &clientRect, hBrush);
            DeleteObject(hBrush);

            // Zeichne das UI im Offscreen-Puffer
            int searchBoxHeight = 0; // Höhe des Suchfelds
            int yPos = searchBoxHeight - scrollPos; // Platz für das Suchfeld und etwas Abstand
            int nmbOfItemsOnWindow = 0;
            HFONT hFont = CreateFont(24, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, TEXT("Segoe UI Symbol"));
            HFONT hOldFont = (HFONT)SelectObject(hdcMem, hFont);

            int scrollbarWidth = GetSystemMetrics(SM_CXVSCROLL);
            //int textWidth = clientRect.right - scrollbarWidth - 30;

            int AlreadyOneChecked = 0;
            int TotalChecked = 0;

            for (size_t i = 0; i < processNames.size(); ++i) {
                const auto& processName = processNames[i];
                AlreadyOneChecked = 0;
                for (auto& window : processWindowsMap[processName]) { // Iterate through all windows of the process
                    if (window.checked == true) AlreadyOneChecked++;
                }
                TotalChecked += AlreadyOneChecked;
                auto& windows = processWindowsMap[processName];

                // Trimmen und Umwandeln des Prozessnamens
                std::wstring processNameW(processName.begin(), processName.end());
                processNameW = trim(processNameW);
                processNameW = capitalizeIfAllCaps(processNameW);

                // Berechnung der maximalen Breite der Prozessnamen
                int maxProcessNameWidth = 0;
                for (const auto& process : processNames) {
                    std::wstring processNameW(process.begin(), process.end());
                    processNameW = trim(processNameW);
                    processNameW = capitalizeIfAllCaps(processNameW);

                    HFONT hFontBold = CreateFont(0, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, TEXT("Segoe UI"));
                    HFONT hOldFont = (HFONT)SelectObject(hdcMem, hFontBold);

                    SIZE size;
                    GetTextExtentPoint32W(hdcMem, processNameW.c_str(), processNameW.length(), &size);
                    if (size.cx > maxProcessNameWidth) {
                        maxProcessNameWidth = size.cx;
                    }

                    SelectObject(hdcMem, hOldFont);
                    DeleteObject(hFontBold);
                }

                std::wstring textBeforeProcessName = expandedState[processName] ? L"\u25BC        " : L"\u25B6        "; // Set the button text

                if (AlreadyOneChecked == windows.size()) {
                    textBeforeProcessName += L"\u2611";
                } else if (AlreadyOneChecked > 0) {
                    if (expandedState[processName]) {
                        textBeforeProcessName += L"\u25EA "; 
                    } else {
                        textBeforeProcessName += L"\u25EA    ";
                    }
                } else {
                    textBeforeProcessName += checkboxState[processName] ? L"\u2611" : L"\u2610 ";
                }

                std::wstring textAfterProcessName = L" (" + std::to_wstring(AlreadyOneChecked) + L" / " + std::to_wstring(windows.size()) + L")";

                RECT rect = {10, yPos, textWidth, yPos + 30};

                if (i == highlightedRow && highlightedWindowRow == -1) {
                    HBRUSH highlightBrush = CreateSolidBrush(RGB(211, 211, 211)); // Hellgrau
                    FillRect(hdcMem, &rect, highlightBrush);
                    DeleteObject(highlightBrush);
                } else {
                    HBRUSH whiteBrush = CreateSolidBrush(RGB(255, 255, 255)); // Weiß
                    FillRect(hdcMem, &rect, whiteBrush);
                    DeleteObject(whiteBrush);
                }

                SetTextColor(hdcMem, RGB(0, 0, 0));
                SetBkMode(hdcMem, TRANSPARENT); // Setzen Sie den Hintergrundmodus auf transparent

                // Zeichnen Sie den Text vor dem Prozessnamen
                DrawTextW(hdcMem, textBeforeProcessName.c_str(), -1, &rect, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);

                // Icon des Prozesses zeichnen
                DrawIconEx(hdcMem, rect.left + 25, yPos + 5, processIcons[processName], 20, 20, 0, NULL, DI_NORMAL);

                // Berechnen Sie die Breite des vorherigen Textes
                SIZE size;
                HFONT hFontNormal = CreateFont(0, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, TEXT("Segoe UI"));
                HFONT hOldFont = (HFONT)SelectObject(hdcMem, hFontNormal);
                GetTextExtentPoint32W(hdcMem, textBeforeProcessName.c_str(), textBeforeProcessName.length(), &size);
                rect.left += size.cx;
                SelectObject(hdcMem, hOldFont);
                DeleteObject(hFontNormal);

                rect.left = 90;

                // Zeichnen Sie den fetten Prozessnamen
                HFONT hFontBold = CreateFont(0, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, TEXT("Segoe UI"));
                hOldFont = (HFONT)SelectObject(hdcMem, hFontBold);

                // Anpassen der Y-Position des fetten Prozessnamens um 3 Pixel nach oben
                RECT processNameRect = rect;
                processNameRect.top -= 3;

                DrawTextW(hdcMem, processNameW.c_str(), -1, &processNameRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);

                // Berechnen Sie die Breite des Prozessnamens mit der fetten Schriftart
                GetTextExtentPoint32W(hdcMem, processNameW.c_str(), processNameW.length(), &size);
                rect.left += maxProcessNameWidth + 20;

                SelectObject(hdcMem, hOldFont);
                DeleteObject(hFontBold);

                hOldFont = (HFONT)SelectObject(hdcMem, hFontNormal);
                DrawTextW(hdcMem, textAfterProcessName.c_str(), -1, &rect, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
                SelectObject(hdcMem, hOldFont);
                DeleteObject(hFontNormal);



                //SetWindowPos(expandButtons[processName], HWND_TOPMOST, 10, yPos, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW);
                // Überprüfe, ob die Position geändert wurde
                /*if (buttonPositions[processName] != yPos) {
                    //SetWindowPos(expandButtons[processName], HWND_TOPMOST, 10, yPos, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOREDRAW);
                    buttonPositions[processName] = yPos; // Aktualisiere die gespeicherte Position
                }*/
                yPos += 30;
                nmbOfItemsOnWindow++;

                SetTextColor(hdcMem, RGB(0, 0, 255)); // Fensterzeilen in Blau
                if (expandedState[processName]) {
                    // Fenster-Namen sortieren
                    auto& windows = processWindowsMap[processName];
                    std::sort(windows.begin(), windows.end(), compareWindowsByName);

                    for (size_t j = 0; j < windows.size(); ++j) {
                        const auto& window = windows[j];
                        if (window.visible || window.checked) {
                            std::wstring windowText = std::wstring(window.checked ? L"\u2611 " : L"\u2610 ") + std::wstring(window.title.begin(), window.title.end());
                            RECT windowRect = { 90, yPos, textWidth, yPos + 30 };

                            if (i*100000+j == highlightedWindowRow) {
                                HBRUSH highlightBrush = CreateSolidBrush(RGB(211, 211, 211)); // Hellgrau
                                FillRect(hdcMem, &windowRect, highlightBrush);
                                DeleteObject(highlightBrush);
                            } else {
                                HBRUSH whiteBrush = CreateSolidBrush(RGB(255, 255, 255)); // Weiß
                                FillRect(hdcMem, &windowRect, whiteBrush);
                                DeleteObject(whiteBrush);
                            }

                            SetTextColor(hdcMem, RGB(0, 0, 255)); // Fensterzeilen in Blau
                            SetBkMode(hdcMem, TRANSPARENT); // Setzen Sie den Hintergrundmodus auf transparent
                            DrawTextW(hdcMem, windowText.c_str(), -1, &windowRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
                            DrawIconEx(hdcMem, windowRect.left - 25, yPos + 7, processIcons[processName], 16, 16, 0, NULL, DI_NORMAL);
                            yPos += 30;
                            nmbOfItemsOnWindow++;
                        }
                    }
                }
            }

            std::wstring windowTitle = L"Manage Multiple Open Windows - Total Windows Selected: " + std::to_wstring(TotalChecked);
            SetWindowText(hwnd, windowTitle.c_str());

            // Scroll-Informationen aktualisieren
            SCROLLINFO si = {}; // Initialize a SCROLLINFO structure
            si.cbSize = sizeof(si); // Set the size of the SCROLLINFO structure
            si.fMask = SIF_RANGE | SIF_PAGE; // Specify the masks to use
            si.nMin = 0; // Set the minimum scroll range
            si.nMax = std::max(nmbOfItemsOnWindow * 30, 400);
            si.nPage = si.nMax / nmbOfItemsOnWindow; // Set the page length for scrolling
            SetScrollInfo(hwnd, SB_VERT, &si, TRUE); // Set the scroll information for the vertical scrollbar

            AdjustWindowSize(hwnd); // Adjust the window size
            /*SetWindowPos(hSearchBox, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
            SetWindowPos(hEraseButton, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
            InvalidateRect(hSearchBox, NULL, TRUE);
            UpdateWindow(hSearchBox);
            InvalidateRect(hEraseButton, NULL, TRUE);
            UpdateWindow(hEraseButton);
            InvalidateWindow(hwnd); // Invalidate and redraw the window*/

            //AdjustWindowSize(hwnd); // Adjust the window size
     
            SelectObject(hdcMem, hOldFont);
            DeleteObject(hFont);

            // Kopiere den Offscreen-Puffer auf den Bildschirm
            BitBlt(hdc, 0, 0, clientRect.right, clientRect.bottom, hdcMem, 0, 0, SRCCOPY);

            // Bereinige
            SelectObject(hdcMem, hbmOld);
            DeleteObject(hbmMem);
            DeleteDC(hdcMem);

            EndPaint(hwnd, &ps);
        }
        break;


        case WM_SIZE: { // Message when the window size is changed
            RECT rect; // Declaration of a RECT structure to store the client rectangles
            GetClientRect(hwnd, &rect); // Retrieve the client rectangles of the window
            SCROLLINFO si = {}; // Initialize a SCROLLINFO structure
            si.cbSize = sizeof(si); // Set the size of the SCROLLINFO structure
            si.fMask = SIF_PAGE; // Specify the masks to use
            si.nPage = rect.bottom - rect.top - 115; // Set the page length for scrolling
            SetScrollInfo(hwnd, SB_VERT, &si, TRUE); // Set the scroll information for the vertical scrollbar
            int scrollbarWidth = GetSystemMetrics(SM_CXVSCROLL); // Retrieve the width of the vertical scrollbar
            int width = rect.right - rect.left - scrollbarWidth; // Calculate the width of the window without the scrollbar
            int yPos = rect.bottom - 55; // Calculate the y-position for the white bar
            SetWindowPos(whiteBar, NULL, 0, yPos, width + scrollbarWidth, 55, SWP_NOZORDER); // Position the white bar
            //InvalidateWindow(hwnd);

            // int buttonCount = 4; // Number of buttons
            // int buttonWidth = (width - 20) / buttonCount; // Calculate the width of a button

            // Position the buttons for minimize, restore, close, and arrange
            // SetWindowPos(minimizeButton, NULL, 10, yPos + 5 + 5, buttonWidth, 40, SWP_NOZORDER);
            // SetWindowPos(restoreButton, NULL, 10 + buttonWidth + 5, yPos + 5 + 5, buttonWidth, 40, SWP_NOZORDER);
            // SetWindowPos(closeButton, NULL, 10 + 2 * (buttonWidth + 5), yPos + 5 + 5, buttonWidth, 40, SWP_NOZORDER);
            // SetWindowPos(arrangeButton, NULL, 10 + 3 * (buttonWidth + 5), yPos + 5 + 5, buttonWidth, 40, SWP_NOZORDER);
        }
        break;

        case WM_DESTROY: { // Message when the window is destroyed
            RemoveTrayIcon(hwnd); // Remove the tray icon
            PostQuitMessage(0); // Quit the application
            return 0; // Return 0
        }
        break;

        case WM_CLOSE: { // Message when the window is closed
            //RemoveTrayIcon(hwnd); // Remove the tray icon (commented out)
            //DestroyWindow(hwnd); // Destroy the window (commented out)
            MinimizeToTray(hwnd); // Minimize the window to the tray
            return 0; // Return 0
        }
        break;

        case WM_TRAYICON: { 
            if (lParam == WM_LBUTTONUP) { 
                // Call the same functions as in WM_CREATE
                CreateTrayIcon(hwnd);
                UpdateWindowList(hwnd);
                hwndTT = CreateTooltip(hwnd);

                /*HMENU hMenu = CreateMenu();
                AppendMenu(hMenu, MF_STRING, ID_MINIMIZE, L"&Minimize Window(s)");
                AppendMenu(hMenu, MF_STRING, ID_MAXIMIZE, L"Ma&ximize Window(s)");        
                AppendMenu(hMenu, MF_STRING, ID_RESTORE, L"&Restore Window(s)");
                CreateArrangeOnScreenMenu(hMenu);
                CreateMoveToScreenMenu(hMenu);
                AppendMenu(hMenu, MF_STRING, ID_CLOSE, L"&Close Window(s)");
                SetMenu(hwnd, hMenu);*/
                UpdateDynamicMenus(hwnd);

                SCROLLINFO si = {};
                si.cbSize = sizeof(si);
                si.fMask = SIF_RANGE | SIF_PAGE;
                si.nMin = 0;
                si.nMax = processNames.size() * 30;
                si.nPage = 100;
                SetScrollInfo(hwnd, SB_VERT, &si, TRUE);
                scrollPos = 0;

                HINSTANCE hInstance = (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE);
                hIcon1 = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MYICON));

                auto windows = getOpenWindows();
                for (const auto& processName : processNames) {
                    auto it = windows.end();
                    for (auto winIt = windows.begin(); winIt != windows.end(); ++winIt) {
                        if (winIt->processName == processName) {
                            it = winIt;
                            break;
                        }
                    }
                    if (it != windows.end()) {
                        HICON hIcon = ExtractIconW(hInstance, it->exePath.c_str(), 0);
                        if (hIcon == NULL) {
                            hIcon = hIcon1;
                        }
                        processIcons[processName] = hIcon;
                    }
                }

                // Additional actions if needed
                RefreshWindowList(hwnd); 
                ShowWindow(hwnd, SW_RESTORE); 
                SaveCurrentWindows(); 
                AdjustWindowSize(hwnd); 
                //Sleep(150);
                SetForegroundWindow(hwnd); 
                InvalidateWindow(hwnd);
            } else if (lParam == WM_RBUTTONUP) { 
                ShowTrayMenu(hwnd); 
            }
        }
        break;

        default: { // Default message
            return DefWindowProc(hwnd, uMsg, wParam, lParam); // Call the default window procedure
        }
    }
    return 0; 
}

// Main function
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // Main function for Windows applications

    const wchar_t CLASS_NAME[] = L"SampleWindowClass"; // Define the class name for the window

    WNDCLASSW wc = { }; // Initialize a WNDCLASSA structure
    wc.lpfnWndProc = WindowProc; // Set the window procedure
    wc.hInstance = hInstance; // Set the instance handle
    wc.lpszClassName = CLASS_NAME; // Set the class name
    wc.hCursor = LoadCursor(NULL, IDC_ARROW); // Load the default cursor (arrow)
    wc.hbrBackground = CreateSolidBrush(RGB(255, 255, 255)); // Create a white background brush
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MYICON)); // Load the window icon

    RegisterClassW(&wc); // Register the window class

    std::wstring windowTitle = L"Manage Multiple Open Windows - Total Windows Seelcted: " + std::to_wstring(0);
    HWND hwnd = CreateWindowExW(
        0, // Extended window style
        CLASS_NAME, // Name of the window class
        windowTitle.c_str(), // Window title
        WS_OVERLAPPED | WS_SYSMENU | WS_VSCROLL | WS_EX_COMPOSITED, // Window style
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, // Position and size of the window
        NULL, // No parent window
        NULL, // No menu
        hInstance, // Instance handle
        NULL // No additional parameters
    );

    if (hwnd == NULL) {
        return 0; // Return 0 if the window could not be created
    }

    HICON hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MYICON)); // Load the window icon
    SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon); // Set the small window icon
    SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon); // Set the large window icon

    ShowWindow(hwnd, SW_HIDE); // Hide the window at startup
    UpdateWindow(hwnd); // Update the window

    MSG msg = {}; // Initialize a MSG structure
    while (GetMessage(&msg, NULL, 0, 0)) { // Message loop
        TranslateMessage(&msg); // Translate the message (e.g., keyboard input)
        DispatchMessage(&msg); // Send the message to the window procedure
    }

    return 0; // Return 0 when the application exits
}