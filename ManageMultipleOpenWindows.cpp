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
#include <winuser.h>
#include <stdexcept>
#include <dwmapi.h>
#include <thread>
#pragma comment(lib, "Dwmapi.lib")
#pragma comment(lib, "psapi.lib") // Link psapi.lib library

extern "C" UINT GetDpiForWindow(HWND hwnd);
// Manuelle Deklaration der Typdefinition und der Funktion
typedef HANDLE DPI_AWARENESS_CONTEXT;
#ifndef DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2
#define DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2 ((DPI_AWARENESS_CONTEXT)-4)
#endif

extern "C" BOOL SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT value);

#define WM_TRAYICON (WM_USER + 1) // Define a custom message for the tray icon
#define WM_UPDATE_LIST (WM_USER + 2) // Define a custom message to update the window list
#define IDC_SEARCHBOX            101
#define IDC_ERASEBUTTON          201
#define IDC_GOTOBUTTON           301
#define ID_MINIMIZE             2000
#define ID_RESTORE              2001
#define ID_CLOSE                2002
#define ID_ARRANGE              2003 // Needs 2003-200x for enumerate all screens
#define ID_MAXIMIZE             2050
#define ID_MOVE_TO_SCREEN_BASE  2104
#define ID_SAVEANDCLOSE         2200
#define ID_BUTTON_START         2300
#define ID_FILE_EXIT            2060
#define ID_SIZE_MINIMIZE        2070
#define ID_PROCESSNAMESTART     3000
#define IDI_ICON_MINIMIZE       3101
#define IDI_ICON_ARRANGE        3102
#define IDI_ICON_CLOSE          3103
#define ID_EXIT_BUTTON          3200 
#define ID_CLOSE_PROGRAMM       3250
#define ID_QUITMENU_BUTTON      3300
#define ID_IMAGE_START          3400
#define ID_QUITMENU_IMAGE       3500
#define ID_EXIT_IMAGE           3600
#define ID_CTRL_X               3700
#define ID_CTRL_W               3710
#define ID_CTRL_I               3720
#define ID_CTRL_A               3730
#define ID_CTRL_O               3740
#define ID_CTRL_R               3750
#define ID_CTRL_M               3760
#define ID_CTRL_N               3770
#define HOTKEY_ID               9000
#define HOTKEY_EXIT             9001

// Struktur zur Speicherung der Timer-IDs und ihrer Intervalle
struct TimerInfo {
    UINT_PTR id;
    int interval;
};

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
LRESULT CALLBACK CustomMenuProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam); // Window procedure to handle messages
void CreateCustomMenu(HWND parentHwnd, POINT pt);
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
void ShowTemporaryTiles(HWND hwnd, int screenIndex);
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
int highlightedRow = 0; // Globale Variable zur Speicherung der hervorgehobenen Zeile
int highlightedWindowRow = -1; // Globale Variable zur Speicherung der hervorgehobenen Fensterzeile
bool isRedrawPending = false;
int screenCount = 1;
int textWidth = 900;
static int defaultYPos = -1;
static int defaultXPos = 0;
bool blinkState = false; // Globale Variable für den Blinkzustand
int globalScreenIndexChosen = 1;
int currentLine = -1;
int globalTotalChecked = 0;
HWND hwndOverlay = NULL; // Globale Variable zum Speichern des Overlay-Fenster-Handles
HANDLE hThread = NULL;
bool g_bThreadRunning = true;
const UINT_PTR BLINKING_TIMER_ID = 1;
int BLINKING_TIMER_ID_time = 250;
const UINT_PTR TOOLTIP_TIMER_ID = 2;
int TOOLTIP_TIMER_ID_time = 3000;
const UINT_PTR TRAY_CONTEXTMENU_TIMER_ID = 3;
int TRAY_CONTEXTMENU_TIMER_ID_time = 100;
HWND lastHoveredImage = NULL;
HICON lastOriginalIcon = NULL;
bool globalTotalCheckedIgnore = false;

std::vector<TimerInfo> timers = {
    {BLINKING_TIMER_ID, BLINKING_TIMER_ID_time},
    {TOOLTIP_TIMER_ID, TOOLTIP_TIMER_ID_time},
    {TRAY_CONTEXTMENU_TIMER_ID, TRAY_CONTEXTMENU_TIMER_ID_time}
};

HWND hwndTT;
HWND hSearchBox;
HWND hEraseButton;
HWND hGoToButton;
HMENU hMenu;

bool operator==(const RECT& lhs, const RECT& rhs) {
    return lhs.left == rhs.left &&
           lhs.top == rhs.top &&
           lhs.right == rhs.right &&
           lhs.bottom == rhs.bottom;
}

void CheckAndStopTimers(HWND hwnd, std::vector<TimerInfo>& timers, std::vector<BOOL>& timerStates) {
    // Implementiere die Logik zum Prüfen und Stoppen der Timer
    for (size_t i = 0; i < timers.size(); ++i) {
        timerStates[i] = KillTimer(hwnd, timers[i].id);
    }
}

void RestartTimers(HWND hwnd, std::vector<TimerInfo>& timers, std::vector<BOOL>& timerStates) {
    // Implementiere die Logik zum Neustarten der Timer
    for (size_t i = 0; i < timers.size(); ++i) {
        if (timerStates[i]) {
            SetTimer(hwnd, timers[i].id, timers[i].interval, nullptr);
        }
    }
}


BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam) {
    ////////std::cout << "EnumWindowsProc" << std::endl;
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

int getTaskbarHeight(HWND hwnd) {
    ////////std::cout << "getTaskbarHeight" << std::endl;
    HWND taskbar = FindWindow(L"Shell_TrayWnd", NULL);
    if (taskbar) {
        RECT rect;
        if (GetWindowRect(taskbar, &rect)) {
            UINT dpi = GetDpiForWindow(hwnd);
            float scaleFactor = dpi / 96.0f; // 96 ist der Standard-DPI-Wert
            int taskbarHeight = (rect.bottom - rect.top) / scaleFactor;
            //////////std::cout << "taskbar height: " << taskbarHeight << std::endl;
            return taskbarHeight;
        }
    }
    return 0; // Fehlerfall
}

// Function to return open windows
std::vector<WindowInfo> getOpenWindows() {
    ////////std::cout << "getOpenWindows" << std::endl;
    std::vector<WindowInfo> windows; // Vector to store windows
    EnumWindows(EnumWindowsProc, reinterpret_cast<LPARAM>(&windows)); // Enumerate windows
    return windows; // Return the list of windows
}

// Function to minimize the window to the tray
void MinimizeToTray(HWND hwnd) {
    ////////std::cout << "MinimizeToTray" << std::endl;
    std::vector<BOOL> timerStates(timers.size());
    CheckAndStopTimers(hwnd, timers, timerStates);
    ShowWindow(hwnd, SW_HIDE); // Hide the window
}

// Function to create the tray icon
void CreateTrayIcon(HWND hwnd) {
    ////////std::cout << "CreateTrayIcon" << std::endl;
    NOTIFYICONDATAW nid = {}; // Structure for the tray icon
    nid.cbSize = sizeof(nid); // Size of the structure
    nid.hWnd = hwnd; // Handle of the window
    nid.uID = 1; // ID of the tray icon
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP; // Flags for the tray icon
    nid.uCallbackMessage = WM_TRAYICON; // Message for the tray icon
    nid.hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_MYICON)); // Load the icon
    wcscpy(nid.szTip, L"Minimize, Maximize, Restore, Close, Arrange or Move Many Windows at Once"); // Tooltip for the tray icon
    Shell_NotifyIconW(NIM_ADD, &nid); // Add the tray icon
}

// Function to remove the tray icon
void RemoveTrayIcon(HWND hwnd) {
    ////////std::cout << "RemoveTrayIcon" << std::endl;
    NOTIFYICONDATAW nid = {}; // Structure for the tray icon
    nid.cbSize = sizeof(nid); // Size of the structure
    nid.hWnd = hwnd; // Handle of the window
    nid.uID = 1; // ID of the tray icon
    Shell_NotifyIconW(NIM_DELETE, &nid); // Remove the tray icon
}

// Function to display the tray menu
void ShowTrayMenu(HWND hwnd){
    ////////std::cout << "ShowTrayMenu" << std::endl;
    POINT pt; // Struktur für die Cursorposition
    GetCursorPos(&pt); // Cursorposition abrufen
    //SetForegroundWindow(hwnd); // Fenster in den Vordergrund bringen

    // Benutzerdefiniertes Menü anzeigen
    CreateCustomMenu(hwnd, pt);

    // Optional: Standard-Popup-Menü hinzufügen
    /*HMENU hMenu = CreatePopupMenu(); // Popup-Menü erstellen
    AppendMenuW(hMenu, MF_STRING, ID_TRAY_EXIT, L"Exit"); // Menüeintrag hinzufügen
    TrackPopupMenu(hMenu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hwnd, NULL); // Menü anzeigen
    DestroyMenu(hMenu);*/ // Menü zerstören
}

// Function to process messages
void ProcessMessages() {
    ////////std::cout << "ProcessMessages" << std::endl;
    MSG msg; // Structure for the message
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) { // Retrieve messages
        TranslateMessage(&msg); // Translate the message
        DispatchMessage(&msg); // Dispatch the message
    }
}

// Function to save the current windows
void SaveCurrentWindows() {
    ////////std::cout << "SaveCurrentWindows" << std::endl;
    currentWindows = getOpenWindows(); // Save the current windows
}

bool CompareMonitors(const MonitorInfo& a, const MonitorInfo& b) {
    ////////std::cout << "CompareMonitors" << std::endl;
    return a.rect.left < b.rect.left;
}

bool CompareMonitorsWidthAndHeight(const MonitorInfo& a, const MonitorInfo& b) {
    ////////std::cout << "CompareMonitorsWidthAndHeight" << std::endl;
    int widthA = a.rect.right - a.rect.left;
    int widthB = b.rect.right - b.rect.left;
    
    if (widthA != widthB) {
        return widthA > widthB;
    } else {
        int heightA = a.rect.bottom - a.rect.top;
        int heightB = b.rect.bottom - b.rect.top;
        return heightA > heightB;
    }
}

RECT GetScreenRect(int screenIndex) {
    ////////std::cout << "GetScreenRect" << std::endl;
    std::vector<MonitorInfo> monitors;
    EnumDisplayMonitors(NULL, NULL, MonitorEnumProc, (LPARAM)&monitors);

    if (screenIndex < 0 || screenIndex >= monitors.size()) {
        //MessageBox(NULL, "Ungueltiger Bildschirm-Index", "Fehler", MB_OK | MB_ICONERROR);
        return {0, 0, 0, 0};
    }

    return monitors[screenIndex].rect;
}

BOOL CALLBACK RedrawWindowCallback(HWND hwnd, LPARAM lParam) {
    ////////std::cout << "RedrawWindowCallback" << std::endl;
    RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
    return TRUE;
}

void ClearTemporaryTiles() {
    if (hThread != NULL) {
        // Setzen Sie das Flag, um den Thread zu beenden
        g_bThreadRunning = false;

        // Senden Sie WM_QUIT an den Thread
        PostThreadMessage(GetThreadId(hThread), WM_QUIT, 0, 0);

        // Warten, bis der Thread beendet ist
        WaitForSingleObject(hThread, INFINITE);
        CloseHandle(hThread);
        hThread = NULL;
    }

    // Fenster zerstören
    if (hwndOverlay != NULL) {
        globalTotalCheckedIgnore = false;
        DestroyWindow(hwndOverlay);
        hwndOverlay = NULL;
    }
}

void DrawRectangleWithScaling(HDC hdc, HWND hwnd, RECT screenRect, int x, int y, int windowWidth, int windowHeight) {
    ////////std::cout << "DrawRectangleWithScaling" << std::endl;
    // Ermitteln des DPI-Werts für das Fenster
    UINT dpi = GetDpiForWindow(hwnd);
    float scaleFactor = dpi / 96.0f; // 96 ist der Standard-DPI-Wert

    //////////std::cout << "scaleFactor: " << scaleFactor << std::endl;

    // Anpassen der Koordinaten basierend auf dem Skalierungsfaktor
    RECT tileRect = {
        static_cast<LONG>(screenRect.left + x * scaleFactor),
        static_cast<LONG>(screenRect.top + y * scaleFactor),
        static_cast<LONG>(screenRect.left + (x + windowWidth) * scaleFactor),
        static_cast<LONG>(screenRect.top + (y + windowHeight) * scaleFactor)
    };

    // Zeichnen des Rechtecks
    Rectangle(hdc, tileRect.left, tileRect.top, tileRect.right, tileRect.bottom);
}

LRESULT CALLBACK OverlayWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            // Zeichnen Sie hier die Tiles
            RECT rect;
            GetClientRect(hwnd, &rect);

            // Beispielhafte Zeichnung der Tiles
            HPEN hPen = CreatePen(PS_SOLID, 5, RGB(255, 0, 0));
            HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
            HBRUSH hBrush = (HBRUSH)GetStockObject(NULL_BRUSH);
            HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hBrush);

            int numWindows = globalTotalChecked;
            if (globalTotalCheckedIgnore || numWindows == 0) numWindows = 1;

            if (numWindows > 0) {
                int cols = static_cast<int>(ceil(sqrt(numWindows)));
                int rows = (numWindows + cols - 1) / cols;
                int windowWidth = (rect.right - rect.left) / cols;
                int windowHeight = (rect.bottom - rect.top) / rows;

                int x = 0, y = 0;
                for (int counter = 0; counter < numWindows; counter++) {
                    RECT tileRect = {rect.left + x, rect.top + y, rect.left + x + windowWidth - 10, rect.top + y + windowHeight - 10};
                    Rectangle(hdc, tileRect.left, tileRect.top, tileRect.right, tileRect.bottom);

                    x += windowWidth;
                    if (x >= rect.right - 5) {
                        x = 0;
                        y += windowHeight;
                    }
                }
            }

            SelectObject(hdc, hOldPen);
            SelectObject(hdc, hOldBrush);
            DeleteObject(hPen);

            EndPaint(hwnd, &ps);
            return 0;
        }
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}

int ShowMessageBoxAndHandleTimers(HWND hwnd, const std::wstring& message, const std::wstring& title, UINT type, std::vector<TimerInfo>& timers) {
    // Timer prüfen und stoppen
    std::vector<BOOL> timerStates(timers.size());
    CheckAndStopTimers(hwnd, timers, timerStates);

    // MessageBox anzeigen und Ergebnis speichern
    int result = MessageBoxW(hwnd, message.c_str(), title.c_str(), type | MB_DEFBUTTON2);

    // Timer neu starten
    RestartTimers(hwnd, timers, timerStates);

    // Ergebnis zurückgeben
    return result;
}

void ShowTemporaryTiles(HWND hwnd, int screenIndex) { // Diese Funktion wird nicht mehr verwendet, sondern OverlayWindowProc()
    try {
        RECT screenRect = GetScreenRect(screenIndex);
        int screenWidth = screenRect.right - screenRect.left;
        int screenHeight = screenRect.bottom - screenRect.top - getTaskbarHeight(hwnd);

        int numWindows = globalTotalChecked;
        if (globalTotalCheckedIgnore || numWindows == 0) numWindows = 1;

        if (numWindows > 0) {
            int cols = static_cast<int>(ceil(sqrt(numWindows)));
            int rows = (numWindows + cols - 1) / cols;
            int windowWidth = screenWidth / cols;
            int windowHeight = screenHeight / rows;

            HDC hdcScreen = GetDC(hwnd);
            if (!hdcScreen) {
                throw std::runtime_error("GetDC failed");
            }

            HPEN hPen = CreatePen(PS_SOLID, 5, RGB(0, 0, 255));
            HPEN hOldPen = (HPEN)SelectObject(hdcScreen, hPen);
            HBRUSH hBrush = (HBRUSH)GetStockObject(NULL_BRUSH);
            HBRUSH hOldBrush = (HBRUSH)SelectObject(hdcScreen, hBrush);

            int x = 0, y = 0;
            for (int counter = 0; counter < numWindows; counter++) {
                RECT tileRect = {screenRect.left + x, screenRect.top + y, screenRect.left + x + windowWidth - 10, screenRect.top + y + windowHeight - 10};
                Rectangle(hdcScreen, tileRect.left, tileRect.top, tileRect.right, tileRect.bottom);

                x += windowWidth;
                if (x >= screenWidth - 20) {
                    x = 0;
                    y += windowHeight;
                }
            }

            SelectObject(hdcScreen, hOldPen);
            SelectObject(hdcScreen, hOldBrush);
            DeleteObject(hPen);
            ReleaseDC(hwnd, hdcScreen);

            SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
            SetForegroundWindow(hwnd);
        }
    } catch (const std::exception& e) {
        //MessageBox(hwnd, std::wstring(e.what(), e.what() + strlen(e.what())).c_str(), L"Error in ShowTemporaryTiles", MB_OK);
    }
}

DWORD WINAPI ShowTemporaryTilesThread(LPVOID lpParam) {
    auto params = static_cast<std::pair<HWND, int>*>(lpParam);

    const wchar_t CLASS_NAME[] = L"OverlayWindowClass";
    WNDCLASS wc = {};
    wc.lpfnWndProc = OverlayWindowProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);

    RegisterClass(&wc);

    // Holen Sie sich die Bildschirmkoordinaten für den angegebenen Bildschirm
    RECT screenRect = GetScreenRect(params->second);
    int screenWidth = screenRect.right - screenRect.left;
    int screenHeight = screenRect.bottom - screenRect.top;

    // Debug-Ausgaben
    /*std::wcout << L"Screen Index: " << params->second << std::endl;
    std::wcout << L"Screen Rect: (" << screenRect.left << L", " << screenRect.top << L") - ("
               << screenRect.right << L", " << screenRect.bottom << L")" << std::endl;
    std::wcout << L"Screen Width: " << screenWidth << L", Screen Height: " << screenHeight << std::endl;*/

    // Erstellen Sie das Overlay-Fenster mit den richtigen Koordinaten und Abmessungen
    hwndOverlay = CreateWindowEx(
        WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TRANSPARENT,
        CLASS_NAME,
        L"Overlay",
        WS_POPUP,
        screenRect.left, screenRect.top, screenWidth, screenHeight,
        NULL,
        NULL,
        GetModuleHandle(NULL),
        NULL
    );

    if (hwndOverlay == NULL) {
        delete params;
        throw std::runtime_error("CreateWindowEx failed");
    }

    // Setzen Sie die Transparenz des Overlay-Fensters
    SetLayeredWindowAttributes(hwndOverlay, RGB(0, 0, 0), 0, LWA_COLORKEY);

    ShowWindow(hwndOverlay, SW_SHOW);
    UpdateWindow(hwndOverlay);

    // Stellen Sie sicher, dass das Overlay-Fenster auf dem richtigen Bildschirm positioniert ist
    SetWindowPos(hwndOverlay, HWND_TOPMOST, screenRect.left, screenRect.top, screenWidth, screenHeight, SWP_SHOWWINDOW);

    RECT overlayRect;
    GetWindowRect(hwndOverlay, &overlayRect);
    /*std::wcout << L"Overlay Window Rect: (" << overlayRect.left << L", " << overlayRect.top << L") - ("
               << overlayRect.right << L", " << overlayRect.bottom << L")" << std::endl;*/

    // Erzwingen Sie das Neuzeichnen des Fensters
    RedrawWindow(hwndOverlay, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);

    MSG msg = {};
    while (g_bThreadRunning) {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                g_bThreadRunning = false;
            } else {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
    }

    delete params;
    return 0;
}


void StartShowTemporaryTilesInThread(HWND hwnd, int screenIndex) {
    g_bThreadRunning = true;
    auto params = new std::pair<HWND, int>(hwnd, screenIndex);
    hThread = CreateThread(
        NULL,
        0,
        ShowTemporaryTilesThread,
        params,
        0,
        NULL
    );

    if (hThread == NULL) {
        delete params;
        throw std::runtime_error("CreateThread failed");
    }
}

BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData) {
    ////////std::cout << "MonitorEnumProc" << std::endl;
    std::vector<MonitorInfo>* monitors = reinterpret_cast<std::vector<MonitorInfo>*>(dwData);
    
    MONITORINFOEX mi;
    mi.cbSize = sizeof(mi);
    if (GetMonitorInfo(hMonitor, &mi)) {
        monitors->push_back({static_cast<int>(monitors->size()), mi.rcMonitor, L""});
    }
    ////////std::cout << "MonitorEnumProc beendet" << std::endl;
    return TRUE;
}

bool IsValidInput(const wchar_t* input) {
    ////////std::cout << "IsValidInput" << std::endl;
    while (*input) {
        if (!iswalnum(*input) && // Alphanumerische Zeichen
            *input != L'\\' && *input != L'/' && // Backslash und Forwardslash
            *input != L'*' && *input != L'-' && *input != L'+' && *input != L'=' && *input != L'!' &&
            *input != L'{' && *input != L'}' && *input != L'&' && *input != L'%' && *input != L'#' &&
            *input != L'@' && *input != L'(' && *input != L')' && *input != L'^' && *input != L'~' &&
            *input != L'é' && *input != L'è' && *input != L'à' && *input != L'$' && *input != L'£' &&
            *input != L'.' && *input != L'<' && *input != L'>' &&
            *input != L'ä' && *input != L'ö' && *input != L'ü' && *input != L'ß') { // Umlaute
            return false;
        }
        input++;
    }
    return true;
}

void MoveWindowToScreen(HWND hwnd, int screenIndex) {
    if (!IsWindow(hwnd)) {
        return;
    }

    WINDOWPLACEMENT wp;
    wp.length = sizeof(WINDOWPLACEMENT);
    if (!GetWindowPlacement(hwnd, &wp)) {
        return;
    }

    bool wasMaximized = (wp.showCmd == SW_SHOWMAXIMIZED);

    if (wasMaximized) {
        ShowWindow(hwnd, SW_RESTORE);
    }

    RECT windowRect;
    if (!GetWindowRect(hwnd, &windowRect)) {
        return;
    }

    RECT screenRect = GetScreenRect(screenIndex);
    if (screenRect.left == 0 && screenRect.top == 0 && screenRect.right == 0 && screenRect.bottom == 0) {
        return;
    }

    int windowWidth = windowRect.right - windowRect.left;
    int windowHeight = windowRect.bottom - windowRect.top;

    // Calculate the relative position on the current screen
    HMONITOR hMonitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
    MONITORINFO mi;
    mi.cbSize = sizeof(MONITORINFO);
    if (!GetMonitorInfo(hMonitor, &mi)) {
        return;
    }

    int currentScreenX = mi.rcMonitor.left;
    int currentScreenY = mi.rcMonitor.top;

    int relativeX = windowRect.left - currentScreenX;
    int relativeY = windowRect.top - currentScreenY;

    // Calculate the new position on the target screen
    int newX = screenRect.left + relativeX;
    int newY = screenRect.top + relativeY;

    // Debug output
    //std::cout << "MoveWindowToScreen Debug Info:\n";
    //std::cout << "Current Screen X: " << currentScreenX << "\n";
    //std::cout << "Current Screen Y: " << currentScreenY << "\n";
    //std::cout << "Window Rect: (" << windowRect.left << ", " << windowRect.top << ", " << windowRect.right << ", " << windowRect.bottom << ")\n";
    //std::cout << "Screen Rect: (" << screenRect.left << ", " << screenRect.top << ", " << screenRect.right << ", " << screenRect.bottom << ")\n";
    //std::cout << "Relative X: " << relativeX << "\n";
    //std::cout << "Relative Y: " << relativeY << "\n";
    //std::cout << "New X: " << newX << "\n";
    //std::cout << "New Y: " << newY << "\n";

    // Move the window to the new position on the target screen
    if (!SetWindowPos(hwnd, NULL, newX, newY, windowWidth, windowHeight, SWP_NOZORDER | SWP_SHOWWINDOW)) {
        return;
    }

    if (wasMaximized) {
        ShowWindow(hwnd, SW_MAXIMIZE);
    }
}

HBITMAP CaptureAndResizeScreen(HWND hwnd, RECT rect, int width, int height) {
    ////////std::cout << "CaptureAndResizeScreen" << std::endl;
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
    ////////std::cout << "AddMenuItemWithImage" << std::endl;
    MENUITEMINFOW mii = { sizeof(MENUITEMINFOW) };
    mii.fMask = MIIM_BITMAP | MIIM_STRING | MIIM_ID;
    mii.wID = uIDNewItem;
    mii.dwTypeData = const_cast<LPWSTR>(text.c_str());
    mii.cch = static_cast<UINT>(text.size());
    mii.hbmpItem = hBitmap;
    InsertMenuItemW(hMenu, uIDNewItem, FALSE, &mii);
}

void MoveWindowToPrimaryMonitor(HWND hwnd) {
    ////////std::cout << "MoveWindowToPrimaryMonitor" << std::endl;
    // Get the primary monitor's work area
    RECT primaryMonitorRect;
    SystemParametersInfo(SPI_GETWORKAREA, 0, &primaryMonitorRect, 0);

    // Get the window's current size
    RECT windowRect;
    GetWindowRect(hwnd, &windowRect);

    // Calculate new position to center the window on the primary monitor
    int newX = primaryMonitorRect.left + (primaryMonitorRect.right - primaryMonitorRect.left - (windowRect.right - windowRect.left)) / 2;
    int newY = primaryMonitorRect.top + (primaryMonitorRect.bottom - primaryMonitorRect.top - (windowRect.bottom - windowRect.top)) / 2;

    // Move the window
    SetWindowPos(hwnd, HWND_TOP, newX, newY, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
}

HICON InvertIconColors(HICON hIcon) {
    ICONINFO iconInfo;
    GetIconInfo(hIcon, &iconInfo);

    HDC hdc = GetDC(NULL);
    HDC hdcMem = CreateCompatibleDC(hdc);
    HBITMAP hbmOld = (HBITMAP)SelectObject(hdcMem, iconInfo.hbmColor);

    BITMAP bm;
    GetObject(iconInfo.hbmColor, sizeof(bm), &bm);

    // Invertiere die Farben
    for (int y = 0; y < bm.bmHeight; y++) {
        for (int x = 0; x < bm.bmWidth; x++) {
            COLORREF color = GetPixel(hdcMem, x, y);
            SetPixel(hdcMem, x, y, RGB(255 - GetRValue(color), 255 - GetGValue(color), 255 - GetBValue(color)));
        }
    }

    SelectObject(hdcMem, hbmOld);
    DeleteDC(hdcMem);
    ReleaseDC(NULL, hdc);

    // Erstelle ein neues Icon mit den invertierten Farben
    HICON hIconInverted = CreateIconIndirect(&iconInfo);

    // Bereinige
    DeleteObject(iconInfo.hbmColor);
    DeleteObject(iconInfo.hbmMask);

    return hIconInverted;
}

void CreateMoveToScreenMenu(HMENU hMenu) {
    ////////std::cout << "CreateMoveToScreenMenu" << std::endl;
    HMENU hMoveToScreenMenu = CreateMenu();
    std::vector<MonitorInfo> monitors;
    EnumDisplayMonitors(NULL, NULL, MonitorEnumProc, reinterpret_cast<LPARAM>(&monitors));

    // Sortiere die Monitore nach ihrer Breite
    std::sort(monitors.begin(), monitors.end(), CompareMonitorsWidthAndHeight);
    /*for (const auto& monitor : monitors) {
    std::wcout << L"Monitor " << monitor.index << L": " 
               << (monitor.rect.right - monitor.rect.left) << L" width" << std::endl;
    }*/
 
    auto largestMonitor = monitors[0];
    auto secondLargestMonitor = monitors[0];
    if (monitors.size() > 1) {
        secondLargestMonitor = monitors[1];
    }
    int widthLargestMonitor = largestMonitor.rect.bottom - largestMonitor.rect.top;
    int widthSecondLargestMonitor = secondLargestMonitor.rect.bottom - secondLargestMonitor.rect.top;

    //////////std::cout << largestMonitor.index << std::endl;
    //////////std::cout << largestMonitor.rect.right - largestMonitor.rect.left << " " << largestMonitor.rect.bottom - largestMonitor.rect.top << std::endl;
    int width = largestMonitor.rect.right - largestMonitor.rect.left;
    int height = largestMonitor.rect.bottom - largestMonitor.rect.top;
    int newWidth, newHeight;
    int firstIconHeight = 0; // Variable to store the height of the first icon

    // Berechne das Seitenverhältnis für den breitesten Monitor
    if (width > height) {
        newWidth = 25;
        newHeight = static_cast<int>(35.0 * height / width);
    } else {
        newHeight = 25;
        newWidth = static_cast<int>(35.0 * width / height);
    }
    firstIconHeight = newHeight; // Speichere die Höhe des ersten Icons
    
    // Sortiere die Monitore nach ihrer linken Koordinate
    std::sort(monitors.begin(), monitors.end(), CompareMonitors);

    if (monitors.size() > 1) { // Check if more than one screen is present
        screenCount = -1;

        for (const auto& monitor : monitors) {
            screenCount++;
            width = monitor.rect.right - monitor.rect.left;
            height = monitor.rect.bottom - monitor.rect.top;
            newHeight = firstIconHeight;
            std::wstring menuText = L"Screen " + std::to_wstring(screenCount + 1) + L" (" + 
                                    std::to_wstring(width) + L"x" + 
                                    std::to_wstring(height) + L")";

            /*if (screenCount == 0) {
                // Berechne das Seitenverhältnis für den ersten Monitor
                if (width > height) {
                    newWidth = 25;
                    newHeight = static_cast<int>(35.0 * height / width);
                } else {
                    newHeight = 25;
                    newWidth = static_cast<int>(35.0 * width / height);
                }
                firstIconHeight = newHeight; // Speichere die Höhe des ersten Icons
            } else {*/
                // Berechne die Größe der Icons für die weiteren Monitore
                //newWidth = static_cast<int>(newHeight * width / height);
                //////////std::cout << largestMonitor.index << " " << monitor.index << std::endl;
                //////////std::cout << newWidth << " " << newHeight << std::endl;
                //////////std::cout << width << " " << height << std::endl;
                if (monitor.rect == largestMonitor.rect) newHeight = static_cast<int>(newHeight * widthLargestMonitor / widthSecondLargestMonitor);
                //////////std::cout << newWidth << " " << newHeight << std::endl;


            //}

            HBITMAP hBitmap = CaptureAndResizeScreen(NULL, monitor.rect, newWidth, newHeight);
            AddMenuItemWithImage(hMoveToScreenMenu, ID_MOVE_TO_SCREEN_BASE + monitor.index, hBitmap, menuText);
        }
        AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hMoveToScreenMenu, L"&Move Window(s)\tCtrl+M");
    } else {
        //Move to the same (and only) screen does not make sense, so no menu
        //AppendMenu(hMenu, MF_STRING, ID_MOVE_TO_SCREEN_BASE, L"Mo&ve Window(s)");
    }
}

// Funktion zum Trimmen von Leerzeichen
std::wstring trim(const std::wstring& str) {
    ////////std::cout << "trim" << std::endl;
    size_t first = str.find_first_not_of(L' ');
    if (first == std::wstring::npos) return L"";
    size_t last = str.find_last_not_of(L' ');
    return str.substr(first, last - first + 1);
}

// Funktion zum Umwandeln von Großbuchstaben in nur den ersten Großbuchstaben
std::wstring capitalizeIfAllCaps(const std::wstring& str) {
    ////////std::cout << "capitalizeIfAllCaps" << std::endl;
    bool allCaps = true;
    for (wchar_t c : str) {
        if (!std::iswupper(c)) {
            allCaps = false;
            break;
        }
    }
    std::wstring result = str;
    if (allCaps) {
        for (wchar_t& c : result) {
            c = std::towlower(c);
        }
        result[0] = std::towupper(result[0]);
        return result;
    }
    result[0] = std::towupper(result[0]);
    return result;
}

std::wstring toLower(const std::wstring& str) {
    ////////std::cout << "toLower" << std::endl;
    std::wstring lowerStr = str;
    std::transform(lowerStr.begin(), lowerStr.end(), lowerStr.begin(), ::towlower);
    return lowerStr;
}

void ShowTooltip(HWND hwnd, const wchar_t* message) {
    ////////std::cout << "ShowTooltip" << std::endl;
    TOOLINFO ti = { 0 };
    ti.cbSize = sizeof(TOOLINFO);
    ti.uFlags = TTF_SUBCLASS | TTF_CENTERTIP;
    ti.hwnd = hwnd;
    ti.hinst = GetModuleHandle(NULL);
    ti.lpszText = (LPWSTR)message;

    RECT rect;
    GetClientRect(hwnd, &rect);
    ti.rect = rect;

    HWND hwndTT = CreateWindowEx(WS_EX_TOPMOST, TOOLTIPS_CLASS, NULL,
        WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP, CW_USEDEFAULT, CW_USEDEFAULT,
        CW_USEDEFAULT, CW_USEDEFAULT, hwnd, NULL, GetModuleHandle(NULL), NULL);

    SendMessage(hwndTT, TTM_ADDTOOL, 0, (LPARAM)&ti);
    SendMessage(hwndTT, TTM_SETMAXTIPWIDTH, 0, 300);
    SendMessage(hwndTT, TTM_TRACKPOSITION, 0, (LPARAM)MAKELONG(rect.left + 10, rect.top + 10));
    SendMessage(hwndTT, TTM_TRACKACTIVATE, TRUE, (LPARAM)&ti);

    // Timer to hide the tooltip after a few seconds
    SetTimer(hwnd,TOOLTIP_TIMER_ID, TOOLTIP_TIMER_ID_time, NULL); // 3 seconds
}

void CreateArrangeOnScreenMenu(HMENU hMenu) {
    ////////std::cout << "CreateArrangeOnScreenMenu" << std::endl;
    HMENU hArrangeOnScreenMenu = CreateMenu();
    std::vector<MonitorInfo> monitors;
    EnumDisplayMonitors(NULL, NULL, MonitorEnumProc, reinterpret_cast<LPARAM>(&monitors));
    
    // Sortiere die Monitore nach ihrer Breite
    std::sort(monitors.begin(), monitors.end(), CompareMonitorsWidthAndHeight);
    /*for (const auto& monitor : monitors) {
    std::wcout << L"Monitor " << monitor.index << L": " 
               << (monitor.rect.right - monitor.rect.left) << L" width" << std::endl;
    }*/

    auto largestMonitor = monitors[0];
    auto secondLargestMonitor = monitors[0];
    if (monitors.size() > 1) {
        secondLargestMonitor = monitors[1];
    }
    int widthLargestMonitor = largestMonitor.rect.bottom - largestMonitor.rect.top;
    int widthSecondLargestMonitor = secondLargestMonitor.rect.bottom - secondLargestMonitor.rect.top;

    //////////std::cout << largestMonitor.index << std::endl;
    //////////std::cout << largestMonitor.rect.right - largestMonitor.rect.left << " " << largestMonitor.rect.bottom - largestMonitor.rect.top << std::endl;
    int width = largestMonitor.rect.right - largestMonitor.rect.left;
    int height = largestMonitor.rect.bottom - largestMonitor.rect.top;
    int newWidth, newHeight;
    int firstIconHeight = 0; // Variable to store the height of the first icon

    // Berechne das Seitenverhältnis für den breitesten Monitor
    if (width > height) {
        newWidth = 25;
        newHeight = static_cast<int>(35.0 * height / width);
    } else {
        newHeight = 25;
        newWidth = static_cast<int>(35.0 * width / height);
    }
    firstIconHeight = newHeight; // Speichere die Höhe des ersten Icons
    
    // Sortiere die Monitore nach ihrer linken Koordinate
    std::sort(monitors.begin(), monitors.end(), CompareMonitors);

    //if (monitors.size() > 1) { // Check if more than one screen is present
        screenCount = -1;

        for (const auto& monitor : monitors) {
            screenCount++;
            width = monitor.rect.right - monitor.rect.left;
            height = monitor.rect.bottom - monitor.rect.top;
            newHeight = firstIconHeight;
            std::wstring menuText = L"Screen " + std::to_wstring(screenCount + 1) + L" (" + 
                                    std::to_wstring(width) + L"x" + 
                                    std::to_wstring(height) + L")";

            /*if (screenCount == 0) {
                // Berechne das Seitenverhältnis für den ersten Monitor
                if (width > height) {
                    newWidth = 25;
                    newHeight = static_cast<int>(35.0 * height / width);
                } else {
                    newHeight = 25;
                    newWidth = static_cast<int>(35.0 * width / height);
                }
                firstIconHeight = newHeight; // Speichere die Höhe des ersten Icons
            } else {*/
                // Berechne die Größe der Icons für die weiteren Monitore
                //newWidth = static_cast<int>(newHeight * width / height);
                //////////std::cout << largestMonitor.index << " " << monitor.index << std::endl;
                //////////std::cout << newWidth << " " << newHeight << std::endl;
                //////////std::cout << width << " " << height << std::endl;
                if (monitor.rect == largestMonitor.rect) newHeight = static_cast<int>(newHeight * widthLargestMonitor / widthSecondLargestMonitor);
                //////////std::cout << newWidth << " " << newHeight << std::endl;


            //}

            HBITMAP hBitmap = CaptureAndResizeScreen(NULL, monitor.rect, newWidth, newHeight);
            AddMenuItemWithImage(hArrangeOnScreenMenu, ID_ARRANGE + monitor.index, hBitmap, menuText);
        }
        AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hArrangeOnScreenMenu, L"Arra&nge Window(s)\tCtrl+N");
    //} else {
    //    AppendMenu(hMenu, MF_STRING, ID_ARRANGE, L"&Arrange Window(s)");
    //}
    ////////std::cout << "CreateArrangeOnScreenMenu beendet" << std::endl;
}

// Function to check if the windows have changed
bool HasWindowsChanged() {
    ////////std::cout << "HasWindowsChanged" << std::endl;
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
    ////////std::cout << "MoveWindowToMainMonitor" << std::endl;
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

void AdjustWindowToFitMenu(HWND hwnd) {
    HMENU hMenu = GetMenu(hwnd);
    if (!hMenu) return;

    HDC hdc = GetDC(hwnd);
    HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
    SelectObject(hdc, hFont);

    // DPI-Skalierung abrufen
    UINT dpi = GetDpiForWindow(hwnd);
    float scaleFactor = dpi / 96.0f;

    int totalWidth = 0;
    int itemCount = GetMenuItemCount(hMenu);

    for (int i = 0; i < itemCount; ++i) {
        MENUITEMINFO mii = { sizeof(MENUITEMINFO) };
        mii.fMask = MIIM_STRING;
        wchar_t itemText[256];
        mii.dwTypeData = itemText;
        mii.cch = sizeof(itemText) / sizeof(itemText[0]);
        GetMenuItemInfo(hMenu, i, TRUE, &mii);

        SIZE textSize;
        GetTextExtentPoint32(hdc, itemText, lstrlen(itemText), &textSize);
        totalWidth += static_cast<int>(textSize.cx * scaleFactor) + 25; // Füge 25 Pixel Puffer hinzu
    }

    ReleaseDC(hwnd, hdc);

    // Lege eine minimale Breite fest
    const int minWidth = 700; // Beispielwert, anpassen nach Bedarf
    int newWidth = std::max(totalWidth, minWidth);

    RECT windowRect;
    GetWindowRect(hwnd, &windowRect);

    RECT adjustedRect = {0, 0, newWidth, windowRect.bottom - windowRect.top};
    AdjustWindowRectEx(&adjustedRect, GetWindowLong(hwnd, GWL_STYLE), TRUE, GetWindowLong(hwnd, GWL_EXSTYLE));

    int newHeight = adjustedRect.bottom - adjustedRect.top;
    textWidth = newWidth;
    //SetWindowPos(hSearchBox, NULL, windowRect.left, windowRect.top, newWidth, newHeight, SWP_NOZORDER | SWP_NOMOVE);

}

// Function to allow sorting case-insensitive
bool caseInsensitiveCompare(const std::wstring& a, const std::wstring& b) {
    ////////std::cout << "caseInsensitiveCompare" << std::endl;
    std::wstring lowerA = a;
    std::wstring lowerB = b;
    std::transform(lowerA.begin(), lowerA.end(), lowerA.begin(), ::towlower);
    std::transform(lowerB.begin(), lowerB.end(), lowerB.begin(), ::towlower);
    return lowerA < lowerB;
}

void SetEditPlaceholder(HWND hwndEdit, const std::wstring& placeholder) {
    ////////std::cout << "SetEditPlaceholder" << std::endl;
    SetWindowText(hwndEdit, placeholder.c_str());
    SendMessage(hwndEdit, EM_SETSEL, 0, -1); // Markiere den gesamten Text
    SendMessage(hwndEdit, EM_SETSEL, -1, -1); // Entferne die Markierung
}

// Function to allow on-mouse-over tooltip
HWND CreateTooltip(HWND hwndParent) {
    ////////std::cout << "CreateTooltip" << std::endl;
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
    ////////std::cout << "UpdateWindowList" << std::endl;
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
    ////////std::cout << "AdjustWindowSize" << std::endl;
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
    int usableScreenHeight = screenHeight - titleBarHeight - 25 - getTaskbarHeight(hwnd); // Calculate the usable screen height (minus 50px to not be straight up to the bottom of the screen)
    int contentHeight = si.nMax + 30 + 40; // Calculate the content height
    int contentWidth = textWidth; // Set the content width
    int newHeight = std::min(contentHeight, usableScreenHeight); // Calculate the new window height

    // Calculate the new y-position
    int newYPos = yPos - (newHeight - height);

    /*if (newYPos + newHeight > screenHeight - getTaskbarHeight(hwnd)) { // Überprüfen Sie, ob das Fenster am unteren Rand des Bildschirms abgeschnitten wird
        newYPos = screenHeight - getTaskbarHeight(hwnd) - newHeight; // Verschieben Sie das Fenster nach oben, um es auf den Bildschirm zu passen
        if (newYPos < 0) {
            newYPos = 0; // Stellen Sie sicher, dass das Fenster nicht über den oberen Rand des Bildschirms hinaus verschoben wird
            newHeight = screenHeight - titleBarHeight - 25 - getTaskbarHeight(hwnd); // Passen Sie die Höhe an, um auf den Bildschirm zu passen
        }
        // Speichern Sie die neue Y-Koordinate als Standard
        defaultYPos = newYPos;
    }*/

    // Verwenden Sie die Standard-Y-Koordinate, wenn sie gesetzt ist
    if (defaultYPos != -1) {
        newYPos = defaultYPos;
    } else {
        newYPos = yPos; // Verwenden Sie die ursprüngliche Y-Koordinate, wenn keine Standard-Y-Koordinate gesetzt ist
    }

    SetWindowPos(hwnd, NULL, xPos, newYPos, contentWidth, newHeight, SWP_NOZORDER); // Setzen Sie die neue Fensterposition und -größe
}

// Wrapper-Funktion für CreateArrangeOnScreenMenu
DWORD WINAPI ThreadFuncCreateArrangeMenu(LPVOID lpParam) {
    HMENU hMenu = (HMENU)lpParam;
    CreateArrangeOnScreenMenu(hMenu);
    return 0;
}

// Wrapper-Funktion für CreateMoveToScreenMenu
DWORD WINAPI ThreadFuncCreateMoveMenu(LPVOID lpParam) {
    HMENU hMenu = (HMENU)lpParam;
    CreateMoveToScreenMenu(hMenu);
    return 0;
}

void ExecuteCreateArrangeAndMoveMenuInThreads(HMENU hMenu) {
    // Erstelle den ersten Thread
    HANDLE hThread1 = CreateThread(
        NULL,       // Standard-Sicherheitsattribute
        0,          // Standard-Stackgröße
        ThreadFuncCreateArrangeMenu, // Thread-Funktion
        hMenu,      // Parameter für die Thread-Funktion
        0,          // Standard-Erstellungsflags
        NULL        // Thread-ID (optional)
    );

    // Warte, bis der erste Thread beendet ist
    WaitForSingleObject(hThread1, INFINITE);

    // Schließe das Handle des ersten Threads
    CloseHandle(hThread1);

    // Erstelle den zweiten Thread
    HANDLE hThread2 = CreateThread(
        NULL,       // Standard-Sicherheitsattribute
        0,          // Standard-Stackgröße
        ThreadFuncCreateMoveMenu, // Thread-Funktion
        hMenu,      // Parameter für die Thread-Funktion
        0,          // Standard-Erstellungsflags
        NULL        // Thread-ID (optional)
    );

    // Warte, bis der zweite Thread beendet ist
    WaitForSingleObject(hThread2, INFINITE);

    // Schließe das Handle des zweiten Threads
    CloseHandle(hThread2);
}

void InitializeMenu(HWND hwnd) {
    ////////std::cout << "InitializeMenu" << std::endl;
    hMenu = CreateMenu();
    /*AppendMenu(hMenu, MF_STRING, ID_MINIMIZE, L"&Minimize Window(s)");
    AppendMenu(hMenu, MF_STRING, ID_MAXIMIZE, L"Ma&ximize Window(s)");
    AppendMenu(hMenu, MF_STRING, ID_RESTORE, L"&Restore Window(s)");

    // Füge die dynamischen Einträge direkt hinzu
    ExecuteCreateArrangeAndMoveMenuInThreads(hMenu);
    //CreateArrangeOnScreenMenu(hMenu);
    //CreateMoveToScreenMenu(hMenu);

    //AppendMenu(hMenu, MF_STRING, ID_CLOSE, L"&Close Window(s)");
    AppendMenu(hMenu, MF_STRING, ID_SAVEANDCLOSE, L"&Save And Close Window(s)");
    AppendMenu(hMenu, MF_STRING, ID_EXIT_BUTTON, L"&Exit");
    SetMenu(hwnd, hMenu);*/

    // File menu
    HMENU hFileMenu = CreateMenu();
    AppendMenu(hFileMenu, MF_STRING, ID_CLOSE_PROGRAMM, L"&Close Program to Tray\tCtrl+W");
    AppendMenu(hFileMenu, MF_STRING, ID_EXIT_BUTTON, L"E&xit Program\tCtrl+X");
    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hFileMenu, L"&File");

    // Size menu
    HMENU hSizeMenu = CreateMenu();
    AppendMenu(hSizeMenu, MF_STRING, ID_MINIMIZE, L"M&inimize Window(s)\tCtrl+I");
    AppendMenu(hSizeMenu, MF_STRING, ID_MAXIMIZE, L"M&aximize Window(s)\tCtrl+A");
    AppendMenu(hSizeMenu, MF_STRING, ID_RESTORE, L"&Restore Window(s)\tCtrl+R");
    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hSizeMenu, L"Minimize& / Maximize / Restore");

    // Arrange menu
    HMENU hArrangeMenu = CreateMenu();
    ExecuteCreateArrangeAndMoveMenuInThreads(hArrangeMenu);
    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hArrangeMenu, L"&Arrange / Move Window(s)");

    // Close menu
    HMENU hCloseMenu = CreateMenu();
    AppendMenu(hCloseMenu, MF_STRING, ID_CLOSE, L"&Close Window(s) without Saving");
    AppendMenu(hCloseMenu, MF_STRING, ID_SAVEANDCLOSE, L"&Save And Close Window(s)");
    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hCloseMenu, L"&Close / Save Window(s)");

    SetMenu(hwnd, hMenu);

}

void UpdateDynamicMenus(HWND hwnd) {
    ////////std::cout << "UpdateDynamicMenus" << std::endl;
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
    ////////std::cout << "InvalidateWindow" << std::endl;
    /*if (!isRedrawPending) {
        isRedrawPending = true;
        InvalidateWindow(hwnd);
    }*/
   InvalidateRect(hwnd, NULL, TRUE);
}

void RefreshWindowList(HWND hwnd) {
    ////////std::cout << "RefreshWindowList" << std::endl;
    // Invalidate and redraw the window immediately
    RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE); // Invalidate and redraw

    // Process messages
    ProcessMessages(); // Process messages

    // Update the window list
    UpdateWindowList(hwnd); // Update the window list
}

bool compareWindowsByName(const WindowInfo& a, const WindowInfo& b) {
    ////////std::cout << "compareWindowsByName" << std::endl;
    std::wstring nameA = a.title;
    std::wstring nameB = b.title;
    std::transform(nameA.begin(), nameA.end(), nameA.begin(), ::tolower);
    std::transform(nameB.begin(), nameB.end(), nameB.begin(), ::tolower);
    return nameA < nameB; // Sort windows by window name case-insensitive
}

// Function to confirm closing windows
bool ConfirmClose(HWND parentHwnd) {
    ////////std::cout << "ConfirmClose" << std::endl;
    // Bringe das Parent-Fenster in den Vordergrund
    SetForegroundWindow(parentHwnd);

    // Zeige das Bestätigungsdialogfeld an
    int result = ShowMessageBoxAndHandleTimers(parentHwnd, L"Do you really want to close the selected windows?", L"Confirmation", MB_YESNO | MB_ICONQUESTION | MB_TOPMOST, timers);
    //int result = MessageBoxW(parentHwnd, L"Do you really want to close the selected windows?", L"Confirmation", MB_YESNO | MB_ICONQUESTION | MB_TOPMOST);

    // Gib zurück, ob der Benutzer "Ja" gewählt hat
    return (result == IDYES);
}

void SearchAndCheck(const std::wstring& searchString, HWND hwnd) {
    ////////std::cout << "SearchAndCheck" << std::endl;
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
    bool foundAnything = false;
    if (lowerSearchString.length() >= 1) {
        for (auto& processName : processNames) {
            std::wstring lowerProcessName = toLower(processName);
            bool processMatch = lowerProcessName.find(lowerSearchString) != std::wstring::npos;
            if (processMatch) {
                expandedState[processName] = true;
            }

            for (auto& window : processWindowsMap[processName]) {
                std::wstring lowerWindowTitle = toLower(window.title);
                bool windowMatch = lowerWindowTitle.find(lowerSearchString) != std::wstring::npos;
                if (windowMatch) {
                    //window.checked = true;
                    window.visible = true; // Setze das sichtbare Attribut auf true, wenn es übereinstimmt
                    expandedState[processName] = true; // Klappe den Prozess auf, wenn ein Fenster übereinstimmt
                    //ShowWindow(hGoToButton, SW_HIDE); // Hat funktioniert, aber braucht es nicht
                    foundAnything = true;
                }
                else
                {
                    //ShellExecute(NULL, L"open", L"search-ms:", NULL, NULL, SW_SHOWNORMAL);
                }
                if (window.checked)
                    expandedState[processName] = true;
            }
        }
    }
    if (searchString != std::wstring(L"\u2315 Search Name (CTRL-F)")  && lowerSearchString.length() >= 1 && !foundAnything)
    {
        ShowWindow(hGoToButton, SW_SHOW);
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

void simulateWindowsKeyPress() {
    ////////std::cout << "simulateWindowsKeyPress" << std::endl;
    INPUT inputs[2] = {};

    inputs[0].type = INPUT_KEYBOARD;
    inputs[0].ki.wVk = VK_LWIN;

    inputs[1].type = INPUT_KEYBOARD;
    inputs[1].ki.wVk = VK_LWIN;
    inputs[1].ki.dwFlags = KEYEVENTF_KEYUP;

    SendInput(2, inputs, sizeof(INPUT));
}

void simulateTextInput(const std::wstring& text) {
    ////////std::cout << "simulateTextInput" << std::endl;
    INPUT* inputs = new INPUT[text.length() * 2];

    for (size_t i = 0; i < text.length(); ++i) {
        inputs[i * 2].type = INPUT_KEYBOARD;
        inputs[i * 2].ki.wVk = 0;
        inputs[i * 2].ki.wScan = text[i];
        inputs[i * 2].ki.dwFlags = KEYEVENTF_UNICODE;

        inputs[i * 2 + 1].type = INPUT_KEYBOARD;
        inputs[i * 2 + 1].ki.wVk = 0;
        inputs[i * 2 + 1].ki.wScan = text[i];
        inputs[i * 2 + 1].ki.dwFlags = KEYEVENTF_UNICODE | KEYEVENTF_KEYUP;
    }

    SendInput(text.length() * 2, inputs, sizeof(INPUT));

    delete[] inputs;
}

void SearchAndCheckErase(HWND hwnd) {
    ////////std::cout << "SearchAndCheckErase" << std::endl;
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


void CreateCustomMenu(HWND parentHwnd, POINT pt) {
    auto start = std::chrono::high_resolution_clock::now();

    WNDCLASS wc = {};
    wc.lpfnWndProc = CustomMenuProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = L"CustomMenuClass";
    RegisterClass(&wc);

    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "RegisterClass: " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << "us\n";
    start = std::chrono::high_resolution_clock::now();

    RECT screenRect;
    GetWindowRect(GetDesktopWindow(), &screenRect);

    int width = 400;
    int height = 30 * processNames.size() + getTaskbarHeight(parentHwnd) * 2 + 30;

    if (pt.x + 30 + width > screenRect.right)
        pt.x = screenRect.right - width - 30;
    if (pt.y + height > screenRect.bottom)
        pt.y = screenRect.bottom - height;

    end = std::chrono::high_resolution_clock::now();
    std::cout << "Calculate dimensions: " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << "us\n";
    start = std::chrono::high_resolution_clock::now();

    HWND hwnd = CreateWindowEx(
        WS_EX_TOOLWINDOW | WS_EX_TOPMOST | WS_EX_NOACTIVATE,
        L"CustomMenuClass",
        NULL,
        WS_POPUP,
        pt.x, pt.y, width, height,
        parentHwnd,
        NULL,
        GetModuleHandle(NULL),
        NULL
    );

    end = std::chrono::high_resolution_clock::now();
    std::cout << "CreateWindowEx: " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << "us\n";
    start = std::chrono::high_resolution_clock::now();

    // Set WS_EX_COMPOSITED style for double buffering
    SetWindowLongPtr(hwnd, GWL_EXSTYLE, GetWindowLongPtr(hwnd, GWL_EXSTYLE) | WS_EX_COMPOSITED);

    SetWindowPos(hwnd, HWND_TOPMOST, pt.x, pt.y, width + 10, height, SWP_SHOWWINDOW);

    end = std::chrono::high_resolution_clock::now();
    std::cout << "SetWindowLongPtr and SetWindowPos: " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << "us\n";

    // Perform additional setup in a separate thread
    std::thread([hwnd, width, height]() {
        auto start = std::chrono::high_resolution_clock::now();

        // Abgerundete Ecken hinzufügen
        HRGN hRgn = CreateRoundRectRgn(0, 0, width, height, 20, 20); // 20 ist der Radius der abgerundeten Ecken
        SetWindowRgn(hwnd, hRgn, TRUE);

        auto end = std::chrono::high_resolution_clock::now();
        std::cout << "CreateRoundRectRgn and SetWindowRgn: " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << "us\n";
        start = std::chrono::high_resolution_clock::now();

        // Schwarzen Rahmen hinzufügen
        SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 255, LWA_COLORKEY);

        end = std::chrono::high_resolution_clock::now();
        std::cout << "SetLayeredWindowAttributes: " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << "us\n";
        start = std::chrono::high_resolution_clock::now();

        // Schatten hinzufügen
        MARGINS margins = {3, 3, 3, 3}; // Schattenbreite auf 10 Pixel einstellen
        DwmExtendFrameIntoClientArea(hwnd, &margins);

        end = std::chrono::high_resolution_clock::now();
        std::cout << "DwmExtendFrameIntoClientArea: " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << "us\n";
        start = std::chrono::high_resolution_clock::now();

        // Blur-Effekt hinzufügen
        DWM_BLURBEHIND bb = {};
        bb.dwFlags = DWM_BB_ENABLE | DWM_BB_BLURREGION;
        bb.fEnable = TRUE;
        bb.hRgnBlur = hRgn;
        DwmEnableBlurBehindWindow(hwnd, &bb);

        end = std::chrono::high_resolution_clock::now();
        std::cout << "DwmEnableBlurBehindWindow: " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << "us\n";

        SetTimer(hwnd, TRAY_CONTEXTMENU_TIMER_ID, TRAY_CONTEXTMENU_TIMER_ID_time, NULL);

        end = std::chrono::high_resolution_clock::now();
        std::cout << "SetTimer: " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << "us\n";

        // Only set foreground and focus if necessary
        if (GetForegroundWindow() != hwnd) {
            SetForegroundWindow(hwnd);
            SetFocus(hwnd);
        }

        end = std::chrono::high_resolution_clock::now();
        std::cout << "SetForegroundWindow and SetFocus: " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << "us\n";
    }).detach();
}

LRESULT CALLBACK CustomMenuProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static std::vector<HWND> images;

    switch (uMsg)
    {
        case WM_CREATE:
        {
            HFONT hFont = CreateFont(
                -MulDiv(8, GetDeviceCaps(GetDC(hwnd), LOGPIXELSY), 74),
                0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");

            HINSTANCE hInstance = (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE);
            static HICON hIconMinimize = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(IDI_ICON_MINIMIZE), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR | LR_LOADTRANSPARENT);
            static HICON hIconArrange = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(IDI_ICON_ARRANGE), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR | LR_LOADTRANSPARENT);
            static HICON hIconClose = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(IDI_ICON_CLOSE), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR | LR_LOADTRANSPARENT);

            int yPos = 10;
            int imageID = ID_IMAGE_START;
            int width  = 30;
            int height = 30; 
            int startx = 280;        
            for (const auto& processName : processNames)
            {
                HWND hImage1 = CreateWindow(
                    L"STATIC",
                    NULL,
                    WS_VISIBLE | WS_CHILD | SS_ICON | SS_NOTIFY,
                    startx,
                    yPos,
                    width,
                    height,
                    hwnd,
                    (HMENU)MAKEINTRESOURCE(imageID++),
                    (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
                    NULL);

                HWND hImage2 = CreateWindow(
                    L"STATIC",
                    NULL,
                    WS_VISIBLE | WS_CHILD | SS_ICON | SS_NOTIFY,
                    startx + width + 10,
                    yPos,
                    width,
                    height,
                    hwnd,
                    (HMENU)MAKEINTRESOURCE(imageID++),
                    (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
                    NULL);

                HWND hImage3 = CreateWindow(
                    L"STATIC",
                    NULL,
                    WS_VISIBLE | WS_CHILD | SS_ICON | SS_NOTIFY,
                    startx + 2 * width + 10 + 10,
                    yPos,
                    width,
                    height,
                    hwnd,
                    (HMENU)MAKEINTRESOURCE(imageID++),
                    (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
                    NULL);

                SendMessage(hImage1, STM_SETIMAGE, IMAGE_ICON, (LPARAM)hIconMinimize);
                SendMessage(hImage2, STM_SETIMAGE, IMAGE_ICON, (LPARAM)hIconArrange);
                SendMessage(hImage3, STM_SETIMAGE, IMAGE_ICON, (LPARAM)hIconClose);

                images.push_back(hImage1);
                images.push_back(hImage2);
                images.push_back(hImage3);
                yPos += 30;
            }

        /*HWND hQuitMenuImage = CreateWindow(
            L"STATIC",
            L"Quit Menu",
            WS_VISIBLE | WS_CHILD | SS_CENTER | SS_NOTIFY | WS_BORDER | SS_CENTERIMAGE,
            10,
            yPos + 30,
            100,
            30,
            hwnd,
            (HMENU)ID_QUITMENU_IMAGE,
            (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
            NULL);

            images.push_back(hQuitMenuImage);*/

            width  = 32;
            height = 32;
            HWND hExitButton = CreateWindow(
                L"BUTTON",
                L"Exit",
                WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_ICON | BS_FLAT,
                348,
                yPos + 30,
                width,
                height,
                hwnd,
                (HMENU)ID_EXIT_IMAGE,
                (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
                NULL
            );

            HICON hIconExit = (HICON)LoadImage(GetModuleHandle(L"SHELL32.DLL"), MAKEINTRESOURCE(16805), IMAGE_ICON, 32, 32, LR_DEFAULTCOLOR);
            if (hIconExit == NULL) {
                OutputDebugString(L"Failed to load icon\n");
            } else {
                SendMessage(hExitButton, BM_SETIMAGE, IMAGE_ICON, (LPARAM)hIconExit);
            }

            //images.push_back(hExitButton);

            // Abgerundete Ecken hinzufügen
            HRGN hRgn = CreateRoundRectRgn(0, 0, width, height, 20, 20); // 20 ist der Radius der abgerundeten Ecken
            SetWindowRgn(hExitButton, hRgn, TRUE);

            // Hintergrundfarbe und Textfarbe anpassen
            HDC hdc = GetDC(hExitButton);
            SetBkColor(hdc, RGB(0, 0, 0)); // Schwarzer Hintergrund
            SetTextColor(hdc, RGB(255, 255, 255)); // Weißer Text
            ReleaseDC(hExitButton, hdc);
          
            break;
        }

        case WM_ERASEBKGND:
            return 1;
            
        case WM_MOUSEMOVE:
        {
            POINT pt;
            GetCursorPos(&pt);
            ScreenToClient(hwnd, &pt);

            int newLine = pt.y / 30;
            if (newLine != currentLine)
            {
                currentLine = newLine;
                RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
            }
            break;
        }

        case WM_SETCURSOR:
        {
            SetCursor(LoadCursor(NULL, IDC_ARROW));
            HWND hWndCursor = (HWND)wParam;
            if (std::find(images.begin(), images.end(), hWndCursor) != images.end()) {
                // Der Mauszeiger befindet sich über einem der STATIC-Elemente
                POINT pt;
                GetCursorPos(&pt);
                ScreenToClient(hwnd, &pt);

                RECT rect;
                GetWindowRect(hWndCursor, &rect);
                MapWindowPoints(HWND_DESKTOP, hwnd, (LPPOINT)&rect, 2);

                if (PtInRect(&rect, pt)) {
                    if (hWndCursor != lastHoveredImage) {
                        // Setze das vorherige Icon zurück
                        if (lastHoveredImage != NULL) {
                            SendMessage(lastHoveredImage, STM_SETIMAGE, IMAGE_ICON, (LPARAM)lastOriginalIcon);
                        }

                        // Invertiere das neue Icon
                        HICON hIcon = (HICON)SendMessage(hWndCursor, STM_GETIMAGE, IMAGE_ICON, 0);
                        lastOriginalIcon = hIcon; // Speichere das Original-Icon
                        HICON hIconInverted = InvertIconColors(hIcon);
                        SendMessage(hWndCursor, STM_SETIMAGE, IMAGE_ICON, (LPARAM)hIconInverted);
                        lastHoveredImage = hWndCursor;
                        OutputDebugString(L"Icon inverted\n");
                    }
                }
                return TRUE; // Verhindert die Standard-Cursor-Verarbeitung
            } else {
                // Setze das Icon zurück, wenn der Mauszeiger das STATIC-Element verlässt
                if (lastHoveredImage != NULL) {
                    SendMessage(lastHoveredImage, STM_SETIMAGE, IMAGE_ICON, (LPARAM)lastOriginalIcon);
                    lastHoveredImage = NULL;
                    lastOriginalIcon = NULL;
                    OutputDebugString(L"Icon reset\n");
                }
            }
            return TRUE;
        }

        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            RECT rect;
            GetClientRect(hwnd, &rect);

            HDC memDC = CreateCompatibleDC(hdc);
            HBITMAP hbmMem = CreateCompatibleBitmap(hdc, rect.right, rect.bottom);
            HBITMAP hbmOld = (HBITMAP)SelectObject(memDC, hbmMem);

            // Hintergrund füllen
            HBRUSH hBrush = CreateSolidBrush(GetSysColor(COLOR_MENU));
            FillRect(memDC, &rect, hBrush);
            DeleteObject(hBrush);

            HFONT hFont = CreateFont(20, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, 
                                    OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, 
                                    DEFAULT_PITCH | FF_SWISS, L"Segoe UI");
            HFONT hOldFont = (HFONT)SelectObject(memDC, hFont);

            int yPos = 10;
            for (int i = 0; i < processNames.size(); ++i)
            {
                auto processName = processNames[i];
                std::wstring processNameW(processName.begin(), processName.end());
                processNameW = trim(processNameW);
                processNameW = capitalizeIfAllCaps(processNameW);

                if (i == currentLine)
                {
                    RECT lineRect = {0, yPos - 5, rect.right, yPos - 5 + 30};
                    FillRect(memDC, &lineRect, (HBRUSH)GetStockObject(LTGRAY_BRUSH));
                }

                auto it = processIcons.find(processName);
                if (it != processIcons.end())
                {
                    DrawIconEx(memDC, 10, yPos, it->second, 20, 20, 0, NULL, DI_NORMAL);
                }

                SetBkMode(memDC, TRANSPARENT);
                TextOut(memDC, 40, yPos, processNameW.c_str(), processNameW.length());

                yPos += 30;
            }

            BitBlt(hdc, 0, 0, rect.right, rect.bottom, memDC, 0, 0, SRCCOPY);

            SelectObject(memDC, hOldFont);
            DeleteObject(hFont);
            SelectObject(memDC, hbmOld);
            DeleteObject(hbmMem);
            DeleteDC(memDC);

            EndPaint(hwnd, &ps);
            break;
        }

        case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            int totalImages = processNames.size() * 3;
            
            if (wmId == ID_EXIT_IMAGE)
            {
                SetWindowPos(hwnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
                int result = MessageBox(NULL, L"Are you sure you want to exit the 'Manage Multiple Open Windows' program?", L"Exit Confirmation", MB_YESNO | MB_ICONQUESTION  | MB_TOPMOST | MB_DEFBUTTON2);
                //int result = ShowMessageBoxAndHandleTimers(NULL, L"Are you sure you want to exit the 'Manage Multiple Open Windows' program?", L"Exit Confirmation", MB_YESNO | MB_ICONQUESTION  | MB_TOPMOST, timers);

                if (result == IDYES)
                {
                    PostQuitMessage(0);
                }
                else
                {
                    SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
                }
            }
            if (wmId == ID_QUITMENU_IMAGE)
            {
                KillTimer(hwnd, TRAY_CONTEXTMENU_TIMER_ID);
                DestroyWindow(hwnd);
            }

            if (wmId >= ID_IMAGE_START && wmId < ID_IMAGE_START + totalImages)
            {
                int index = (wmId - ID_IMAGE_START) / 3;
                int imageNumber = (wmId - ID_IMAGE_START) % 3 + 1;
                if (imageNumber == 1)
                {
                    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
                    int screenHeight = GetSystemMetrics(SM_CYSCREEN);
                    for (auto& entry : processWindowsMap) {
                        for (auto& window : entry.second) {
                            if (window.processName == processNames[index]) {
                                SetWindowPos(window.hwnd, NULL, 0, 0, screenWidth, screenHeight, SWP_NOZORDER);
                                window.arranged = false;
                                ShowWindow(window.hwnd, SW_MINIMIZE);
                                ProcessMessages();
                            }
                        }
                    }              
                }
            if (imageNumber == 2)
            {
                int screenIndex = 0; 
                std::vector<MonitorInfo> monitors;
                EnumDisplayMonitors(NULL, NULL, MonitorEnumProc, (LPARAM)&monitors);
                globalScreenIndexChosen++;
                if (globalScreenIndexChosen > monitors.size()) globalScreenIndexChosen = 1;
                if (monitors.size() > 1) screenIndex = globalScreenIndexChosen - 1;
                RECT screenRect = GetScreenRect(screenIndex);
                int screenWidth = screenRect.right - screenRect.left;
                int screenHeight = screenRect.bottom - screenRect.top - getTaskbarHeight(hwnd);
                //////std::cout << "screenWidth: " << screenWidth << std::endl;

                int numWindows = 0;
                for (auto& entry : processWindowsMap) {
                    for (auto& window : entry.second) {
                        if (window.processName == processNames[index]) {
                            window.arranged = true; 
                            numWindows++;
                        }
                    }
                }
        //////std::cout << "numWindows: " << numWindows << std::endl;
                int cols = static_cast<int>(ceil(sqrt(numWindows))); // Calculate the number of columns
                int rows = (numWindows + cols - 1) / cols; // Calculate the number of rows
        //////std::cout << "start0: " << std::endl;
        //////std::cout << "screenWidth: " << screenWidth << std::endl;
        //////std::cout << "cols: " << cols << std::endl;
        //////std::cout << "screenHeight: " << screenHeight << std::endl;
        //////std::cout << "rows: " << rows << std::endl;
                int windowWidth = (screenWidth) / cols; // Calculate the window width
                int windowHeight = (screenHeight) / rows; // Calculate the window height
        //////std::cout << "start0a: " << std::endl;
                int x = 0, y = 0; // Initialize the X and Y positions
                int maxAdjustedWidth = 0;
                int minAdjustedWidth = screenWidth;
                numWindows = 0;
                //////std::cout << "start: " << std::endl;
                // Zuerst die Breite des breitesten Fensters ermitteln
                for (const auto& entry : processWindowsMap) { // Iterate through all processes
                    //////std::cout << "start1: " << std::endl;
                    for (const auto& window : entry.second) { // Iterate through all windows of a process
                        //////std::cout << "start2: " << std::endl;
                        if (window.processName == processNames[index]) { // Check if the window is selected
                            //////std::cout << "start3: " << std::endl;
                            if (IsIconic(window.hwnd) || IsZoomed(window.hwnd)) ShowWindow(window.hwnd, SW_RESTORE); // Restore the window if minimized or maximized
                            //SetWindowPos(window.hwnd, NULL, 0, 0, screenWidth, screenHeight, SWP_NOZORDER);
                            MoveWindowToScreen(window.hwnd, screenIndex);
                            ProcessMessages(); // Process messages
                            //ShowWindow(window.hwnd, SW_MINIMIZE); // Minimize the window
                            if (IsIconic(window.hwnd) || IsZoomed(window.hwnd)) ShowWindow(window.hwnd, SW_RESTORE); // Restore the window
                            SetForegroundWindow(window.hwnd); // Bring the window to the foreground
                            ProcessMessages(); // Process messages
                            //MoveWindow(window.hwnd, screenRect.left + x, screenRect.top + y, 50, 50, TRUE); // Move and resize the window
                            //ProcessMessages(); // Process messages
                            MoveWindow(window.hwnd, screenRect.left + x, screenRect.top + y, windowWidth - 5, windowHeight, TRUE); // Move and resize the window
                            ProcessMessages(); // Process messages

                            /*x += windowWidth; // Increment the X position
                            if (x >= screenWidth) { // Check if the X position exceeds the screen width
                                x = 0; // Reset the X position
                                y += windowHeight; // Increment the Y position
                            }*/
                            Sleep(100);
                            // Größe des klienten Bereichs ermitteln
                            RECT clientRect;
                            GetClientRect(window.hwnd, &clientRect);
                            int clientWidth = clientRect.right - clientRect.left;
                            int clientHeight = clientRect.bottom - clientRect.top;

                            // Fensterstil und erweiterte Stile ermitteln
                            DWORD style = GetWindowLong(window.hwnd, GWL_STYLE);
                            DWORD exStyle = GetWindowLong(window.hwnd, GWL_EXSTYLE);

                            // Anpassung der Fenstergröße basierend auf dem klienten Bereich
                            RECT adjustedRect = {0, 0, clientWidth, clientHeight};
                            AdjustWindowRectEx(&adjustedRect, style, FALSE, exStyle);

                            // Berechnete Fenstergröße
                            int adjustedWidth = adjustedRect.right - adjustedRect.left;
                            int adjustedHeight = adjustedRect.bottom - adjustedRect.top;

                            // Aktualisiere die maximale Breite
                            if (adjustedWidth > maxAdjustedWidth) {
                                maxAdjustedWidth = adjustedWidth;
                            }
                            // Aktualisiere die minimal Breite
                            if (adjustedWidth < minAdjustedWidth) {
                                minAdjustedWidth = adjustedWidth;
                            }

                            numWindows++; // Increment the counter
                        }
                    }
                }
                //////std::cout << "minAdjustedWidth: " << minAdjustedWidth << std::endl;
                //////std::cout << "cols: " << cols << std::endl;
                // Anzahl der Fenster nebeneinander und die Anzahl der Fensterreihen berechnen
                cols = screenWidth / std::max(maxAdjustedWidth, screenWidth/cols);
                //////std::cout << "cols: " << cols << std::endl;                
                rows = (numWindows + cols - 1) / cols;
                windowWidth = screenWidth / cols; // Calculate the window width
                windowHeight = (screenHeight) / rows; // Calculate the window height

                // Fenster anordnen
                x = 0;
                y = 0;
                for (const auto& entry : processWindowsMap) { // Iterate through all processes
                    for (const auto& window : entry.second) { // Iterate through all windows of a process
                        if (window.processName == processNames[index]) { // Check if the window is selected
                            MoveWindow(window.hwnd, screenRect.left + x, screenRect.top + y, windowWidth, windowHeight, TRUE); // Move and resize the window
                            ProcessMessages(); // Process messages

                            x += windowWidth; // Increment the X position
                            if (x >= screenWidth - 20) { // Check if the X position exceeds the screen width
                                x = 0; // Reset the X position
                                y += windowHeight; // Increment the Y position
                            }
                        }
                    }
                }

                /*for (auto& entry : processWindowsMap) { // Iterate through all processes
                    for (auto& window : entry.second) { // Iterate through all windows of a process
                        window.checked = false; // Deselect the window
                    }
                }
                for (auto& state : checkboxState) { // Iterate through all checkbox states
                    state.second = false; // Deselect the checkbox
                }
                for (auto& state : expandedState) { // Iterate through all expanded states
                    state.second = false; // Collapse the expanded state
                }*/
            }

            if (imageNumber == 3)
            {
                HWND parentHwnd = GetParent(hwnd);
                if (ConfirmClose(parentHwnd)) {
                    ProcessMessages();
                    for (const auto& entry : processWindowsMap) {
                        for (const auto& window : entry.second) {
                            if (window.processName == processNames[index]) {
                                SetForegroundWindow(window.hwnd);

                                INPUT inputs[4] = {};

                                inputs[0].type = INPUT_KEYBOARD;
                                inputs[0].ki.wVk = VK_CONTROL;

                                inputs[1].type = INPUT_KEYBOARD;
                                inputs[1].ki.wVk = 'S';

                                inputs[2].type = INPUT_KEYBOARD;
                                inputs[2].ki.wVk = 'S';
                                inputs[2].ki.dwFlags = KEYEVENTF_KEYUP;

                                inputs[3].type = INPUT_KEYBOARD;
                                inputs[3].ki.wVk = VK_CONTROL;
                                inputs[3].ki.dwFlags = KEYEVENTF_KEYUP;

                                SendInput(ARRAYSIZE(inputs), inputs, sizeof(INPUT));
                                PostMessage(window.hwnd, WM_CLOSE, 0, 0);

                                ProcessMessages();
                            }
                        }
                    }
                }
            }
        }
        break;
        }

        case WM_TIMER:
        {
            POINT pt;
            GetCursorPos(&pt);
            RECT rect;
            GetWindowRect(hwnd, &rect);
            if (!PtInRect(&rect, pt))
            {
                KillTimer(hwnd, TRAY_CONTEXTMENU_TIMER_ID);
                DestroyWindow(hwnd);
            }
            break;
        }
        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

int GetWindowWidth(HWND hwnd) {
    RECT windowRect;
    if (GetWindowRect(hwnd, &windowRect)) {
        int width = windowRect.right - windowRect.left;
        return width;
    }
    return -1; // Fehlerfall
}

void UpdateControlPositions(HWND hwnd) {
    RECT clientRect;
    GetClientRect(hwnd, &clientRect);

    int searchBoxWidth = 200; // Breite des Suchfelds
    int searchBoxHeight = 20; // Höhe des Suchfelds
    int buttonWidth = 20; // Breite der Buttons
    int buttonHeight = searchBoxHeight; // Höhe der Buttons
    int padding = 5; // Abstand zwischen den Steuerelementen

    int searchBoxX = clientRect.right - searchBoxWidth - 2 * buttonWidth - 3 * padding;
    int searchBoxY = 5; // Position Y des Suchfelds

    SetWindowPos(hSearchBox, NULL, searchBoxX, searchBoxY, searchBoxWidth, searchBoxHeight, SWP_NOZORDER);
    SetWindowPos(hEraseButton, NULL, searchBoxX + searchBoxWidth + padding, searchBoxY, buttonWidth, buttonHeight, SWP_NOZORDER);
    SetWindowPos(hGoToButton, NULL, searchBoxX + searchBoxWidth + buttonWidth + 2 * padding, searchBoxY, buttonWidth, buttonHeight, SWP_NOZORDER);
}

// Window procedure
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    ////////std::cout << "WindowProc" << std::endl;
    static int scrollPos = 0; // Static variable to store the scroll position
    int id; // Variable to store the ID
    static HIMAGELIST hImageList;
    static HICON hIcon1;
    static HMENU hMenu = NULL;

    ////std::cout << "uMsg:" << uMsg << std::endl;

    switch (uMsg) { // Check the messages
        case WM_CREATE: {
            ////////std::cout << "WM_CREATE" << std::endl;
            CreateTrayIcon(hwnd);
            if (!initialized) {
                UpdateWindowList(hwnd);
                initialized = true;
                MinimizeToTray(hwnd);
            }
            RECT clientRect;
            GetClientRect(hwnd, &clientRect);
            int searchBoxWidth = 200; // Breite des Suchfelds
            int searchBoxHeight = 20; // Höhe des Suchfelds
            int searchBoxX = clientRect.right - searchBoxWidth - getTaskbarHeight(hwnd); // Position X des Suchfelds
            int searchBoxY = 5; // Position Y des Suchfelds

            // Erstellen Sie das Suchfeld
            hSearchBox = CreateWindowEx(0, TEXT("EDIT"), NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL, 
                                        searchBoxX, searchBoxY, searchBoxWidth, searchBoxHeight, hwnd, (HMENU)IDC_SEARCHBOX, 
                                        ((LPCREATESTRUCT)lParam)->hInstance, NULL);
            SendMessage(hSearchBox, EM_SETLIMITTEXT, 32, 0); // Begrenze die Eingabe auf 32 Zeichen
            SetEditPlaceholder(hSearchBox, L"\u2315 Search Name (CTRL-F)"); // Setze den Hint

            // Erstellen Sie den "Erase"-Button
            hEraseButton = CreateWindowEx(0, TEXT("BUTTON"), L"\u2716", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 
                                        searchBoxX + searchBoxWidth, searchBoxY, 20, searchBoxHeight, hwnd, (HMENU)IDC_ERASEBUTTON, 
                                        ((LPCREATESTRUCT)lParam)->hInstance, NULL);

           // Erstellen Sie den "Go to"-Button für Suchstrings, die nicht gefunden werden
            hGoToButton = CreateWindowEx(0, TEXT("BUTTON"), L"\u21AA", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 
                                        searchBoxX + searchBoxWidth + 20, searchBoxY, 20, searchBoxHeight, hwnd, (HMENU)IDC_GOTOBUTTON, 
                                        ((LPCREATESTRUCT)lParam)->hInstance, NULL);


            // Erstellen Sie die Schriftart "Segoe UI"
            HFONT hFont = CreateFont(
                16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("Segoe UI")
            );

            // Wenden Sie die Schriftart auf die Steuerelemente an
            SendMessage(hSearchBox, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hEraseButton, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hGoToButton, WM_SETFONT, (WPARAM)hFont, TRUE);            

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
        SetTimer(hwnd, BLINKING_TIMER_ID, 250, NULL);
        //InvalidateWindow(hwnd);
        break;

        case WM_KEYDOWN: {
            ////std::cout << "WM_KEYDOWN" << std::endl;
            switch (wParam) {
                case VK_UP:
                    if (highlightedRow > 0) {
                        highlightedRow--;
                        highlightedWindowRow = -1; // Reset window row highlight
                        InvalidateRect(hwnd, NULL, TRUE);
                    }
                break;
                case VK_DOWN:
                    if (highlightedRow < processNames.size() - 1) {
                        highlightedRow++;
                        highlightedWindowRow = -1; // Reset window row highlight
                        InvalidateRect(hwnd, NULL, TRUE);
                    }
                break;
                case VK_RETURN:
                {
                    int yPos = 0 - scrollPos;
                    HFONT hFont = CreateFont(24, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, TEXT("Segoe UI")); // Create a font
                    HDC hdc = GetDC(hwnd); // Retrieve the device context
                    HFONT hOldFont = (HFONT)SelectObject(hdc, hFont); // Select the new font and save the old font
                    for (size_t i = 0; i < processNames.size(); ++i) {
                        const auto& processName = processNames[i];
                        RECT rect = { 30, yPos, textWidth, yPos + 30 };
                        if (i == highlightedRow) {
                            checkboxState[processName] = !checkboxState[processName];
                            for (auto& window : processWindowsMap[processName]) {
                                window.checked = checkboxState[processName];
                            }
                            InvalidateRect(hwnd, NULL, TRUE);
                        }
                        rect = { 0, yPos, 30, yPos + 30 };
                        if (i == highlightedRow) {
                            expandedState[processName] = !expandedState[processName];
                            for (size_t j = 0; j < processWindowsMap[processName].size(); ++j) { // Iterate through all windows of the process
                                auto& window = processWindowsMap[processName][j]; // Retrieve the current window
                                window.visible = true;
                                yPos += 30;
                            }
                            InvalidateWindow(hwnd);
                        }
                        yPos += 30; // Increase the y-position

                        if (expandedState[processName]) { // Check if the process is expanded
                            for (size_t j = 0; j < processWindowsMap[processName].size(); ++j) { // Iterate through all windows of the process
                                auto& window = processWindowsMap[processName][j]; // Retrieve the current window
                                RECT windowRect = { 50, yPos, textWidth, yPos + 30 }; // Define a rectangle for the window
                                if (j == highlightedWindowRow && window.visible) { // Check if the cursor is in the rectangle
                                    window.checked = !window.checked; // Toggle the checkbox state for the window
                                    InvalidateRect(hwnd, NULL, TRUE); // Invalidate and redraw the window
                                }
                                if (window.visible || window.checked) 
                                    yPos += 30; // Increase the y-position 
                            }
                        }
                    }
                    SelectObject(hdc, hOldFont); // Restore the old font
                    DeleteObject(hFont); // Delete the new font
                    ReleaseDC(hwnd, hdc); // Release the device context
                }
                break;
            }
            if ((wParam == 'F') && (GetKeyState(VK_CONTROL) & 0x8000)) {
                // Ctrl+F was pressed
                SetFocus(hSearchBox);
                SendMessage(hSearchBox, EM_SETSEL, 0, -1); // Select all text in the search box
                return 0;
            /*} else if (wParam == VK_ESCAPE) {
                // ESC key was pressed
                wParam = IDC_ERASEBUTTON;*/
            }
            if ((wParam == 'X') && (GetKeyState(VK_CONTROL) & 0x8000)) {
                // Handle the hotkey (CTRL+X)
                SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(ID_CTRL_X, 0), 0);
                /*SetWindowPos(hwnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
                //int result = MessageBox(NULL, L"Are you sure you want to exit the 'Manage Multiple Open Windows' program?", L"Exit Confirmation", MB_YESNO | MB_ICONQUESTION  | MB_TOPMOST);
                int result = ShowMessageBoxAndHandleTimers(NULL, L"Are you sure you want to exit the 'Manage Multiple Open Windows' program?", L"Exit Confirmation", MB_YESNO | MB_ICONQUESTION  | MB_TOPMOST, timers);

                if (result == IDYES)
                {
                    PostQuitMessage(0);
                }
                else
                {
                    SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
                }*/
            }
            if ((wParam == 'W') && (GetKeyState(VK_CONTROL) & 0x8000)) {
                // Handle the hotkey (CTRL+W) (close to tray)
                SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(ID_CTRL_W, 0), 0);
            }
            if ((wParam == 'I') && (GetKeyState(VK_CONTROL) & 0x8000)) {
                // Handle the hotkey (CTRL+I) (minimize windows)
                SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(ID_CTRL_I, 0), 0);
            }
            if ((wParam == 'A') && (GetKeyState(VK_CONTROL) & 0x8000)) {
                // Handle the hotkey (CTRL+A) (maximize windows)
                SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(ID_CTRL_A, 0), 0);
            }
            if ((wParam == 'R') && (GetKeyState(VK_CONTROL) & 0x8000)) {
                // Handle the hotkey (CTRL+R) (restore windows)
                SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(ID_CTRL_R, 0), 0);
            }      
            if ((wParam == 'N') && (GetKeyState(VK_CONTROL) & 0x8000)) {
                // Handle the hotkey (CTRL+N) (arrange windows)
                SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(ID_CTRL_N, 0), 0);
            }                                    
            if ((wParam == 'M') && (GetKeyState(VK_CONTROL) & 0x8000)) {
                // Handle the hotkey (CTRL+M) (move windows)
                SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(ID_CTRL_M, 0), 0);
            }                                    
        }
        break;

        case WM_TIMER: {
            ////////std::cout << "WM_KEYDOWN" << std::endl;
            ////std::cout << "WM_TIMER" << std::endl;
            if (wParam == BLINKING_TIMER_ID) {
                //DestroyWindow(hwnd);
                blinkState = !blinkState; // Toggle the blink state
                InvalidateRect(hwnd, NULL, TRUE); // Fenster neu zeichnen
            } /*else if (wParam == TOOLTIP_TIMER_ID) {
            KillTimer(hwnd, TOOLTIP_TIMER_ID);
            HWND hwndTT = FindWindowEx(NULL, NULL, TOOLTIPS_CLASS, NULL);
            if (hwndTT) {
                SendMessage(hwndTT, TTM_TRACKACTIVATE, FALSE, 0);
                DestroyWindow(hwndTT);
            }*/
        }
        break;

        case WM_DRAWITEM: { // Message when drawing an item
            ////////std::cout << "WM_DRAWITEM" << std::endl;
            /*LPDRAWITEMSTRUCT lpDrawItem = (LPDRAWITEMSTRUCT)lParam; // Draw item structure
            if (lpDrawItem->CtlID >= ID_PROCESSNAMESTART && lpDrawItem->CtlID < ID_PROCESSNAMESTART + processNames.size()) { // Check the ID
                std::wstring processName = processNames[lpDrawItem->CtlID - ID_PROCESSNAMESTART]; // Retrieve the process name
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
            ////////std::cout << "WM_UPDATE_LIST" << std::endl;
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
            ////////std::cout << "WM_VSCROLL" << std::endl;
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
            ////////std::cout << "WM_ERASEBKGND" << std::endl;
            return 1; // Hintergrund nicht löschen

        case WM_COMMAND: { // Message when a command is executed (e.g., button click)
            ////////std::cout << "WM_COMMAND" << std::endl;
            id = LOWORD(wParam); // Extract the command ID from wParam
            //std::wstring message = L"" + std::to_wstring(id);
            //MessageBoxW(hwnd, message.c_str(), L"Debug Info", MB_OK);
            if (id == IDC_GOTOBUTTON) {
                wchar_t searchString[256];
                GetWindowTextW(hSearchBox, searchString, 256);
                simulateWindowsKeyPress();
                Sleep(1000);
                simulateTextInput(searchString);
                SetWindowText(hSearchBox, L"");
                ShowWindow(hEraseButton, SW_SHOW); 
                SearchAndCheckErase(hwnd);
                //ShowWindow(hGoToButton, SW_HIDE); // Hat funktioniert, aber braucht es nicht
                SendMessage(hwnd, WM_VSCROLL, MAKEWPARAM(SB_THUMBPOSITION, 1), 0);                 // Setzen Sie die Scrollposition auf 1                
                InvalidateRect(hEraseButton, NULL, TRUE);
                UpdateWindow(hEraseButton);
                if (GetWindowTextLength(hSearchBox) == 0) {
                    SetWindowText(hSearchBox, L"\u2315 Search Name (CTRL-F)"); // Setzen Sie hier Ihren Such-Hint-Text ein
                }
                SetFocus(hwnd);
            }
            if (id == IDC_ERASEBUTTON) {
                SetWindowText(hSearchBox, L"");
                ShowWindow(hEraseButton, SW_SHOW); 
                SearchAndCheckErase(hwnd);
                //ShowWindow(hGoToButton, SW_HIDE); // Hat funktioniert, aber braucht es nicht
                SendMessage(hwnd, WM_VSCROLL, MAKEWPARAM(SB_THUMBPOSITION, 1), 0);                 // Setzen Sie die Scrollposition auf 1                
                InvalidateRect(hEraseButton, NULL, TRUE);
                UpdateWindow(hEraseButton);
                if (GetWindowTextLength(hSearchBox) == 0) {
                    SetWindowText(hSearchBox, L"\u2315 Search Name (CTRL-F)"); // Setzen Sie hier Ihren Such-Hint-Text ein
                }
                SetFocus(hwnd);
            }
            if (HIWORD(wParam) == EN_CHANGE && LOWORD(wParam) == IDC_SEARCHBOX) {
                wchar_t searchString[256];
                GetWindowTextW(hSearchBox, searchString, 256);
                //ShowWindow(hGoToButton, SW_HIDE); // Hat funktioniert, aber braucht es nicht
                if (IsValidInput(searchString)) {
                    SearchAndCheck(searchString, hwnd);
                } else {
                    //ShowTooltip(hSearchBox, L"Invalid input. Please use only allowed characters.");                    SetFocus(hSearchBox); // Setzen Sie den Fokus wieder auf das Suchfeld
                }
                SendMessage(hwnd, WM_VSCROLL, MAKEWPARAM(SB_THUMBPOSITION, 1), 0);                 // Setzen Sie die Scrollposition auf 1
                //InvalidateWindow(hwnd);
            }
            if (HIWORD(wParam) == EN_UPDATE && LOWORD(wParam) == IDC_SEARCHBOX) {
                if (GetKeyState(VK_RETURN) & 0x8000) {
                    wchar_t searchString[256];
                    GetWindowTextW(hSearchBox, searchString, 256);
                    //ShowWindow(hGoToButton, SW_HIDE); // Hat funktioniert, aber braucht es nicht
                    if (IsValidInput(searchString)) {
                        SearchAndCheck(searchString, hwnd);
                    } else {
                        //ShowTooltip(hSearchBox, L"Invalid input. Please use only allowed characters.");                        SetFocus(hSearchBox); // Setzen Sie den Fokus wieder auf das Suchfeld
                    }
                    SendMessage(hwnd, WM_VSCROLL, MAKEWPARAM(SB_THUMBPOSITION, 1), 0);                 // Setzen Sie die Scrollposition auf 1                    
                    //InvalidateWindow(hwnd);
                }
            }
            // Check if the command ID is one of the specified IDs
            if (id == ID_MINIMIZE || id == ID_MAXIMIZE || id == ID_RESTORE || id == ID_CLOSE || 
                (id >= ID_ARRANGE && id <= ID_ARRANGE + screenCount) || 
                (id >= ID_MOVE_TO_SCREEN_BASE && id <= ID_MOVE_TO_SCREEN_BASE + screenCount) || id == ID_SAVEANDCLOSE) {

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
                    int result = ShowMessageBoxAndHandleTimers(hwnd, L"Please select one or more window(s).", L"Warning", MB_OK | MB_ICONWARNING, timers);
                    break;
                }
            }

            if (id == ID_MINIMIZE or LOWORD(wParam) == ID_CTRL_I) { // Check if the ID is 2000 (Minimize)
                MinimizeToTray(hwnd); // Minimize the window to the tray
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
            } else if (id == ID_MAXIMIZE || LOWORD(wParam) == ID_CTRL_A) { // Check if the ID is 2000 (Minimize)
                MinimizeToTray(hwnd); // Minimize the window to the tray
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
            } else if (id == ID_RESTORE || LOWORD(wParam) == ID_CTRL_R) { // Check if the ID is 2001 (Restore)
                MinimizeToTray(hwnd); // Minimize the window to the tray
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
            } else if (id == ID_CLOSE) { // Check if the ID is 2002 (Close)
                if (ConfirmClose(hwnd)) { // Display confirmation dialog
                    MinimizeToTray(hwnd); // Minimize the window to the tray
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
                }
                } else if (id == ID_EXIT_BUTTON || LOWORD(wParam) == ID_CTRL_X) {
                    SetWindowPos(hwnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
                    //int result = MessageBox(NULL, L"Are you sure you want to exit the 'Manage Multiple Open Windows' program?", L"Exit Confirmation", MB_YESNO | MB_ICONQUESTION  | MB_TOPMOST);
                    int result = ShowMessageBoxAndHandleTimers(NULL, L"Are you sure you want to exit the 'Manage Multiple Open Windows' program?", L"Exit Confirmation", MB_YESNO | MB_ICONQUESTION  | MB_TOPMOST, timers);

                    if (result == IDYES)
                    {
                        PostQuitMessage(0);
                    }
                    else
                    {
                        SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
                    }
                } else if (id == ID_CLOSE_PROGRAMM || LOWORD(wParam) == ID_CTRL_W) {
                    MinimizeToTray(hwnd); // Minimize the window to the tray
                } else if (id == ID_SAVEANDCLOSE) { // Check if the ID is SAVEANDCLOSE
                if (ConfirmClose(hwnd)) { // Display confirmation dialog
                    ProcessMessages(); // Process messages
                    MinimizeToTray(hwnd); // Minimize the window to the tray
                    for (const auto& entry : processWindowsMap) { // Iterate through all processes
                        for (const auto& window : entry.second) { // Iterate through all windows of a process
                            if (window.checked) { // Check if the window is selected
                                // Fenster in den Vordergrund bringen
                                SetForegroundWindow(window.hwnd);

                                // Tastenkombination Ctrl+S senden
                                INPUT inputs[4] = {};

                                // Ctrl-Taste drücken
                                inputs[0].type = INPUT_KEYBOARD;
                                inputs[0].ki.wVk = VK_CONTROL;

                                // S-Taste drücken
                                inputs[1].type = INPUT_KEYBOARD;
                                inputs[1].ki.wVk = 'S';

                                // S-Taste loslassen
                                inputs[2].type = INPUT_KEYBOARD;
                                inputs[2].ki.wVk = 'S';
                                inputs[2].ki.dwFlags = KEYEVENTF_KEYUP;

                                // Ctrl-Taste loslassen
                                inputs[3].type = INPUT_KEYBOARD;
                                inputs[3].ki.wVk = VK_CONTROL;
                                inputs[3].ki.dwFlags = KEYEVENTF_KEYUP;

                                // Eingaben senden
                                SendInput(ARRAYSIZE(inputs), inputs, sizeof(INPUT));
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
                }
            } else if ((id >= ID_ARRANGE && id <= ID_ARRANGE + screenCount) || LOWORD(wParam) == ID_CTRL_N) {//else if (id == 2003) { Check if the ID is 2003 (Arrange)
                ////std::cout << "arrange" << std::endl;
                MinimizeToTray(hwnd); // Minimize the window to the tray
                //MessageBoxW(hwnd, L"aha2", L"Debug Info", MB_OK);
                ClearTemporaryTiles();
                int screenIndex = 0;
                if (id != ID_CTRL_N) 
                    screenIndex = (id - ID_ARRANGE);
                else
                {
                    std::vector<MonitorInfo> monitors;
                    EnumDisplayMonitors(NULL, NULL, MonitorEnumProc, (LPARAM)&monitors);
                    globalScreenIndexChosen++;
                    if (globalScreenIndexChosen > monitors.size()) globalScreenIndexChosen = 1;
                    if (monitors.size() > 1) screenIndex = globalScreenIndexChosen - 1;
                }
                //std::cout << id << " " << screenIndex << std::endl;

                RECT screenRect = GetScreenRect(screenIndex);
                //RECT rect; // Declaration of a RECT structure
                //GetClientRect(hwnd, &rect); // Retrieve the client rectangles of the window
                //int screenWidth = GetSystemMetrics(SM_CXSCREEN); // Retrieve the screen width
                int screenWidth = screenRect.right - screenRect.left;
                int screenHeight = screenRect.bottom - screenRect.top - getTaskbarHeight(hwnd);
                //int screenHeight = GetSystemMetrics(SM_CYSCREEN); // Retrieve the screen height
                ////////std::cout << "screenWidth : " << screenWidth << std::endl;

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

                int windowWidth = (screenWidth) / cols; // Calculate the window width
                int windowHeight = (screenHeight) / rows; // Calculate the window height

                int x = 0, y = 0; // Initialize the X and Y positions
                int maxAdjustedWidth = 0;
                int minAdjustedWidth = screenWidth;
                numWindows = 0;
//std::cout << "arrange1" << std::endl;
                // Zuerst die Breite des breitesten Fensters ermitteln
                for (const auto& entry : processWindowsMap) { // Iterate through all processes
                    for (const auto& window : entry.second) { // Iterate through all windows of a process
                        if (window.checked) { // Check if the window is selected
                            if (IsIconic(window.hwnd) || IsZoomed(window.hwnd)) ShowWindow(window.hwnd, SW_RESTORE); // Restore the window if minimized or maximized
                            //SetWindowPos(window.hwnd, NULL, 0, 0, screenWidth, screenHeight, SWP_NOZORDER);
                            MoveWindowToScreen(window.hwnd, screenIndex);
                            ProcessMessages(); // Process messages
                            //ShowWindow(window.hwnd, SW_MINIMIZE); // Minimize the window
                            if (IsIconic(window.hwnd) || IsZoomed(window.hwnd)) ShowWindow(window.hwnd, SW_RESTORE); // Restore the window
                            SetForegroundWindow(window.hwnd); // Bring the window to the foreground
                            ProcessMessages(); // Process messages
                            //MoveWindow(window.hwnd, screenRect.left + x, screenRect.top + y, 50, 50, TRUE); // Move and resize the window
                            //ProcessMessages(); // Process messages
                            MoveWindow(window.hwnd, screenRect.left + x, screenRect.top + y, windowWidth - 5, windowHeight, TRUE); // Move and resize the window
                            ProcessMessages(); // Process messages

                            x += windowWidth; // Increment the X position
                            if (x >= screenWidth - 20) { // Check if the X position exceeds the screen width
                                x = 0; // Reset the X position
                                y += windowHeight; // Increment the Y position
                            }
                            Sleep(100);
                            // Größe des klienten Bereichs ermitteln
                            RECT clientRect;
                            GetClientRect(window.hwnd, &clientRect);
                            int clientWidth = clientRect.right - clientRect.left;
                            int clientHeight = clientRect.bottom - clientRect.top;

                            // Fensterstil und erweiterte Stile ermitteln
                            DWORD style = GetWindowLong(window.hwnd, GWL_STYLE);
                            DWORD exStyle = GetWindowLong(window.hwnd, GWL_EXSTYLE);

                            // Anpassung der Fenstergröße basierend auf dem klienten Bereich
                            RECT adjustedRect = {0, 0, clientWidth, clientHeight};
                            AdjustWindowRectEx(&adjustedRect, style, FALSE, exStyle);

                            // Berechnete Fenstergröße
                            int adjustedWidth = adjustedRect.right - adjustedRect.left;
                            int adjustedHeight = adjustedRect.bottom - adjustedRect.top;
                            //////std::cout << windowWidth << " " << adjustedWidth << std::endl;
                            // Aktualisiere die maximale Breite
                            if (adjustedWidth > maxAdjustedWidth) {
                                maxAdjustedWidth = adjustedWidth;
                            }
                            // Aktualisiere die minimal Breite
                            if (adjustedWidth < minAdjustedWidth) {
                                minAdjustedWidth = adjustedWidth;
                            }

                            numWindows++; // Increment the counter
                        }
                    }
                }

                ////////std::cout << "minAdjustedWidth: " << minAdjustedWidth << std::endl;
                ////////std::cout << "cols: " << cols << std::endl;
                // Anzahl der Fenster nebeneinander und die Anzahl der Fensterreihen berechnen
                //std::cout << maxAdjustedWidth << " " << screenWidth << " " << cols << std::endl;
                cols = screenWidth / std::max(maxAdjustedWidth, screenWidth/cols);
                                //std::cout << cols << std::endl;
                ////////std::cout << "cols: " << cols << std::endl;                
                rows = (numWindows + cols - 1) / cols;
                                //std::cout << rows << std::endl;
                windowWidth = screenWidth / cols; // Calculate the window width
                //std::cout << windowWidth << std::endl;
                windowHeight = (screenHeight) / rows; // Calculate the window height
//std::cout << windowHeight << std::endl;
                // Fenster anordnen
                x = 0;
                y = 0;
                for (const auto& entry : processWindowsMap) { // Iterate through all processes
                    for (const auto& window : entry.second) { // Iterate through all windows of a process
                        if (window.checked) { // Check if the window is selected
                            MoveWindow(window.hwnd, screenRect.left + x, screenRect.top + y, windowWidth, windowHeight, TRUE); // Move and resize the window
                            ProcessMessages(); // Process messages

                            x += windowWidth; // Increment the X position
                            if (x >= screenWidth - 20) { // Check if the X position exceeds the screen width
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
                ProcessMessages(); // Process messages
            } else if (id == ID_TRAY_EXIT) { // Check if the ID is for tray exit
                PostQuitMessage(0); // Quit the application
            } else if ((id >= ID_MOVE_TO_SCREEN_BASE && id <= ID_MOVE_TO_SCREEN_BASE + screenCount) || LOWORD(wParam) == ID_CTRL_M) { // Check if the ID is 2004ff (Move to Screen x)
                MinimizeToTray(hwnd); // Minimize the window to the tray
                ClearTemporaryTiles();
                int screenIndex = 0;
                if (id != ID_CTRL_M) 
                    screenIndex = (id - ID_MOVE_TO_SCREEN_BASE);
                else
                {
                    std::vector<MonitorInfo> monitors;
                    EnumDisplayMonitors(NULL, NULL, MonitorEnumProc, (LPARAM)&monitors);
                    globalScreenIndexChosen++;
                    if (globalScreenIndexChosen > monitors.size()) globalScreenIndexChosen = 1;
                    if (monitors.size() > 1) screenIndex = globalScreenIndexChosen - 1;
                }
                //int screenIndex = (id - ID_MOVE_TO_SCREEN_BASE) ; // Handle move to screen action
                RECT screenRect = GetScreenRect(screenIndex);
                 for (const auto& entry : processWindowsMap) { // Iterate through all processes
                    for (const auto& window : entry.second) { // Iterate through all windows of a process
                        if (window.checked) { // Check if the window is selected
                            if (IsIconic(window.hwnd) || IsZoomed(window.hwnd)) ShowWindow(window.hwnd, SW_RESTORE);
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
            } else if (id >= ID_PROCESSNAMESTART && id < ID_PROCESSNAMESTART + processNames.size()) { // Check if the ID is within the range of process names and thus collapse/expand needed
                //MessageBoxW(hwnd, L"aha3", L"Debug Info", MB_OK);
                std::wstring processName = processNames[id - ID_PROCESSNAMESTART]; // Retrieve the process name based on the ID
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
                if (wcscmp(text, L"\u2315 Search Name (CTRL-F)") == 0) {
                    SetWindowText(hSearchBox, L"");
                }
            } else if (HIWORD(wParam) == EN_KILLFOCUS && LOWORD(wParam) == IDC_SEARCHBOX) {
                wchar_t text[256];
                GetWindowText(hSearchBox, text, 256);
                if (wcslen(text) == 0) {
                    SetEditPlaceholder(hSearchBox, L"\u2315 Search Name (CTRL-F)");
                }
            } else if (HIWORD(wParam) == EN_CHANGE && LOWORD(wParam) == IDC_SEARCHBOX) {
                wchar_t searchString[256];
                GetWindowTextW(hSearchBox, searchString, 256);
                SearchAndCheck(searchString, hwnd);
            }
        }
        break;

        case WM_LBUTTONDOWN: { // Message when the left mouse button is pressed
            ////////std::cout << "WM_LBUTTONDOWN" << std::endl;
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
                RECT rect = { 30, yPos, textWidth, yPos + 30 };
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

        case WM_MENUSELECT: {
            ////////std::cout << "WM_MENUSELECT" << std::endl;
            //////////std::cout << "Beginne WM_MENUSELECT" << std::endl;
            // Löschen Sie die temporären Kacheln bei jeder Menüauswahl
            try {
                //////////std::cout << "Vor ClearTemporaryTiles" << std::endl;
                ClearTemporaryTiles();
                //////////std::cout << "Nach ClearTemporaryTiles" << std::endl;

                if (HIWORD(wParam) & MF_MOUSESELECT) {
                    std::vector<MonitorInfo> monitors;
                    //////////std::cout << "Vor EnumDisplayMonitors" << std::endl;
                    if (!EnumDisplayMonitors(NULL, NULL, MonitorEnumProc, reinterpret_cast<LPARAM>(&monitors))) {
                        throw std::runtime_error("EnumDisplayMonitors failed");
                    }
                    //////////std::cout << "Nach EnumDisplayMonitors" << std::endl;

                    int menuIndex = LOWORD(wParam) - ID_ARRANGE;
                    if (menuIndex >= 0 && menuIndex < monitors.size()) {
                        //////////std::cout << "Vor ShowTemporaryTiles" << std::endl;
                        SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
                        StartShowTemporaryTilesInThread(hwnd, menuIndex);
                        //////////std::cout << "Nach ShowTemporaryTiles" << std::endl;
                    }
                    menuIndex = LOWORD(wParam) - ID_MOVE_TO_SCREEN_BASE;
                    if (menuIndex >= 0 && menuIndex < monitors.size()) {
                        //////////std::cout << "Vor ShowTemporaryTiles for Move To Screen" << std::endl;
                        SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
                        globalTotalCheckedIgnore = true;
                        StartShowTemporaryTilesInThread(hwnd, menuIndex);
                        //////////std::cout << "Nach ShowTemporaryTiles for Move To Screen" << std::endl;
                    }
                }
            } catch (const std::exception& e) {
                ////////std::cout << "Error in WM_MENUSELECT: " << e.what() << std::endl;
            }
            //////////std::cout << "Alles gut bei WM_MENUSELECT bis zum Ende" << std::endl;
        }
        break;

        /*case WM_MOVE:
        {
            int xPos = (int)(short)LOWORD(lParam); // horizontale Position
            int yPos = (int)(short)HIWORD(lParam); // vertikale Position

            // Setze die neuen Standardkoordinaten
            defaultXPos = xPos;
            defaultYPos = yPos;
        }
        break;*/

        case WM_MOUSEMOVE: { // Message when the mouse is moved
            ////////std::cout << "WM_MOUSEMOVE" << std::endl;
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
            ////////std::cout << "WM_LBUTTONUP" << std::endl;
            isScrolling = false; // Stop scrolling
            ReleaseCapture(); // Release the mouse
        }
        break;
        
        case WM_MOUSEWHEEL: { // Nachricht, wenn das Mausrad gedreht wird
            ////////std::cout << "WM_MOUSEWHEEL" << std::endl;
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
            ////////std::cout << "WM_PAINT" << std::endl;
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

                std::wstring textBeforeProcessName = expandedState[processName] ? L"\u25BC " : L"\u25B6 "; // Set the button text

                if (AlreadyOneChecked == windows.size()) {
                    textBeforeProcessName += L" "; // Platzhalter für das Quadrat
                } else if (AlreadyOneChecked > 0) {
                    if (expandedState[processName]) {
                        textBeforeProcessName += L" "; // Platzhalter für das Zeichen \u25EA
                    } else {
                        textBeforeProcessName += L" "; // Platzhalter für das Zeichen \u25EA
                    }
                } else {
                    if (i == highlightedRow && highlightedWindowRow == -1) textBeforeProcessName += checkboxState[processName] ? L" " : L"\u2610 "; // Platzhalter für das Quadrat
                }

                std::wstring textAfterProcessName = L" (" + std::to_wstring(AlreadyOneChecked) + L" / " + std::to_wstring(windows.size()) + L")";

                RECT rect = {10, yPos, textWidth, yPos + 30};

                if (i == highlightedRow && highlightedWindowRow == -1) {
                    HBRUSH highlightBrush = CreateSolidBrush(RGB(224, 224, 224)); // Hellgrau
                    FillRect(hdcMem, &rect, highlightBrush);
                    DeleteObject(highlightBrush);

                    // Hellblaue Linie am unteren Rand
                    HPEN bluePen = CreatePen(PS_SOLID, 2, RGB(74, 204, 229)); // Hellblau
                    HPEN oldPen = (HPEN)SelectObject(hdcMem, bluePen);
                    MoveToEx(hdcMem, rect.left, rect.bottom - 1, NULL);
                    LineTo(hdcMem, rect.right, rect.bottom - 1);
                    SelectObject(hdcMem, oldPen);
                    DeleteObject(bluePen);
                } else {
                    HBRUSH whiteBrush = CreateSolidBrush(RGB(255, 255, 255)); // Weiß
                    FillRect(hdcMem, &rect, whiteBrush);
                    DeleteObject(whiteBrush);
                }

                SetTextColor(hdcMem, RGB(0, 0, 0));
                SetBkMode(hdcMem, TRANSPARENT); // Setzen Sie den Hintergrundmodus auf transparent

                // Zeichnen Sie den Text vor dem Quadrat und dem Zeichen \u25EA
                DrawTextW(hdcMem, textBeforeProcessName.c_str(), -1, &rect, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);

                // Zeichnen Sie das Quadrat in Blau
                if (AlreadyOneChecked == windows.size() || (AlreadyOneChecked == 0 && checkboxState[processName])) {
                    //SetTextColor(hdcMem, RGB(0, 0, 255)); // Blau
                    SetTextColor(hdcMem, blinkState ? RGB(0, 0, 255) : RGB(255, 0, 0)); // Blau oder Rot
                    SIZE size;
                    GetTextExtentPoint32W(hdcMem, textBeforeProcessName.c_str(), textBeforeProcessName.length(), &size);
                    int textWidth = size.cx;
                    GetTextExtentPoint32W(hdcMem, L" ", 1, &size);
                    int spaceWidth = size.cx;
                    RECT squareRect = rect;
                    squareRect.left += textWidth - spaceWidth; // Korrigieren Sie die Position des Quadrats
                    DrawTextW(hdcMem, L"\u25A0", -1, &squareRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
                    SetTextColor(hdcMem, RGB(0, 0, 0)); // Schwarz
                }

                // Zeichnen Sie das Zeichen \u25EA in Blau
                if (AlreadyOneChecked > 0) {
                    //SetTextColor(hdcMem, RGB(0, 0, 255)); // Blau
                    SetTextColor(hdcMem, blinkState ? RGB(0, 0, 255) : RGB(255, 0, 0)); // Blau oder Rot
                    SIZE size;
                    GetTextExtentPoint32W(hdcMem, textBeforeProcessName.c_str(), textBeforeProcessName.length(), &size);
                    int textWidth = size.cx;
                    GetTextExtentPoint32W(hdcMem, L" ", 1, &size);
                    int spaceWidth = size.cx;
                    RECT arrowRect = rect;
                    arrowRect.left += textWidth - spaceWidth; // Korrigieren Sie die Position des Zeichens \u25EA
                    DrawTextW(hdcMem, L"\u25EA", -1, &arrowRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
                    SetTextColor(hdcMem, RGB(0, 0, 0)); // Schwarz
                }

                // Icon des Prozesses zeichnen
                DrawIconEx(hdcMem, rect.left + 45, yPos + 5, processIcons[processName], 20, 20, 0, NULL, DI_NORMAL);

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
                            //std::wstring windowText = std::wstring(window.checked ? L"\u25A0 " : L"\u2610 ") + std::wstring(window.title.begin(), window.title.end());
                            RECT windowRect = { 90, yPos, textWidth, yPos + 30 };

                            std::wstring windowText = std::wstring(L"    ") + std::wstring(window.title.begin(), window.title.end());

                            if (i*100000+j == highlightedWindowRow || window.checked) windowText = std::wstring(window.checked ? L"\u25A0 " : L"\u2610 ") + std::wstring(window.title.begin(), window.title.end());

                            if (i*100000+j == highlightedWindowRow) {
                                HBRUSH highlightBrush = CreateSolidBrush(RGB(224, 224, 224)); // Hellgrau
                                FillRect(hdcMem, &windowRect, highlightBrush);
                                DeleteObject(highlightBrush);

                                // Hellblaue Linie am unteren Rand
                                HPEN bluePen = CreatePen(PS_SOLID, 2, RGB(74, 204, 229)); // Hellblau
                                HPEN oldPen = (HPEN)SelectObject(hdcMem, bluePen);
                                MoveToEx(hdcMem, windowRect.left, windowRect.bottom - 1, NULL);
                                LineTo(hdcMem, windowRect.right, windowRect.bottom - 1);
                                SelectObject(hdcMem, oldPen);
                                DeleteObject(bluePen);
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
            globalTotalChecked = TotalChecked;
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
            ////////std::cout << "WM_SIZE" << std::endl;
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
            if (wParam == SIZE_MINIMIZED) {
                KillTimer(hwnd, BLINKING_TIMER_ID); // Timer stoppen
            } else {
                SetTimer(hwnd, BLINKING_TIMER_ID, 250, NULL); // Timer neu starten
            }
            // int buttonCount = 4; // Number of buttons
            // int buttonWidth = (width - 20) / buttonCount; // Calculate the width of a button

            // Position the buttons for minimize, restore, close, and arrange
            // SetWindowPos(minimizeButton, NULL, 10, yPos + 5 + 5, buttonWidth, 40, SWP_NOZORDER);
            // SetWindowPos(restoreButton, NULL, 10 + buttonWidth + 5, yPos + 5 + 5, buttonWidth, 40, SWP_NOZORDER);
            // SetWindowPos(closeButton, NULL, 10 + 2 * (buttonWidth + 5), yPos + 5 + 5, buttonWidth, 40, SWP_NOZORDER);
            // SetWindowPos(arrangeButton, NULL, 10 + 3 * (buttonWidth + 5), yPos + 5 + 5, buttonWidth, 40, SWP_NOZORDER);
            UpdateControlPositions(hwnd);
        }
        break;

        case WM_DESTROY: { // Message when the window is destroyed
            ////////std::cout << "WM_DESTROY" << std::endl;
            UnregisterHotKey(NULL, HOTKEY_ID);
            RemoveTrayIcon(hwnd); // Remove the tray icon
            PostQuitMessage(0); // Quit the application
            return 0; // Return 0
        }
        break;

        case WM_CLOSE: { // Message when the window is closed
            ////std::cout << "WM_CLOSE" << std::endl;
            //RemoveTrayIcon(hwnd); // Remove the tray icon (commented out)
            //DestroyWindow(hwnd); // Destroy the window (commented out)
            MinimizeToTray(hwnd); // Minimize the window to the tray
            return 0; // Return 0
        }
        break;

    case WM_TRAYICON:
    case WM_HOTKEY: { 
        ////std::cout << "WM_HOTKEY or WM_TRAYICON" << std::endl;
        if (uMsg == WM_HOTKEY && wParam != HOTKEY_ID) {
            ////std::cout << "Incorrect hotkey ID" << std::endl;
            break; // Ensure it's the correct hotkey
        }
        if (lParam == WM_LBUTTONUP || uMsg == WM_HOTKEY) { 
            ////std::cout << "Restoring window" << std::endl;

            // Show the window and bring it to the foreground as early as possible
            // Get the screen dimensions
            RECT screenRect;
            GetWindowRect(GetDesktopWindow(), &screenRect);
            int screenWidth = screenRect.right;
            int screenHeight = screenRect.bottom;

            // Get the window dimensions
            RECT windowRect;
            GetWindowRect(hwnd, &windowRect);
            int windowWidth = windowRect.right - windowRect.left;
            int windowHeight = windowRect.bottom - windowRect.top;

            // Calculate the new position to center the window
            int newX = (screenWidth - windowWidth) / 2;
            int newY = (screenHeight - windowHeight) / 2;

            // Restore, move, and bring the window to the foreground in one call
            SetWindowPos(hwnd, HWND_TOP, newX, newY, windowWidth, windowHeight, SWP_SHOWWINDOW);

            // Perform essential operations
            RefreshWindowList(hwnd);
            AdjustWindowToFitMenu(hwnd);
            //UpdateWindowList(hwnd);

            // Perform non-essential operations asynchronously
            std::thread([hwnd]() {
                AdjustWindowSize(hwnd); 
                std::vector<BOOL> timerStates(timers.size());
                RestartTimers(hwnd, timers, timerStates);
                CreateTrayIcon(hwnd);
                //UpdateWindowList(hwnd);
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

                SaveCurrentWindows(); 
                ProcessMessages();
                //InvalidateWindow(hwnd);
                ProcessMessages();
            }).detach();
                // Additional actions if needed
                //RefreshWindowList(hwnd); 
                //ShowWindow(hwnd, SW_RESTORE); 
                //MoveWindowToPrimaryMonitor(hwnd);
                //SaveCurrentWindows(); 
                //AdjustWindowSize(hwnd); 
                //AdjustWindowToFitMenu(hwnd);
                //ProcessMessages();
                //Sleep(150);
                //SetForegroundWindow(hwnd); 
                //InvalidateWindow(hwnd);
                //ProcessMessages();
            SetForegroundWindow(hwnd);
            SetFocus(hwnd);
        } else if (lParam == WM_RBUTTONUP) { 
            ShowTrayMenu(hwnd); 
        }
    }
    break;


        default: { // Default message
            ////////std::cout << "default" << std::endl;
            return DefWindowProc(hwnd, uMsg, wParam, lParam); // Call the default window procedure
        }
    }
    return 0; 
}

// Main function
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // Handle zum aktuellen Prozess abrufen
    HANDLE hProcess = GetCurrentProcess();

    // Prioritätsklasse auf HIGH_PRIORITY_CLASS setzen
    if (SetPriorityClass(hProcess, HIGH_PRIORITY_CLASS)) {
        //////std::cout << "Die Prioritätsstufe wurde erfolgreich erhöht." << std::endl;
    } else {
        //std::cerr << "Fehler beim Setzen der Prioritätsstufe." << std::endl;
    }
    ////////std::cout << "WinMain" << std::endl;
    // Main function for Windows applications
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

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
        WS_OVERLAPPED | WS_SYSMENU | WS_VSCROLL | WS_EX_COMPOSITED, // Window style. 
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, // Position and size of the window
        NULL, // No parent window
        NULL, // No menu
        hInstance, // Instance handle
        NULL // No additional parameters
    );

    if (hwnd == NULL) {
        return 0; // Return 0 if the window could not be created
    }

    // Register the hotkey (CTRL+SHIFT+M)
    if (!RegisterHotKey(hwnd, HOTKEY_ID, MOD_CONTROL | MOD_SHIFT, 'M')) {
        ////std::cout << "Failed to register hotkey" << std::endl;
        return 1;
    }

    // Register the hotkey (CTRL+X)
    /*if (!RegisterHotKey(hwnd, HOTKEY_EXIT, MOD_CONTROL, 'X')) {
        ////std::cout << "Failed to register hotkey" << std::endl;
        return 1;
    }*/

    HICON hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MYICON)); // Load the window icon
    SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon); // Set the small window icon
    SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon); // Set the large window icon

    ShowWindow(hwnd, SW_HIDE); // Hide the window at startup
    UpdateWindow(hwnd); // Update the window

    MSG msg = {}; // Initialize a MSG structure
    while (GetMessage(&msg, NULL, 0, 0)) {
        ////std::cout << "Message received: " << msg.message << std::endl;
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    UnregisterHotKey(NULL, HOTKEY_ID);

    return 0; // Return 0 when the application exits
}