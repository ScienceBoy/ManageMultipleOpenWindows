// Update log:
// 2025.02.10: First version pushed to Windows Store
// 2025.02.12: Changed the exit-button on the context menu of the tray icon
// 2025.02.12: Removed the move to main screen for the action "minimize"
// 2025.02.12: Added bool actionOnGoing to avoid an actions is started before a former action has finished
// 2025.02.12: Pushed to Windows Store as v1.02.1225
// 2025.02.12: Removed the move to main screen for the action "minimize" when started via contect menu of the tray icon
// 2025.02.12: Allowed more special characters in search field, e.g. "dot" and "question mark" and french characters
// 2025.02.12: Added MinimizeToTray(hwnd); when GOTO-BUTTON was used to close the main window
// 2025.02.12: Added SHIFT+HOME to select the text in the search box
// 2025.02.12: Added checks for each division to avoid division-by-zero errors
// 2025.02.12: Added "msedgewebview2.exe" and "PDSyleAgent.exe" to list of ignored windows
// 2025.02.13: Pushed to Windows Store as v1.02.1325
// 2025.02.13: Changed version number to always be "v1.mm.ddyy"
// 2025.02.14: Reduced minimum height of main window to 100 pixels
// 2025.02.14: Removed call of AdjustWindowSize() in SearchAndCheck()
// 2025.02.14: Added animation AnimateWindowResize() when changing height of main window
// 2025.02.14: Fixed "jumping" of searchBox by moving UpdateControlPositions(hwnd); to almost the end of wm_paint
// 2025.02.15: Pushed to Windows Store as v1.02.1525
// 2025.02.21: Changed number of selected and total windows to two-digits and added the word "selected" as well as the list of selected window names after them
// 2025.02.21: Introduced bool windowReady to not react on key up / down before window is shown & ready
// 2025.02.22: Changed from displaying (3/12) to (12 windows) if no window of this process is selected or (02 / 12 selected) [ window title 1 ] [ window titel 2 ] if selected
// 2025.02.22: Made the main windows resizable to allow more text horizontalle
// 2025.02.22: Added a toolbar, but not yet with correct icons and with call of actions
// 2025.02.22: Calling the scrollbar update via UpdateScrollBar only if there was a change in the content
// 2025.02.23: Added the icons to the toolbar and made them trigger actions
// 2025.02.24: Changed the relevant " + 30" to " + heightToolbar" to be flexible in toolbar height
// 2025.02.24: Changed the toolbar height from 16px to 24px
// 2025.02.24: Set a minimum of (200 + heightToolbar) for the main window to avoid continuous repainting and hidden text

//#define UNICODE
//#define _UNICODE
#define NOMINMAX

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
#include <clocale>
#include <cwctype>
#include <commctrl.h>
#include <map>
#include <shellapi.h>
#include <winuser.h>
#include <stdexcept>
#include <dwmapi.h>
#include <thread>
#include <functional>
#include <ctime>
#include <sstream>
#include <iomanip>
#pragma comment(lib, "Dwmapi.lib")
#pragma comment(lib, "psapi.lib") // Link psapi.lib library

extern "C" UINT GetDpiForWindow(HWND hwnd);
// Manuelle Deklaration der Typdefinition und der Funktion
//typedef HANDLE DPI_AWARENESS_CONTEXT;
#ifndef DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2
#define DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2 ((DPI_AWARENESS_CONTEXT)-4)
#endif

//extern "C" BOOL SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT value);

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

#define IDI_ICON_MINIMIZE_contextmenu       3101
#define IDI_ICON_ARRANGE_contextmenu        3102
#define IDI_ICON_CLOSE_contextmenu          3103
/*#define IDI_ICON_ABOUT          3104
#define IDI_ICON_EXIT           3105
#define IDI_ICON_MAXIMIZE       3107
#define IDI_ICON_MOVE           3108
#define IDI_ICON_REFRESH        3109
#define IDI_ICON_RESTORE        3110
#define IDI_ICON_SAVE           3111
#define IDI_ICON_SELECTALL      3112
#define IDI_ICON_UNSELECTALL    3113
#define IDI_ICON_SELECTONSCREEN 3114*/

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
#define ID_CTRL_C               3780
#define ID_CTRL_S               3785
#define ID_CTRL_U               3787
#define ID_CTRL_Q               3790
#define ID_CTRL_F               3795
#define ID_SELECT_SCREEN_BASE   3800
#define ID_SELECT_ALL           3840 
#define ID_SELECT_FROM_SCREEN   3850
#define ID_SELECT_NONE          3860 
#define ID_SELECT_VISIBLE       3880 
#define ID_REFRESH              3890
#define ID_ABOUT                3900

#define IDB_TOOLBAR             5000
#define IDI_ICON_REFRESH        5001
#define IDI_ICON_CLOSE          5002
#define IDI_ICON_EXIT           5003
#define IDI_ICON_MINIMIZE       5004
#define IDI_ICON_MAXIMIZE       5005
#define IDI_ICON_RESTORE        5006
#define IDI_ICON_ARRANGE        5007
#define IDI_ICON_MOVE           5008
#define IDI_ICON_SELECT_SCREEN  5009
#define IDI_ICON_SELECTALL      5010
#define IDI_ICON_UNSELECTALL    5011
#define IDI_ICON_SAVE           5012
#define IDI_ICON_ABOUT          5013
#define IDI_ICON                5014
#define IDI_ICON_SELECTONSCREEN 5015

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
    HMONITOR hMonitor;
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
void GetScreenCaptures(bool forceRecreate);
HMENU CreateSelectScreenMenu(HMENU hMenu, bool forceRecreate);
BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData);
void LoadIcons(HINSTANCE hInstance);

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
const int minWidth = 900; // Lege eine minimale Breite fest
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
std::map<int, HBITMAP> hBitmapCache;
int firstIconHeight = 0;
MonitorInfo largestMonitor;
int widthLargestMonitor = 0;
int widthSecondLargestMonitor = 0;
int previousHeight = 0;
POINT prevPt = { -1, -1 };
bool actionOnGoing = false;
bool windowReady = false;
HWND hwndToolbar = NULL;;
int previousNumberOfItems = 0;
HIMAGELIST hImageList;
HICON hIconMinimize, hIconArrange, hIconClose, hIconAbout, hIconExit, hIconIcon, hIconMaximize, hIconMove, hIconRefresh, hIconRestore, hIconSave, hIconSelectAll, hIconUnselectAll, hIconSelectOnScreen;
int heightToolbar = 48;

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

void LoadIcons(HINSTANCE hInstance) {
    hImageList = ImageList_Create(heightToolbar/2, heightToolbar / 2, ILC_COLOR32 | ILC_MASK, 14, 0);
    if (!hImageList) {
        MessageBox(NULL, L"Failed to create image list.", L"Error", MB_OK | MB_ICONERROR);
        return;
    }

    hIconMinimize = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON_MINIMIZE));
    if (!hIconMinimize) MessageBox(NULL, L"Failed to load icon: IDI_ICON_MINIMIZE", L"Error", MB_OK | MB_ICONERROR);

    hIconArrange = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON_ARRANGE));
    if (!hIconArrange) MessageBox(NULL, L"Failed to load icon: IDI_ICON_ARRANGE", L"Error", MB_OK | MB_ICONERROR);

    hIconClose = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON_CLOSE));
    if (!hIconClose) MessageBox(NULL, L"Failed to load icon: IDI_ICON_CLOSE", L"Error", MB_OK | MB_ICONERROR);

    hIconAbout = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON_ABOUT));
    if (!hIconAbout) MessageBox(NULL, L"Failed to load icon: IDI_ICON_ABOUT", L"Error", MB_OK | MB_ICONERROR);

    hIconExit = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON_EXIT));
    if (!hIconExit) MessageBox(NULL, L"Failed to load icon: IDI_ICON_EXIT", L"Error", MB_OK | MB_ICONERROR);

    hIconMaximize = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON_MAXIMIZE));
    if (!hIconMaximize) MessageBox(NULL, L"Failed to load icon: IDI_ICON_MAXIMIZE", L"Error", MB_OK | MB_ICONERROR);

    hIconMove = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON_MOVE));
    if (!hIconMove) MessageBox(NULL, L"Failed to load icon: IDI_ICON_MOVE", L"Error", MB_OK | MB_ICONERROR);

    hIconRefresh = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON_REFRESH));
    if (!hIconRefresh) MessageBox(NULL, L"Failed to load icon: IDI_ICON_REFRESH", L"Error", MB_OK | MB_ICONERROR);

    hIconRestore = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON_RESTORE));
    if (!hIconRestore) MessageBox(NULL, L"Failed to load icon: IDI_ICON_RESTORE", L"Error", MB_OK | MB_ICONERROR);

    hIconSave = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON_SAVE));
    if (!hIconSave) MessageBox(NULL, L"Failed to load icon: IDI_ICON_SAVE", L"Error", MB_OK | MB_ICONERROR);

    hIconSelectAll = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON_SELECTALL));
    if (!hIconSelectAll) MessageBox(NULL, L"Failed to load icon: IDI_ICON_SELECTALL", L"Error", MB_OK | MB_ICONERROR);

    hIconUnselectAll = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON_UNSELECTALL));
    if (!hIconUnselectAll) MessageBox(NULL, L"Failed to load icon: IDI_ICON_UNSELECTALL", L"Error", MB_OK | MB_ICONERROR);

    hIconSelectOnScreen = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON_SELECTONSCREEN));
    if (!hIconSelectOnScreen) MessageBox(NULL, L"Failed to load icon: IDI_ICON_SELECTONSCREEN", L"Error", MB_OK | MB_ICONERROR);

    if (ImageList_AddIcon(hImageList, hIconExit) == -1) MessageBox(NULL, L"Failed to add icon: hIconExit", L"Error", MB_OK | MB_ICONERROR);
    if (ImageList_AddIcon(hImageList, hIconRefresh) == -1) MessageBox(NULL, L"Failed to add icon: hIconRefresh", L"Error", MB_OK | MB_ICONERROR);
    if (ImageList_AddIcon(hImageList, hIconMinimize) == -1) MessageBox(NULL, L"Failed to add icon: hIconMinimize", L"Error", MB_OK | MB_ICONERROR);
    if (ImageList_AddIcon(hImageList, hIconMaximize) == -1) MessageBox(NULL, L"Failed to add icon: hIconMaximize", L"Error", MB_OK | MB_ICONERROR);
    if (ImageList_AddIcon(hImageList, hIconRestore) == -1) MessageBox(NULL, L"Failed to add icon: hIconRestore", L"Error", MB_OK | MB_ICONERROR);
    if (ImageList_AddIcon(hImageList, hIconArrange) == -1) MessageBox(NULL, L"Failed to add icon: hIconArrange", L"Error", MB_OK | MB_ICONERROR);
    if (ImageList_AddIcon(hImageList, hIconMove) == -1) MessageBox(NULL, L"Failed to add icon: hIconMove", L"Error", MB_OK | MB_ICONERROR);
    if (ImageList_AddIcon(hImageList, hIconSelectAll) == -1) MessageBox(NULL, L"Failed to add icon: hIconSelectAll", L"Error", MB_OK | MB_ICONERROR);
    if (ImageList_AddIcon(hImageList, hIconUnselectAll) == -1) MessageBox(NULL, L"Failed to add icon: hIconUnselectAll", L"Error", MB_OK | MB_ICONERROR);
    if (ImageList_AddIcon(hImageList, hIconSelectOnScreen) == -1) MessageBox(NULL, L"Failed to add icon: hIconSelectOnScreen", L"Error", MB_OK | MB_ICONERROR);
    if (ImageList_AddIcon(hImageList, hIconSave) == -1) MessageBox(NULL, L"Failed to add icon: hIconSave", L"Error", MB_OK | MB_ICONERROR);
    //if (ImageList_AddIcon(hImageList, hIconClose) == -1) MessageBox(NULL, L"Failed to add icon: hIconClose", L"Error", MB_OK | MB_ICONERROR);
    if (ImageList_AddIcon(hImageList, hIconAbout) == -1) MessageBox(NULL, L"Failed to add icon: hIconAbout", L"Error", MB_OK | MB_ICONERROR);
}

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam) {
    //std::cout << "EnumWindowsProc" << std::endl;
    wchar_t title[256]; // Buffer für den Fenstertitel
    DWORD processId; // Prozess-ID
    GetWindowThreadProcessId(hwnd, &processId); // Prozess-ID des Fensters abrufen
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processId); // Prozess öffnen
    if (hProcess) {
        wchar_t exePath[MAX_PATH]; // Buffer für den Pfad zur ausführbaren Datei
        if (GetModuleFileNameExW(hProcess, NULL, exePath, sizeof(wchar_t) != 0 ? sizeof(exePath) / sizeof(wchar_t) : 1)) { // Pfad zur ausführbaren Datei abrufen
            wchar_t processName[MAX_PATH]; // Buffer für den Prozessnamen
            if (GetModuleBaseNameW(hProcess, NULL, processName, sizeof(wchar_t) != 0 ? sizeof(processName) / sizeof(wchar_t) : 1)) { // Prozessnamen abrufen
                // Liste der auszuschließenden Prozesse, die von Windows erstellt werden und/oder für den Benutzer nicht nützlich zu sehen sind
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
                    L"Pulse.exe",
                    L"msedgewebview2.exe",
                    L"PDSyleAgent.exe",
                    L"window_minimizer.exe"
                };
                std::wstring processNameStr(processName);
                if (processNameStr.find(L"CodeSetup") == 0 && processNameStr.rfind(L"tmp") == processNameStr.length() - 3) {
                    CloseHandle(hProcess); // Prozesshandle schließen
                    return TRUE; // Prozess ausschließen und Enumeration fortsetzen
                }
                if (IsWindowVisible(hwnd) && std::find(excludedProcesses.begin(), excludedProcesses.end(), processName) == excludedProcesses.end()) { // Überprüfen, ob das Fenster sichtbar und nicht ausgeschlossen ist
                    int length = GetWindowTextW(hwnd, title, sizeof(wchar_t) != 0 ? sizeof(title) / sizeof(wchar_t) : 1); // Fenstertitel abrufen
                    if (length > 0 && wcscmp(title, L"Program Manager") != 0 && wcscmp(title, L"JamPostMessageWindow") != 0) { // "Program Manager" ausschließen
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
    std::vector<WindowInfo>* windows = reinterpret_cast<std::vector<WindowInfo>*>(lParam);
    if (windows->size() > 1) {
        for (auto it = windows->begin(); it != windows->end(); ) {
            if (it->processName == L"ManyOpenWindows" || it->processName == L"Many Open Windows" || it->processName == L"Many Open Windows VS") {
                it = windows->erase(it);
            } else {
                ++it;
            }
        }
    }
    return TRUE; // Enumeration fortsetzen
}

int getTaskbarHeight(HWND hwnd) {
    //std::cout << "getTaskbarHeight" << std::endl;
    HWND taskbar = FindWindow(L"Shell_TrayWnd", NULL);
    if (taskbar) {
        RECT rect;
        if (GetWindowRect(taskbar, &rect)) {
            UINT dpi = GetDpiForWindow(hwnd);
            float scaleFactor = dpi / 96.0f; // 96 ist der Standard-DPI-Wert
            int taskbarHeight = scaleFactor != 0 ? (rect.bottom - rect.top) / scaleFactor : 80;
            //std::cout << "taskbar height: " << taskbarHeight << std::endl;
            return taskbarHeight;
        }
    }
    return 0; // Fehlerfall
}

void UpdateScrollBar(HWND hwnd, int nmbOfItemsOnWindow) {
    // Überprüfe, ob sich die Anzahl der Elemente geändert hat
    if (nmbOfItemsOnWindow != previousNumberOfItems) {
        // Aktualisiere die vorherige Anzahl der Elemente
        previousNumberOfItems = nmbOfItemsOnWindow;

        // SCROLLINFO-Struktur initialisieren
        SCROLLINFO si = {};
        si.cbSize = sizeof(si);
        si.fMask = SIF_RANGE | SIF_PAGE;
        si.nMin = 0;
        si.nMax = std::max(nmbOfItemsOnWindow * 30 + 30, 30);
        si.nPage = si.nMax / std::max(nmbOfItemsOnWindow, 1);

        // Scrollinformationen für den vertikalen Scrollbalken setzen
        SetScrollInfo(hwnd, SB_VERT, &si, TRUE);
    }
}

// Function to return open windows
std::vector<WindowInfo> getOpenWindows() {
    //std::cout << "getOpenWindows" << std::endl;
    std::vector<WindowInfo> windows; // Vector to store windows
    EnumWindows(EnumWindowsProc, reinterpret_cast<LPARAM>(&windows)); // Enumerate windows
    return windows; // Return the list of windows
}

// Function to minimize the window to the tray
void MinimizeToTray(HWND hwnd) {
    //std::cout << "MinimizeToTray" << std::endl;
    std::vector<BOOL> timerStates(timers.size());
    CheckAndStopTimers(hwnd, timers, timerStates);
    ShowWindow(hwnd, SW_HIDE); // Hide the window
}

// Function to create the tray icon
void CreateTrayIcon(HWND hwnd) {
    //std::cout << "CreateTrayIcon" << std::endl;
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
    //std::cout << "RemoveTrayIcon" << std::endl;
    NOTIFYICONDATAW nid = {}; // Structure for the tray icon
    nid.cbSize = sizeof(nid); // Size of the structure
    nid.hWnd = hwnd; // Handle of the window
    nid.uID = 1; // ID of the tray icon
    Shell_NotifyIconW(NIM_DELETE, &nid); // Remove the tray icon
}

// Function to display the tray menu
void ShowTrayMenu(HWND hwnd){
    //std::cout << "ShowTrayMenu" << std::endl;
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
    //std::cout << "ProcessMessages" << std::endl;
    MSG msg; // Structure for the message
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) { // Retrieve messages
        TranslateMessage(&msg); // Translate the message
        DispatchMessage(&msg); // Dispatch the message
    }
}

// Function to save the current windows
void SaveCurrentWindows() {
    //std::cout << "SaveCurrentWindows" << std::endl;
    currentWindows = getOpenWindows(); // Save the current windows
}

bool CompareMonitors(const MonitorInfo& a, const MonitorInfo& b) {
    //std::cout << "CompareMonitors" << std::endl;
    return a.rect.left < b.rect.left;
}

bool compareMonitorInfo(const MonitorInfo& info, const HMONITOR& hMonitor) {
    return info.hMonitor == hMonitor;
}

bool CompareMonitorsWidthAndHeight(const MonitorInfo& a, const MonitorInfo& b) {
    //std::cout << "CompareMonitorsWidthAndHeight" << std::endl;
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
    //std::cout << "GetScreenRect" << std::endl;
    std::vector<MonitorInfo> monitors;
    EnumDisplayMonitors(NULL, NULL, MonitorEnumProc, (LPARAM)&monitors);

    if (screenIndex < 0 || screenIndex >= monitors.size()) {
        //MessageBox(NULL, "Ungueltiger Bildschirm-Index", "Fehler", MB_OK | MB_ICONERROR);
        return {0, 0, 0, 0};
    }

    return monitors[screenIndex].rect;
}

BOOL CALLBACK RedrawWindowCallback(HWND hwnd, LPARAM lParam) {
    //std::cout << "RedrawWindowCallback" << std::endl;
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
    //std::cout << "DrawRectangleWithScaling" << std::endl;
    // Ermitteln des DPI-Werts für das Fenster
    UINT dpi = GetDpiForWindow(hwnd);
    float scaleFactor = dpi / 96.0f; // 96 ist der Standard-DPI-Wert

    //std::cout << "scaleFactor: " << scaleFactor << std::endl;

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
                int rows = cols > 0 ? (numWindows + cols - 1) / cols : 1;
                int windowWidth = cols > 0 ? (rect.right - rect.left) / cols : 1;
                int windowHeight = rows > 0 ? (rect.bottom - rect.top) / rows : 1;

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
            int rows = cols > 0 ? (numWindows + cols - 1) / cols : 1;
            int windowWidth = cols > 0 ? screenWidth / cols : 1;
            int windowHeight = rows > 0 ? screenHeight / rows : 1;

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
    /*//std::wcout << L"Screen Index: " << params->second << std::endl;
    //std::wcout << L"Screen Rect: (" << screenRect.left << L", " << screenRect.top << L") - ("
               << screenRect.right << L", " << screenRect.bottom << L")" << std::endl;
    //std::wcout << L"Screen Width: " << screenWidth << L", Screen Height: " << screenHeight << std::endl;*/

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
    /*//std::wcout << L"Overlay Window Rect: (" << overlayRect.left << L", " << overlayRect.top << L") - ("
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
    //std::cout << "MonitorEnumProc" << std::endl;
    std::vector<MonitorInfo>* monitors = reinterpret_cast<std::vector<MonitorInfo>*>(dwData);
    
    MONITORINFOEX mi;
    mi.cbSize = sizeof(mi);
    if (GetMonitorInfo(hMonitor, &mi)) {
        monitors->push_back({static_cast<int>(monitors->size()), mi.rcMonitor, L""});
    }
    //std::cout << "MonitorEnumProc beendet" << std::endl;
    return TRUE;
}

BOOL CALLBACK MonitorEnumProcSpecial(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData) {
    std::vector<MonitorInfo>* monitors = reinterpret_cast<std::vector<MonitorInfo>*>(dwData);
    
    MONITORINFOEX mi;
    mi.cbSize = sizeof(mi);
    if (GetMonitorInfo(hMonitor, &mi)) {
        monitors->push_back({static_cast<int>(monitors->size()), mi.rcMonitor, mi.szDevice, hMonitor});
        //std::cout << "Monitor added: " << hMonitor << " (" << mi.szDevice << ")" << std::endl;
    } else {
        std::cerr << "GetMonitorInfo failed for monitor: " << hMonitor << " with error: " << GetLastError() << std::endl;
    }
    return TRUE;
}

bool IsValidInput(const wchar_t* input) {
    while (*input) {
        if (!iswalnum(*input) && // Alphanumerische Zeichen
            *input != L'\\' && *input != L'/' && // Backslash und Forwardslash
            *input != L'[' && *input != L']' && *input != L'*' && *input != L'-' && *input != L'+' && *input != L'*' && *input != L'=' && *input != L'!' && *input != L'?' &&
            *input != L'{' && *input != L'}' && *input != L'&' && *input != L'%' && *input != L'#' && *input != L' ' &&
            *input != L'@' && *input != L'(' && *input != L')' && *input != L'^' && *input != L'~' &&
            *input != L'é' && *input != L'è' && *input != L'à' && *input != L'ç' && *input != L'ñ' && *input != L'¡' && *input != L'¿' &&
            *input != L'ì' && *input != L'ò' && *input != L'ù' && *input != L'$' && *input != L'£' &&
            *input != L'.' && *input != L'\u00B7' && // Punkt und Mittelpunkt
            *input != L'<' && *input != L'>' && *input != L',' &&
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

    int windowWidth = windowRect.right - windowRect.left;
    int windowHeight = windowRect.bottom - windowRect.top;

    RECT screenRect = GetScreenRect(screenIndex);
    if (screenRect.left == 0 && screenRect.top == 0 && screenRect.right == 0 && screenRect.bottom == 0) {
        return;
    }

    int newScreenWidth = screenRect.right - screenRect.left;
    int newScreenHeight = screenRect.bottom - screenRect.top;

    //std::cout << newScreenWidth << " " << newScreenHeight << std::endl;

    // Calculate the relative position on the current screen
    HMONITOR hMonitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
    MONITORINFO mi;
    mi.cbSize = sizeof(MONITORINFO);
    if (!GetMonitorInfo(hMonitor, &mi)) {
        return;
    }

    int currentScreenX = mi.rcMonitor.left;
    int currentScreenY = mi.rcMonitor.top;

    int currentScreenWidth = mi.rcMonitor.right - mi.rcMonitor.left;
    int currentScreenHeight = mi.rcMonitor.bottom - mi.rcMonitor.top;

    //std::cout << currentScreenWidth << " " << currentScreenHeight << std::endl;

    float ratioX = currentScreenWidth != 0 ? static_cast<float>(newScreenWidth) / currentScreenWidth : 1;
    float ratioY = currentScreenHeight != 0 ? static_cast<float>(newScreenHeight) / currentScreenHeight : 1;

    //std::cout << ratioX << " " << ratioY << std::endl;

    int relativeX = windowRect.left - currentScreenX;
    int relativeY = windowRect.top - currentScreenY;

    // Calculate the new position on the target screen
    int newX = screenRect.left + static_cast<int>(relativeX * ratioX);
    int newY = screenRect.top + static_cast<int>(relativeY * ratioY);

    //std::cout << newX << " " << newY << std::endl;
    //std::cout << windowWidth * ratioX << " " << windowHeight * ratioY << std::endl;

    // Move the window to the new position on the target screen
    if (!SetWindowPos(hwnd, NULL, newX, newY, static_cast<int>(windowWidth * ratioX), static_cast<int>(windowHeight * ratioY), SWP_NOZORDER | SWP_SHOWWINDOW)) {
        return;
    }

    if (wasMaximized) {
        ShowWindow(hwnd, SW_MAXIMIZE);
    }
}

HBITMAP CaptureAndResizeScreen(HWND hwnd, RECT rect, int width, int height) {
    //std::cout << "CaptureAndResizeScreen" << std::endl;
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
    //std::cout << "AddMenuItemWithImage" << std::endl;
    MENUITEMINFOW mii = { sizeof(MENUITEMINFOW) };
    mii.fMask = MIIM_BITMAP | MIIM_STRING | MIIM_ID;
    mii.wID = uIDNewItem;
    mii.dwTypeData = const_cast<LPWSTR>(text.c_str());
    mii.cch = static_cast<UINT>(text.size());
    mii.hbmpItem = hBitmap;
    InsertMenuItemW(hMenu, uIDNewItem, FALSE, &mii);
}

void MoveWindowToPrimaryMonitor(HWND hwnd) {
    //std::cout << "MoveWindowToPrimaryMonitor" << std::endl;
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

void CreateMoveToScreenMenu(HMENU hMenu, bool forceRecreate = false) {
    // Rufen Sie GetScreenCaptures auf, um die Bildschirmaufnahmen zu aktualisieren
    GetScreenCaptures(forceRecreate);

    HMENU hMoveToScreenMenu = CreateMenu();
    std::vector<MonitorInfo> monitors;
    EnumDisplayMonitors(NULL, NULL, MonitorEnumProc, reinterpret_cast<LPARAM>(&monitors));

    // Sortiere die Monitore nach ihrer linken Koordinate
    std::sort(monitors.begin(), monitors.end(), CompareMonitors);

    if (monitors.size() > 1) { // Check if more than one screen is present
        screenCount = -1;

        for (const auto& monitor : monitors) {
            screenCount++;
            int width = monitor.rect.right - monitor.rect.left;
            int height = monitor.rect.bottom - monitor.rect.top;
            int newHeight = firstIconHeight;
            std::wstring menuText = L"Screen " + std::to_wstring(screenCount + 1) + L" (" + 
                                    std::to_wstring(width) + L"x" + 
                                    std::to_wstring(height) + L")";

            if (monitor.rect == largestMonitor.rect) {
                newHeight = widthSecondLargestMonitor != 0 ? static_cast<int>(newHeight * widthLargestMonitor / widthSecondLargestMonitor) : 1024;
            }

            // Verwenden Sie die HBITMAPs aus dem Cache
            HBITMAP hBitmap = hBitmapCache[screenCount];
            AddMenuItemWithImage(hMoveToScreenMenu, ID_MOVE_TO_SCREEN_BASE + monitor.index, hBitmap, menuText);
        }
        AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hMoveToScreenMenu, L"&Move Window(s)\tCTRL+M");
    } else {
        // Move to the same (and only) screen does not make sense, so no menu
        // AppendMenu(hMenu, MF_STRING, ID_MOVE_TO_SCREEN_BASE, L"Mo&ve Window(s)");
    }
}

// Funktion zum Trimmen von Leerzeichen
std::wstring trim(const std::wstring& str) {
    //std::cout << "trim" << std::endl;
    size_t first = str.find_first_not_of(L' ');
    if (first == std::wstring::npos) return L"";
    size_t last = str.find_last_not_of(L' ');
    return str.substr(first, last - first + 1);
}

// Funktion zum Umwandeln von Großbuchstaben in nur den ersten Großbuchstaben
std::wstring capitalizeIfAllCaps(const std::wstring& str) {
    //std::cout << "capitalizeIfAllCaps" << std::endl;
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
    //std::cout << "toLower" << std::endl;
    std::wstring lowerStr = str;
    std::transform(lowerStr.begin(), lowerStr.end(), lowerStr.begin(), ::towlower);
    return lowerStr;
}

void ShowTooltip(HWND hwnd, const wchar_t* message) {
    //std::cout << "ShowTooltip" << std::endl;
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


void GetScreenCaptures(bool forceRecreate = false) {
    // Prüfen, ob hBitmapCache gefüllt ist
    if (!forceRecreate && !hBitmapCache.empty()) {
        return;
    }

    HMENU hArrangeOnScreenMenu = CreateMenu();
    std::vector<MonitorInfo> monitors;
    EnumDisplayMonitors(NULL, NULL, MonitorEnumProc, reinterpret_cast<LPARAM>(&monitors));
    
    // Sortiere die Monitore nach ihrer Breite
    std::sort(monitors.begin(), monitors.end(), CompareMonitorsWidthAndHeight);

    auto largestMonitor = monitors[0];
    auto secondLargestMonitor = monitors[0];
    if (monitors.size() > 1) {
        secondLargestMonitor = monitors[1];
    }
    int widthLargestMonitor = largestMonitor.rect.bottom - largestMonitor.rect.top;
    int widthSecondLargestMonitor = secondLargestMonitor.rect.bottom - secondLargestMonitor.rect.top;

    int width = largestMonitor.rect.right - largestMonitor.rect.left;
    int height = largestMonitor.rect.bottom - largestMonitor.rect.top;
    int newWidth, newHeight;
    int firstIconHeight = 0; // Variable to store the height of the first icon

    // Berechne das Seitenverhältnis für den breitesten Monitor
    if (width > height) {
        newWidth = 25;
        newHeight = width > 0 ? static_cast<int>(35.0 * height / width) : 1280;
    } else {
        newHeight = 25;
        newWidth = height > 0 ? static_cast<int>(35.0 * width / height) : 1024;
    }
    firstIconHeight = newHeight; // Speichere die Höhe des ersten Icons
    
    // Sortiere die Monitore nach ihrer linken Koordinate
    std::sort(monitors.begin(), monitors.end(), CompareMonitors);

    screenCount = -1;

    for (const auto& monitor : monitors) {
        screenCount++;
        width = monitor.rect.right - monitor.rect.left;
        height = monitor.rect.bottom - monitor.rect.top;
        newHeight = firstIconHeight;
        std::wstring menuText = L"Screen " + std::to_wstring(screenCount + 1) + L" (" + 
                                std::to_wstring(width) + L"x" + 
                                std::to_wstring(height) + L")";

        if (monitor.rect == largestMonitor.rect) {
            newHeight = widthSecondLargestMonitor != 0 ? static_cast<int>(newHeight * widthLargestMonitor / widthSecondLargestMonitor) : 1024;
        }

        // Prüfen, ob HBITMAP bereits vorhanden ist oder neu erzeugt werden soll
        if (forceRecreate || hBitmapCache.find(screenCount) == hBitmapCache.end()) {
            hBitmapCache[screenCount] = CaptureAndResizeScreen(NULL, monitor.rect, newWidth, newHeight);
        }
    }
}

void CreateArrangeOnScreenMenu(HMENU hMenu, bool forceRecreate = true) {
    // Rufen Sie GetScreenCaptures auf, um die Bildschirmaufnahmen zu aktualisieren
    GetScreenCaptures(forceRecreate);

    HMENU hArrangeOnScreenMenu = CreateMenu();
    std::vector<MonitorInfo> monitors;
    EnumDisplayMonitors(NULL, NULL, MonitorEnumProc, reinterpret_cast<LPARAM>(&monitors));
    
    // Sortiere die Monitore nach ihrer linken Koordinate
    std::sort(monitors.begin(), monitors.end(), CompareMonitors);

    screenCount = -1;

    for (const auto& monitor : monitors) {
        screenCount++;
        int width = monitor.rect.right - monitor.rect.left;
        int height = monitor.rect.bottom - monitor.rect.top;
        int newHeight = firstIconHeight;
        std::wstring menuText = L"Screen " + std::to_wstring(screenCount + 1) + L" (" + 
                                std::to_wstring(width) + L"x" + 
                                std::to_wstring(height) + L")";

        // Verwenden Sie die HBITMAPs aus dem Cache
        HBITMAP hBitmap = hBitmapCache[screenCount];
        AddMenuItemWithImage(hArrangeOnScreenMenu, ID_ARRANGE + monitor.index, hBitmap, menuText);
    }
    AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hArrangeOnScreenMenu, L"Arra&nge Window(s)\tCTRL+N");
}

// Function to check if the windows have changed
bool HasWindowsChanged() {
    //std::cout << "HasWindowsChanged" << std::endl;
    // Retrieve the current open windows
    auto newWindows = getOpenWindows();
    
    // Check if the number of windows is different
    if (newWindows.size() != currentWindows.size()) {
        return true; // Number of windows has changed
    }
    
    // Check if any of the windows have changed
    for (size_t i = 0; i < newWindows.size(); ++i) {
        // Compare window handles, titles, and process names
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
    //std::cout << "MoveWindowToMainMonitor" << std::endl;
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
        mii.cch = sizeof(itemText[0]) != 0 ? sizeof(itemText) / sizeof(itemText[0]) : 1;
        GetMenuItemInfo(hMenu, i, TRUE, &mii);

        SIZE textSize;
        GetTextExtentPoint32(hdc, itemText, lstrlen(itemText), &textSize);
        totalWidth += static_cast<int>(textSize.cx * scaleFactor) + 25; // Füge 25 Pixel Puffer hinzu
    }

    ReleaseDC(hwnd, hdc);

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
    //std::cout << "caseInsensitiveCompare" << std::endl;
    std::wstring lowerA = a;
    std::wstring lowerB = b;
    std::transform(lowerA.begin(), lowerA.end(), lowerA.begin(), ::towlower);
    std::transform(lowerB.begin(), lowerB.end(), lowerB.begin(), ::towlower);
    return lowerA < lowerB;
}

int extractLastFiveDigits(int number) {
    if (number < 0) {
        return 0;
    }
    return number % 100000;
}

void SetEditPlaceholder(HWND hwndEdit, const std::wstring& placeholder) {
    //std::cout << "SetEditPlaceholder" << std::endl;
    SetWindowText(hwndEdit, placeholder.c_str());
    SendMessage(hwndEdit, EM_SETSEL, 0, -1); // Markiere den gesamten Text
    SendMessage(hwndEdit, EM_SETSEL, -1, -1); // Entferne die Markierung
}

// Function to allow on-mouse-over tooltip
HWND CreateTooltip(HWND hwndParent) {
    //std::cout << "CreateTooltip" << std::endl;
    HWND hwndTT = CreateWindowEx(WS_EX_TOPMOST, TOOLTIPS_CLASS, NULL,
        WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP,        
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        hwndParent, NULL, GetModuleHandle(NULL), NULL);

    SetWindowPos(hwndTT, HWND_TOPMOST, 0, 0, 0, 0,
        SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

    return hwndTT;
}

// Funktion zur Konvertierung von std::string in std::wstring
std::wstring stringToWString(const std::string& str) {
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
    std::wstring wstrTo(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
    return wstrTo;
}

// Function to update the window list
void UpdateWindowList(HWND hwnd) {
    //std::cout << "UpdateWindowList" << std::endl;
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

// Funktion, um die aktuelle Fensterhöhe zu erhalten
int GetWindowHeight(HWND hwnd) {
    RECT rect;
    GetWindowRect(hwnd, &rect);
    return rect.bottom - rect.top;
}

void AnimateWindowResize(HWND hwnd, int xPos, int yPos, int contentWidth, int newHeight) {
    const int previousHeight = GetWindowHeight(hwnd); // Aktuelle Fensterhöhe ermitteln
    const int minHeight = 100;
    newHeight = std::max(newHeight, minHeight);

    const int steps = 10; // Anzahl der Schritte für die Animation
    const int delay = 5; // Verzögerung in Millisekunden zwischen den Schritten (10 Schritte * 5 ms = 50 ms)
    const int heightIncrement = (newHeight - previousHeight) / steps;

    // Ursprünglichen Fokus speichern
    HWND focusedWindow = GetFocus();

    for (int i = 0; i < steps; ++i) {
        int currentHeight = previousHeight + heightIncrement * i;
        SetWindowPos(hwnd, NULL, xPos, yPos, contentWidth, currentHeight, SWP_NOZORDER);
        // Ursprünglichen Fokus wiederherstellen
        SetFocus(focusedWindow);
        SetFocus(focusedWindow);
        std::this_thread::sleep_for(std::chrono::milliseconds(delay));
    }

    // Setzen Sie die endgültige Größe, um Rundungsfehler zu vermeiden
    SetWindowPos(hwnd, NULL, xPos, yPos, contentWidth, newHeight, SWP_NOZORDER);

    // Ursprünglichen Fokus wiederherstellen
    SetFocus(focusedWindow);
}

// Function to adjust the window size
void AdjustWindowSize(HWND hwnd) {
    //std::cout << "AdjustWindowSize" << std::endl;
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
    int usableScreenHeight = screenHeight - titleBarHeight - 25 - getTaskbarHeight(hwnd) - heightToolbar; // Calculate the usable screen height (minus 50px to not be straight up to the bottom of the screen)
    int contentHeight = si.nMax + 30 + 40; // Calculate the content height
    int contentWidth = textWidth; // Set the content width
    int newHeight = std::max(200 + heightToolbar, std::min(contentHeight, usableScreenHeight)); // Calculate the new window height

    // Calculate the new y-position
    //int newYPos = yPos - (newHeight - height);

    if (yPos + newHeight > screenHeight - getTaskbarHeight(hwnd)) { // Überprüfen Sie, ob das Fenster am unteren Rand des Bildschirms abgeschnitten wird
        /*newYPos = screenHeight - getTaskbarHeight(hwnd) - newHeight; // Verschieben Sie das Fenster nach oben, um es auf den Bildschirm zu passen
        if (newYPos < 0) {
            newYPos = 0; // Stellen Sie sicher, dass das Fenster nicht über den oberen Rand des Bildschirms hinaus verschoben wird
            newHeight = screenHeight - titleBarHeight - 25 - getTaskbarHeight(hwnd); // Passen Sie die Höhe an, um auf den Bildschirm zu passen
        }
        // Speichern Sie die neue Y-Koordinate als Standard
        defaultYPos = newYPos;*/
        newHeight = screenHeight - yPos - getTaskbarHeight(hwnd);
    }
    //std::cout << previousHeight << " " << newHeight << std::endl;
    if (previousHeight != newHeight) {
        AnimateWindowResize(hwnd, xPos, yPos, contentWidth, newHeight);
        previousHeight = newHeight;
    }

    // Verwenden Sie die Standard-Y-Koordinate, wenn sie gesetzt ist
    /*if (defaultYPos != -1) {
        newYPos = defaultYPos;
    } else {*/
        //newYPos = yPos; // Verwenden Sie die ursprüngliche Y-Koordinate, wenn keine Standard-Y-Koordinate gesetzt ist
    //}

    //SetWindowPos(hwnd, NULL, xPos, newYPos, contentWidth, newHeight, SWP_NOZORDER); // Setzen Sie die neue Fensterposition und -größe
}

// Wrapper-Funktion für CreateArrangeOnScreenMenu
DWORD WINAPI ThreadFuncCreateSelectMenu(LPVOID lpParam) {
    HMENU hMenu = (HMENU)lpParam;
    CreateSelectScreenMenu(hMenu, false);
    return 0;
}

// Wrapper-Funktion für CreateMoveToScreenMenu
DWORD WINAPI ThreadFuncCreateMoveMenu(LPVOID lpParam) {
    HMENU hMenu = (HMENU)lpParam;
    CreateMoveToScreenMenu(hMenu);
    return 0;
}

DWORD WINAPI ThreadFuncCreateArrangeMenu(LPVOID lpParam) {
    HMENU hMenu = (HMENU)lpParam;
    CreateArrangeOnScreenMenu(hMenu);
    return 0;
}

void ExecuteCreateArrangeAndMoveMenuInThreads(HMENU hMenu, HMENU hMenuSelect) {
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

    // Erstelle den dritten Thread
    HANDLE hThread3 = CreateThread(
        NULL,       // Standard-Sicherheitsattribute
        0,          // Standard-Stackgröße
        ThreadFuncCreateSelectMenu, // Thread-Funktion
        hMenuSelect,      // Parameter für die Thread-Funktion
        0,          // Standard-Erstellungsflags
        NULL        // Thread-ID (optional)
    );

    // Warte, bis der dritte Thread beendet ist
    WaitForSingleObject(hThread3, INFINITE);

    // Schließe das Handle des dritten Threads
    CloseHandle(hThread3);
}



HMENU CreateSelectScreenMenu(HMENU hMenu, bool forceRecreate = false) {

    GetScreenCaptures(forceRecreate);

    HMENU hSelectScreenMenu = CreateMenu();
    std::vector<MonitorInfo> monitors;
    EnumDisplayMonitors(NULL, NULL, MonitorEnumProc, reinterpret_cast<LPARAM>(&monitors));

    // Sortiere die Monitore nach ihrer linken Koordinate
    std::sort(monitors.begin(), monitors.end(), CompareMonitors);

    if (monitors.size() > 1) { // Check if more than one screen is present
        screenCount = -1;

        for (const auto& monitor : monitors) {
            screenCount++;
            int width = monitor.rect.right - monitor.rect.left;
            int height = monitor.rect.bottom - monitor.rect.top;
            int newHeight = firstIconHeight;
            std::wstring menuText = L"Screen " + std::to_wstring(screenCount + 1) + L" (" + 
                                    std::to_wstring(width) + L"x" + 
                                    std::to_wstring(height) + L")";

            if (monitor.rect == largestMonitor.rect) {
                newHeight = widthSecondLargestMonitor != 0 ? static_cast<int>(newHeight * widthLargestMonitor / widthSecondLargestMonitor) : 1024;
            }

            // Verwenden Sie die HBITMAPs aus dem Cache
            HBITMAP hBitmap = hBitmapCache[screenCount];
            AddMenuItemWithImage(hSelectScreenMenu, ID_SELECT_SCREEN_BASE + monitor.index, hBitmap, menuText);
        }
        AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hSelectScreenMenu, L"Select Windows on Screen");
    }
    return hSelectScreenMenu;
}

void InitializeMenu(HWND hwnd) {
    HMENU hMenu = CreateMenu();
    HMENU hSelectMenu = CreateMenu();
    HMENU hFileMenu = CreateMenu();
    AppendMenu(hFileMenu, MF_STRING, ID_REFRESH, L"Re&fresh List\tCTRL+SHIFT+M");
    AppendMenu(hFileMenu, MF_STRING, ID_CLOSE_PROGRAMM, L"&Close Program to Tray\tCTRL+W");
    AppendMenu(hFileMenu, MF_STRING, ID_EXIT_BUTTON, L"E&xit Program\tCTRL+Q");
    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hFileMenu, L"&MoW");

    // Size menu
    HMENU hSizeMenu = CreateMenu();
    AppendMenu(hSizeMenu, MF_STRING, ID_MINIMIZE, L"M&inimize Window(s)\tCTRL+I");
    AppendMenu(hSizeMenu, MF_STRING, ID_MAXIMIZE, L"M&aximize Window(s)\tCTRL+X");
    AppendMenu(hSizeMenu, MF_STRING, ID_RESTORE, L"&Restore Window(s)\tCTRL+R");
    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hSizeMenu, L"Minimize& / Maximize / Restore");

    // Arrange menu
    HMENU hArrangeMenu = CreateMenu();
    ExecuteCreateArrangeAndMoveMenuInThreads(hArrangeMenu, hSelectMenu);
    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hArrangeMenu, L"&Arrange / Move Window(s)");

    // Select menu
    //HMENU hSelectMenu = CreateMenu();
    AppendMenu(hSelectMenu, MF_STRING, ID_SELECT_ALL, L"&Select All Window(s)\tCTRL+S");
    AppendMenu(hSelectMenu, MF_STRING, ID_SELECT_NONE, L"&Un-select All Window(s)\tCTRL+U");
    //HMENU hSelectScreenMenu = CreateSelectScreenMenu(hSelectMenu);
    //AppendMenu(hSelectMenu, MF_POPUP, (UINT_PTR)hSelectScreenMenu, L"Select Screen(s)");
    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hSelectMenu, L"Select / Un&-Select");

    // Close menu
    //HMENU hCloseMenu = CreateMenu();
    //AppendMenu(hCloseMenu, MF_STRING, ID_CLOSE, L"&Close Application And it's Window(s)");
    AppendMenu(hMenu, MF_STRING, ID_SAVEANDCLOSE, L"Sa&ve And Close Window(s)");
    //AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hCloseMenu, L"&Close / Save Window(s)");

    AppendMenu(hMenu, MF_STRING, ID_ABOUT, L"A&bout");

    SetMenu(hwnd, hMenu);
}

void UpdateDynamicMenus(HWND hwnd) {
    //std::cout << "UpdateDynamicMenus" << std::endl;
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
    //std::cout << "InvalidateWindow" << std::endl;
    /*if (!isRedrawPending) {
        isRedrawPending = true;
        InvalidateWindow(hwnd);
    }*/
   InvalidateRect(hwnd, NULL, TRUE);
}

void RefreshWindowList(HWND hwnd) {
    //std::cout << "RefreshWindowList" << std::endl;
    // Invalidate and redraw the window immediately
 
    //RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE); // Invalidate and redraw
    //ProcessMessages(); // Process messages
    UpdateWindowList(hwnd); // Update the window list
}

bool compareWindowsByName(const WindowInfo& a, const WindowInfo& b) {
    //std::cout << "compareWindowsByName" << std::endl;
    std::wstring nameA = a.title;
    std::wstring nameB = b.title;
    std::transform(nameA.begin(), nameA.end(), nameA.begin(), ::tolower);
    std::transform(nameB.begin(), nameB.end(), nameB.begin(), ::tolower);
    return nameA < nameB; // Sort windows by window name case-insensitive
}

// Function to confirm closing windows
bool ConfirmClose(HWND parentHwnd) {
    //std::cout << "ConfirmClose" << std::endl;
    // Bringe das Parent-Fenster in den Vordergrund
    SetForegroundWindow(parentHwnd);

    // Zeige das Bestätigungsdialogfeld an
    int result = ShowMessageBoxAndHandleTimers(parentHwnd, L"Do you really want to close the selected windows?", L"Confirmation", MB_YESNO | MB_ICONQUESTION | MB_TOPMOST, timers);
    //int result = MessageBoxW(parentHwnd, L"Do you really want to close the selected windows?", L"Confirmation", MB_YESNO | MB_ICONQUESTION | MB_TOPMOST);

    // Gib zurück, ob der Benutzer "Ja" gewählt hat
    return (result == IDYES);
}

void SearchAndCheck(const std::wstring& searchString, HWND hwnd) {
    //std::cout << "SearchAndCheck" << std::endl;
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
                    //std::cout << window.visible << std::endl;
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
    //AdjustWindowSize(hwnd); // Adjust the window size
    InvalidateWindow(hwnd); // Invalidate and redraw the window
    //InvalidateRect(hwnd, NULL, TRUE);
}

void simulateWindowsKeyPress() {
    //std::cout << "simulateWindowsKeyPress" << std::endl;
    INPUT inputs[2] = {};

    inputs[0].type = INPUT_KEYBOARD;
    inputs[0].ki.wVk = VK_LWIN;

    inputs[1].type = INPUT_KEYBOARD;
    inputs[1].ki.wVk = VK_LWIN;
    inputs[1].ki.dwFlags = KEYEVENTF_KEYUP;

    SendInput(2, inputs, sizeof(INPUT));
}

void simulateTextInput(const std::wstring& text) {
    //std::cout << "simulateTextInput" << std::endl;
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
    //std::cout << "SearchAndCheckErase" << std::endl;
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
    //std::cout << "RegisterClass: " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << "us\n";
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
    //std::cout << "Calculate dimensions: " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << "us\n";
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
    //std::cout << "CreateWindowEx: " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << "us\n";
    start = std::chrono::high_resolution_clock::now();

    // Set WS_EX_COMPOSITED style for double buffering
    SetWindowLongPtr(hwnd, GWL_EXSTYLE, GetWindowLongPtr(hwnd, GWL_EXSTYLE) | WS_EX_COMPOSITED);

    SetWindowPos(hwnd, HWND_TOPMOST, pt.x, pt.y, width + 10, height, SWP_SHOWWINDOW);

    end = std::chrono::high_resolution_clock::now();
    //std::cout << "SetWindowLongPtr and SetWindowPos: " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << "us\n";

    // Perform additional setup in a separate thread
    std::thread([hwnd, width, height]() {
        auto start = std::chrono::high_resolution_clock::now();

        // Abgerundete Ecken hinzufügen
        HRGN hRgn = CreateRoundRectRgn(0, 0, width, height, 20, 20); // 20 ist der Radius der abgerundeten Ecken
        SetWindowRgn(hwnd, hRgn, TRUE);

        auto end = std::chrono::high_resolution_clock::now();
        //std::cout << "CreateRoundRectRgn and SetWindowRgn: " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << "us\n";
        start = std::chrono::high_resolution_clock::now();

        // Schwarzen Rahmen hinzufügen
        SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 255, LWA_COLORKEY);

        end = std::chrono::high_resolution_clock::now();
        //std::cout << "SetLayeredWindowAttributes: " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << "us\n";
        start = std::chrono::high_resolution_clock::now();

        // Schatten hinzufügen
        MARGINS margins = {3, 3, 3, 3}; // Schattenbreite auf 10 Pixel einstellen
        DwmExtendFrameIntoClientArea(hwnd, &margins);

        end = std::chrono::high_resolution_clock::now();
        //std::cout << "DwmExtendFrameIntoClientArea: " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << "us\n";
        start = std::chrono::high_resolution_clock::now();

        // Blur-Effekt hinzufügen
        DWM_BLURBEHIND bb = {};
        bb.dwFlags = DWM_BB_ENABLE | DWM_BB_BLURREGION;
        bb.fEnable = TRUE;
        bb.hRgnBlur = hRgn;
        DwmEnableBlurBehindWindow(hwnd, &bb);

        end = std::chrono::high_resolution_clock::now();
        //std::cout << "DwmEnableBlurBehindWindow: " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << "us\n";

        SetTimer(hwnd, TRAY_CONTEXTMENU_TIMER_ID, TRAY_CONTEXTMENU_TIMER_ID_time, NULL);

        end = std::chrono::high_resolution_clock::now();
        //std::cout << "SetTimer: " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << "us\n";

        // Only set foreground and focus if necessary
        if (GetForegroundWindow() != hwnd) {
            SetForegroundWindow(hwnd);
            SetFocus(hwnd);
        }

        end = std::chrono::high_resolution_clock::now();
        //std::cout << "SetForegroundWindow and SetFocus: " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << "us\n";
    }).detach();
}

void PrintScrollInfo(HWND hwnd) {
    SCROLLINFO si;
    si.cbSize = sizeof(SCROLLINFO);
    si.fMask = SIF_ALL; // Retrieve all scroll bar parameters

    if (GetScrollInfo(hwnd, SB_VERT, &si)) {
        // Now si contains all the scrollbar information
        int minPos = si.nMin;
        int maxPos = si.nMax;
        int pageSize = si.nPage;
        int scrollPos = si.nPos;
        int trackPos = si.nTrackPos;

        // Print the scrollbar information using //std::cout
        //std::cout << "Min: " << minPos << ", Max: " << maxPos
        //std::cout << ", Page Size: " << pageSize << ", Scroll Pos: " << scrollPos
        //std::cout << ", Track Pos: " << trackPos << std::endl;
    } else {
        // Handle error
        std::cerr << "Failed to get scroll info" << std::endl;
    }
}

void SendScrollMessage(HWND hwnd, int scrollCode, int pos) {
    SendMessage(hwnd, WM_VSCROLL, MAKELONG(scrollCode, pos), (LPARAM)NULL);
}

void setScrollToNewPos(HWND hwnd, bool up_down)
{
    if (up_down) 
        SendScrollMessage(hwnd, SB_PAGEDOWN, 0);
    else
        SendScrollMessage(hwnd, SB_PAGEUP, 0);
}

// Toolbar erstellen
HWND CreateSimpleToolbar(HWND hwndParent, HINSTANCE hInstance) {
    //MessageBox(hwndParent, L"start", L"Error", MB_OK | MB_ICONERROR);
    HWND focusedWindow = GetFocus(); // Ursprünglichen Fokus speichern
    LoadIcons(hInstance);

    if (!hImageList) {
        //MessageBox(hwndParent, L"Failed to create image list!", L"Error", MB_OK | MB_ICONERROR);
        return NULL;
    }

    if (!hIconMinimize || !hIconArrange || !hIconClose || !hIconAbout || !hIconExit || !hIconMaximize || !hIconMove || !hIconRefresh || !hIconRestore || !hIconSave || !hIconSelectAll || !hIconUnselectAll || !hIconSelectOnScreen) {
        //MessageBox(hwndParent, L"Failed to load one or more icons!", L"Error", MB_OK | MB_ICONERROR);
        return NULL;
    }

    TBBUTTON tbButtons[] = {
        { MAKELONG(0, 0), IDI_ICON_EXIT, TBSTATE_ENABLED, BTNS_BUTTON, {0}, 0, (INT_PTR)L"" },
        { MAKELONG(1, 0), IDI_ICON_REFRESH, TBSTATE_ENABLED, BTNS_BUTTON, {0}, 0, (INT_PTR)L"" },
        { MAKELONG(2, 0), IDI_ICON_MINIMIZE, TBSTATE_ENABLED, BTNS_BUTTON, {0}, 0, (INT_PTR)L"" },
        { MAKELONG(3, 0), IDI_ICON_MAXIMIZE, TBSTATE_ENABLED, BTNS_BUTTON, {0}, 0, (INT_PTR)L"" },
        { MAKELONG(4, 0), IDI_ICON_RESTORE, TBSTATE_ENABLED, BTNS_BUTTON, {0}, 0, (INT_PTR)L"" },
        { MAKELONG(5, 0), IDI_ICON_ARRANGE, TBSTATE_ENABLED, BTNS_BUTTON, {0}, 0, (INT_PTR)L"" },
        { MAKELONG(6, 0), IDI_ICON_MOVE, TBSTATE_ENABLED, BTNS_BUTTON, {0}, 0, (INT_PTR)L"" },
        { MAKELONG(7, 0), IDI_ICON_SELECTALL, TBSTATE_ENABLED, BTNS_BUTTON, {0}, 0, (INT_PTR)L"" },
        { MAKELONG(8, 0), IDI_ICON_UNSELECTALL, TBSTATE_ENABLED, BTNS_BUTTON, {0}, 0, (INT_PTR)L"" },
        { MAKELONG(9, 0), IDI_ICON_SELECTONSCREEN, TBSTATE_ENABLED, BTNS_BUTTON, {0}, 0, (INT_PTR)L"" },
        { MAKELONG(10, 0), IDI_ICON_SAVE, TBSTATE_ENABLED, BTNS_BUTTON, {0}, 0, (INT_PTR)L"" },
        //{ MAKELONG(11, 0), IDI_ICON_CLOSE, TBSTATE_ENABLED, BTNS_BUTTON, {0}, 0, (INT_PTR)L"" },
        { MAKELONG(11, 0), IDI_ICON_ABOUT, TBSTATE_ENABLED, BTNS_BUTTON, {0}, 0, (INT_PTR)L"" }
    };

    hwndToolbar = CreateToolbarEx(
        hwndParent,
        WS_CHILD | WS_VISIBLE | TBSTYLE_TOOLTIPS,
        1,
        12, // Anzahl der Bilder in der Image List
        NULL,
        0,
        tbButtons,
        ARRAYSIZE(tbButtons),
        16, 16, 16, 16,
        sizeof(TBBUTTON)
    );

    if (!hwndToolbar) {
        //MessageBox(hwndParent, L"Failed to create toolbar!", L"Error", MB_OK | MB_ICONERROR);
        return NULL;
    }

    SendMessage(hwndToolbar, TB_SETIMAGELIST, 0, (LPARAM)hImageList);

    // Text für alle Buttons deaktivieren
    /*for (int i = 0; i < ARRAYSIZE(tbButtons); ++i) {
        TBBUTTONINFO tbInfo = { 0 };
        tbInfo.cbSize = sizeof(TBBUTTONINFO);
        tbInfo.dwMask = TBIF_TEXT;
        tbInfo.pszText = L"";
        SendMessage(hwndToolbar, TB_SETBUTTONINFO, tbButtons[i].idCommand, (LPARAM)&tbInfo);
    }*/

    SetFocus(focusedWindow);
    return hwndToolbar;
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
            HICON hIconMinimize_contextmenu = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(IDI_ICON_MINIMIZE_contextmenu), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR | LR_LOADTRANSPARENT);
            HICON hIconArrange_contextmenu = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(IDI_ICON_ARRANGE_contextmenu), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR | LR_LOADTRANSPARENT);
            HICON hIconClose_contextmenu = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(IDI_ICON_CLOSE_contextmenu), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR | LR_LOADTRANSPARENT);

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

                SendMessage(hImage1, STM_SETIMAGE, IMAGE_ICON, (LPARAM)hIconMinimize_contextmenu);
                SendMessage(hImage2, STM_SETIMAGE, IMAGE_ICON, (LPARAM)hIconArrange_contextmenu);
                SendMessage(hImage3, STM_SETIMAGE, IMAGE_ICON, (LPARAM)hIconClose_contextmenu);

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

            //HICON hIconExit = (HICON)LoadImage(GetModuleHandle(L"SHELL32.DLL"), MAKEINTRESOURCE(16805), IMAGE_ICON, 32, 32, LR_DEFAULTCOLOR);
            HICON hIconExit = (HICON)LoadImage(GetModuleHandle(L"SHELL32.DLL"), MAKEINTRESOURCE(221), IMAGE_ICON, 32, 32, LR_DEFAULTCOLOR);
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
          
            //hwndToolbar = CreateSimpleToolbar(hwnd);

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
                    RECT lineRect = {0, yPos - 10, rect.right, yPos - 10 + 30};
                    FillRect(memDC, &lineRect, (HBRUSH)GetStockObject(LTGRAY_BRUSH));
                }

                auto it = processIcons.find(processName);
                if (it != processIcons.end())
                {
                    DrawIconEx(memDC, 10, yPos - 5, it->second, 20, 20, 0, NULL, DI_NORMAL);
                }

                SetBkMode(memDC, TRANSPARENT);
                TextOut(memDC, 40, yPos - 5, processNameW.c_str(), processNameW.length());

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
                int result = MessageBox(NULL, L"Are you sure you want to exit the 'Many Open Windows' program?", L"Exit Confirmation", MB_YESNO | MB_ICONQUESTION  | MB_TOPMOST | MB_DEFBUTTON2);
                //int result = ShowMessageBoxAndHandleTimers(NULL, L"Are you sure you want to exit the 'Many Open Windows' program?", L"Exit Confirmation", MB_YESNO | MB_ICONQUESTION  | MB_TOPMOST, timers);

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
                                //SetWindowPos(window.hwnd, NULL, 0, 0, screenWidth, screenHeight, SWP_NOZORDER);
                                window.arranged = false;
                                ShowWindow(window.hwnd, SW_MINIMIZE);
                                ProcessMessages();
                            }
                        }
                    }              
                }
            if (imageNumber == 2)  // arrange via context menu of tray icon
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
                //std::cout << "screenWidth: " << screenWidth << std::endl;

                int numWindows = 0;
                for (auto& entry : processWindowsMap) {
                    for (auto& window : entry.second) {
                        if (window.processName == processNames[index]) {
                            window.arranged = true; 
                            numWindows++;
                        }
                    }
                }
                if (numWindows == 0) return 0;
        //std::cout << "numWindows: " << numWindows << std::endl;
                int cols = static_cast<int>(ceil(sqrt(numWindows))); // Calculate the number of columns
                int rows = cols > 0 ? (numWindows + cols - 1) / cols : 1; // Calculate the number of rows
        //std::cout << "start0: " << std::endl;
        //std::cout << "screenWidth: " << screenWidth << std::endl;
        //std::cout << "cols: " << cols << std::endl;
        //std::cout << "screenHeight: " << screenHeight << std::endl;
        //std::cout << "rows: " << rows << std::endl;
                int windowWidth = cols > 0 ? (screenWidth) / cols : 1; // Calculate the window width
                int windowHeight = rows > 0 ? (screenHeight) / rows : 1; // Calculate the window height
        //std::cout << "start0a: " << std::endl;
                int x = 0, y = 0; // Initialize the X and Y positions
                int maxAdjustedWidth = 0;
                int minAdjustedWidth = screenWidth;
                numWindows = 0;
                //std::cout << "start: " << std::endl;
                // Zuerst die Breite des breitesten Fensters ermitteln
                for (const auto& entry : processWindowsMap) { // Iterate through all processes
                    //std::cout << "start1: " << std::endl;
                    for (const auto& window : entry.second) { // Iterate through all windows of a process
                        //std::cout << "start2: " << std::endl;
                        if (window.processName == processNames[index]) { // Check if the window is selected
                            //std::cout << "start3: " << std::endl;
                            if (IsIconic(window.hwnd) || IsZoomed(window.hwnd)) ShowWindow(window.hwnd, SW_RESTORE); // Restore the window if minimized or maximized
                            //SetWindowPos(window.hwnd, NULL, 0, 0, screenWidth, screenHeight, SWP_NOZORDER);
                            MoveWindowToScreen(window.hwnd, screenIndex);
                            ProcessMessages(); // Process messages
                            //ShowWindow(window.hwnd, SW_MINIMIZE); // Minimize the window
                            if (IsIconic(window.hwnd) || IsZoomed(window.hwnd)) ShowWindow(window.hwnd, SW_RESTORE); // Restore the window
                            SetForegroundWindow(window.hwnd); // Bring the window to the foreground
                            ProcessMessages(); // Process messages
                            Sleep(100);
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
                //std::cout << "minAdjustedWidth: " << minAdjustedWidth << std::endl;
                //std::cout << "cols: " << cols << std::endl;
                // Anzahl der Fenster nebeneinander und die Anzahl der Fensterreihen berechnen
                cols = screenWidth / std::max(std::max(maxAdjustedWidth, screenWidth/cols), 1);
                //std::cout << "cols: " << cols << std::endl;                
                rows = cols > 0 ? (numWindows + cols - 1) / cols : 1;
                windowWidth = cols > 0 ? screenWidth / cols : 1; // Calculate the window width
                windowHeight = rows > 0 ? (screenHeight) / rows : 1; // Calculate the window height

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

            if (imageNumber == 3) // save & close via context menu of tray icon
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

    int searchBoxX = clientRect.right - searchBoxWidth - 1 * buttonWidth - 2 * padding;
    int searchBoxY = 5; // Position Y des Suchfelds

    SetWindowPos(hSearchBox, NULL, searchBoxX, searchBoxY, searchBoxWidth, searchBoxHeight, SWP_NOZORDER);
    SetWindowPos(hEraseButton, NULL, searchBoxX - 15 - padding, searchBoxY, buttonWidth, buttonHeight, SWP_NOZORDER);
    SetWindowPos(hGoToButton, NULL, searchBoxX + searchBoxWidth + 0 * buttonWidth + 1 * padding, searchBoxY, buttonWidth, buttonHeight, SWP_NOZORDER);

    InvalidateRect(hSearchBox, NULL, TRUE);
    UpdateWindow(hSearchBox);
    InvalidateRect(hEraseButton, NULL, TRUE);
    UpdateWindow(hEraseButton);
    InvalidateRect(hGoToButton, NULL, TRUE);
    UpdateWindow(hGoToButton);
}

// Window procedure
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    //std::cout << "WindowProc" << std::endl;
    static int scrollPos = 0; // Static variable to store the scroll position
    int id; // Variable to store the ID
    static HIMAGELIST hImageList;
    static HICON hIcon1;
    static HMENU hMenu = NULL;

    //std::cout << "uMsg:" << uMsg << std::endl;

    switch (uMsg) { // Check the messages
        case WM_CREATE: {
            //std::cout << "WM_CREATE" << std::endl;
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
            hEraseButton = CreateWindowEx(0, TEXT("BUTTON"), L"\u2716", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_FLAT, 
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
            hwndToolbar = CreateSimpleToolbar(hwnd, ((LPCREATESTRUCT)lParam)->hInstance);
            if (!hwndToolbar) {
                //MessageBox(hwnd, L"Failed to create toolbar!", L"Error", MB_OK | MB_ICONERROR);
                //return -1; // Fenstererstellung abbrechen
            }
            else
            {
                //MessageBox(hwnd, L"success to create toolbar!", L"Error", MB_OK | MB_ICONERROR);
            }
        }
        SetTimer(hwnd, BLINKING_TIMER_ID, 250, NULL);
        //InvalidateWindow(hwnd);
        break;

        case WM_KEYDOWN: {
            //std::cout << "WM_KEYDOWN" << std::endl;
            if (windowReady)
            {
                BYTE keyboardState[256];
                GetKeyboardState(keyboardState);
                WCHAR buffer[2];
                wchar_t ch = 0;
                if (ToUnicode(wParam, HIWORD(lParam) & 0xFF, keyboardState, buffer, 2, 0) == 1) {
                    ch = buffer[0];
                }
                switch (wParam)
                {
                case VK_UP:
                {
                    if (highlightedRow > 0 && highlightedRow <= processNames.size() &&
                        !expandedState.at(processNames.at(highlightedRow - 1)) &&
                        highlightedWindowRow == -1)
                    {
                        highlightedRow--;
                    }
                    else if (highlightedRow == 0 &&
                        !expandedState.at(processNames.at(highlightedRow)) &&
                        highlightedWindowRow == -1)
                    {
                        highlightedRow = 0;
                    }
                    else if (highlightedRow > 0 && highlightedRow <= processNames.size() &&
                        expandedState.at(processNames.at(highlightedRow - 1)) &&
                        highlightedWindowRow == -1)
                    {
                        highlightedWindowRow = (highlightedRow - 1) * 100000 + (processWindowsMap.at(processNames.at(highlightedRow - 1)).size() - 1);
                        for (auto counter = processWindowsMap.at(processNames.at(highlightedRow - 1)).size() - 1; counter >= 0; counter--)
                        {
                            if (!processWindowsMap.at(processNames.at(highlightedRow - 1)).at(counter).visible && !processWindowsMap.at(processNames.at(highlightedRow - 1)).at(counter).checked)
                                highlightedWindowRow--;
                            else
                                break;
                        }
                        highlightedRow--;
                    }
                    else if (highlightedRow == 0 &&
                        highlightedWindowRow == -1)
                    {
                        highlightedRow = 0;
                    }
                    else if (highlightedRow == 0 && highlightedRow <= processNames.size() && expandedState.at(processNames.at(highlightedRow)) &&
                        processWindowsMap.at(processNames.at(highlightedRow)).size() > 1 &&
                        highlightedWindowRow >= highlightedRow * 100000)
                    {
                        int currentCounter = extractLastFiveDigits(highlightedWindowRow);
                        //std::cout << currentCounter << " " << highlightedWindowRow << std::endl;
                        auto& windows = processWindowsMap.at(processNames.at(highlightedRow));
                        int counter;
                        for (counter = currentCounter - 1; counter >= 0; counter--)
                        {
                            auto& window = windows.at(counter);
                            //std::cout << counter << " " << window.visible << " " << window.checked << std::endl;
                            if (!window.visible && !window.checked)
                                highlightedWindowRow--;
                            else
                            {
                                break;
                            }
                        }
                        if (counter < 0)
                        {
                            highlightedWindowRow = -1;
                        }
                        else
                            highlightedWindowRow--;
                        //std::cout << highlightedWindowRow << std::endl;
                    }
                    else if (highlightedRow > 0 && highlightedRow <= processNames.size() && expandedState.at(processNames.at(highlightedRow)) &&
                        processWindowsMap.at(processNames.at(highlightedRow)).size() > 1 &&
                        highlightedWindowRow >= highlightedRow * 100000)
                    {
                        int currentCounter = extractLastFiveDigits(highlightedWindowRow);
                        //std::cout << currentCounter << " " << highlightedWindowRow << std::endl;
                        auto& windows = processWindowsMap.at(processNames.at(highlightedRow));
                        int counter;
                        for (counter = currentCounter - 1; counter >= 0; counter--)
                        {
                            auto& window = windows.at(counter);
                            //std::cout << counter << " " << window.visible << " " << window.checked << std::endl;
                            if (!window.visible && !window.checked)
                                highlightedWindowRow--;
                            else
                            {
                                break;
                            }
                        }
                        if (counter < 0)
                        {
                            highlightedWindowRow = -1;
                        }
                        else
                            highlightedWindowRow--;
                        //std::cout << highlightedWindowRow << std::endl;
                    }
                    else if (expandedState.at(processNames.at(highlightedRow)) &&
                        highlightedRow == 0 &&
                        processWindowsMap.at(processNames.at(highlightedRow)).size() > 1 &&
                        highlightedWindowRow == highlightedRow * 100000)
                    {
                        highlightedWindowRow = -1;
                    }
                    else if (highlightedRow > 0 && highlightedRow <= processNames.size() && expandedState.at(processNames.at(highlightedRow)) &&
                        processWindowsMap.at(processNames.at(highlightedRow)).size() > 1 &&
                        highlightedWindowRow == highlightedRow * 100000)
                    {
                        highlightedWindowRow = -1;
                    }
                    else if (highlightedRow == 0 && highlightedRow <= processNames.size() && expandedState.at(processNames.at(highlightedRow)) &&
                        processWindowsMap.at(processNames.at(highlightedRow)).size() <= 1)
                    {
                        highlightedWindowRow = -1;
                    }
                    else if (highlightedRow > 0 && highlightedRow <= processNames.size() && expandedState.at(processNames.at(highlightedRow)) &&
                        processWindowsMap.at(processNames.at(highlightedRow)).size() <= 1)
                    {
                        highlightedWindowRow = -1;
                    }
                    else if (highlightedRow > 0 && !expandedState.at(processNames.at(highlightedRow)))
                    {
                        highlightedWindowRow = -1;
                        highlightedRow--;
                    }
                    else if (highlightedRow == 0 && !expandedState.at(processNames.at(highlightedRow)))
                    {
                        highlightedWindowRow = -1;
                        highlightedRow = 0;
                    }
                    else
                    {
                        highlightedWindowRow = -1;
                        highlightedRow = 0;
                    }
                    if (highlightedWindowRow != -1)
                        if (highlightedRow < (processNames.size() - 3) + processWindowsMap.at(processNames.at(highlightedRow)).size())
                            setScrollToNewPos(hwnd, 0);
                        else if (highlightedRow < processNames.size() - 3)
                            setScrollToNewPos(hwnd, 0);
                }
                break;
                case VK_DOWN:
                {
                    if (highlightedRow >= 0 && highlightedRow < processNames.size() - 1 &&
                        !expandedState.at(processNames.at(highlightedRow)) &&
                        highlightedWindowRow == -1)
                    {
                        //std::cout << "Case 1: highlightedRow incremented from " << highlightedRow << " to " << (highlightedRow + 1) << std::endl;
                        highlightedRow++;
                    }
                    else if (highlightedRow == processNames.size() - 1 &&
                        !expandedState.at(processNames.at(highlightedRow)) &&
                        highlightedWindowRow == -1)
                    {
                        //std::cout << "Case 2: highlightedRow set to " << highlightedRow << std::endl;
                        highlightedRow = processNames.size() - 1;
                    }
                    else if (highlightedRow >= 0 && highlightedRow < processNames.size() - 1 &&
                        expandedState.at(processNames.at(highlightedRow)) &&
                        highlightedWindowRow == -1)
                    {
                        highlightedWindowRow = (highlightedRow) * 100000;
                        //std::cout << "Case 3: highlightedWindowRow set to " << highlightedWindowRow << std::endl;
                        for (auto counter = 0; counter < processWindowsMap.at(processNames.at(highlightedRow)).size(); counter++)
                        {
                            if (!processWindowsMap.at(processNames.at(highlightedRow)).at(counter).visible &&
                                !processWindowsMap.at(processNames.at(highlightedRow)).at(counter).checked)
                                highlightedWindowRow++;
                            else
                                break;
                        }
                    }
                    else if (highlightedRow == processNames.size() - 1 &&
                        highlightedWindowRow == -1)
                    {
                        //std::cout << "Case 4: highlightedRow set to " << highlightedRow << std::endl;
                        highlightedRow = processNames.size() - 1;
                    }
                    else if (highlightedRow >= 0 && highlightedRow < processNames.size() && expandedState.at(processNames.at(highlightedRow)) &&
                        processWindowsMap.at(processNames.at(highlightedRow)).size() > 1 &&
                        highlightedWindowRow >= highlightedRow * 100000)
                    {
                        int currentCounter = extractLastFiveDigits(highlightedWindowRow);
                        auto& windows = processWindowsMap.at(processNames.at(highlightedRow));
                        int counter;
                        for (counter = currentCounter + 1; counter < windows.size(); counter++)
                        {
                            auto& window = windows.at(counter);
                            //std::cout << counter << " " << window.visible << " " << window.checked << std::endl;
                            if (!window.visible && !window.checked)
                                highlightedWindowRow++;
                            else
                            {
                                break;
                            }
                        }
                        if (counter >= processWindowsMap.at(processNames.at(highlightedRow)).size())
                        {
                            highlightedWindowRow = -1;
                            highlightedRow++;
                            //std::cout << "Case 5: highlightedWindowRow set to -1" << std::endl;
                        }
                        else
                        {
                            highlightedWindowRow++;
                            //std::cout << "Case 5: highlightedWindowRow incremented to " << highlightedWindowRow << std::endl;
                        }
                    }
                    else if (expandedState.at(processNames.at(highlightedRow)) &&
                        highlightedRow == processNames.size() - 1 &&
                        processWindowsMap.at(processNames.at(highlightedRow)).size() > 1 &&
                        highlightedWindowRow == (highlightedRow + 1) * 100000)
                    {
                        highlightedWindowRow = -1;
                        //std::cout << "Case 6: highlightedWindowRow set to -1" << std::endl;
                    }
                    else if (highlightedRow >= 0 && highlightedRow < processNames.size() && expandedState.at(processNames.at(highlightedRow)) &&
                        processWindowsMap.at(processNames.at(highlightedRow)).size() > 1 &&
                        highlightedWindowRow == highlightedRow * 100000)
                    {
                        highlightedWindowRow = -1;
                        //std::cout << "Case 7: highlightedWindowRow set to -1" << std::endl;
                    }
                    else if (highlightedRow == processNames.size() - 1 && expandedState.at(processNames.at(highlightedRow)) &&
                        processWindowsMap.at(processNames.at(highlightedRow)).size() <= 1)
                    {
                        highlightedWindowRow = -1;
                        highlightedRow++;
                        //std::cout << "Case 8: highlightedWindowRow set to -1" << std::endl;
                    }
                    else if (highlightedRow >= 0 && highlightedRow < processNames.size() && expandedState.at(processNames.at(highlightedRow)) &&
                        processWindowsMap.at(processNames.at(highlightedRow)).size() <= 1)
                    {
                        highlightedWindowRow = -1;
                        highlightedRow++;
                        //std::cout << "Case 9: highlightedWindowRow set to -1" << std::endl;
                    }
                    else if (highlightedRow >= 0 && !expandedState.at(processNames.at(highlightedRow)))
                    {
                        highlightedWindowRow = -1;
                        //std::cout << "Case 10: highlightedWindowRow set to -1, highlightedRow incremented from " << highlightedRow << " to " << (highlightedRow + 1) << std::endl;
                        highlightedRow++;
                    }
                    else if (highlightedRow == processNames.size() - 1 && !expandedState.at(processNames.at(highlightedRow)))
                    {
                        highlightedWindowRow = -1;
                        //std::cout << "Case 11: highlightedWindowRow set to -1, highlightedRow set to " << highlightedRow << std::endl;
                        highlightedRow = processNames.size() - 1;
                    }
                    else
                    {
                        highlightedWindowRow = -1;
                        highlightedRow = 0;
                    }
                    if (highlightedWindowRow != -1)
                        if (highlightedRow + extractLastFiveDigits(highlightedWindowRow) > 5 && highlightedRow < (processNames.size() - 1) + processWindowsMap.at(processNames.at(highlightedRow)).size())
                            setScrollToNewPos(hwnd, 1);
                        else if (highlightedRow > 5 && highlightedRow < processNames.size() - 1)
                            setScrollToNewPos(hwnd, 1);
                }
                break;
                case VK_RETURN:
                {
                    if (highlightedWindowRow != -1)
                    {
                        std::wstring key = std::to_wstring(highlightedRow);
                        //std::wcout << L"Highlighted window row is not -1. Key: " << key << std::endl;
                        const auto& processName = processNames[highlightedRow];
                        checkboxState[processName] = !checkboxState[processName];
                        //std::wcout << L"Checkbox state for key " << key << L" is now " << checkboxState[processName] << std::endl;

                        std::wstring processKey = std::to_wstring(extractLastFiveDigits(highlightedWindowRow));
                        //std::wcout << L"Process key: " << processKey << std::endl;
                        int counter = 0;
                        for (auto& window : processWindowsMap[processName])
                        {//std::cout << "counter: " << counter << "  extractLastFiveDigits(highlightedWindowRow): " << extractLastFiveDigits(highlightedWindowRow) << std::endl;
                            if (counter == extractLastFiveDigits(highlightedWindowRow))
                            {//std::cout << "match" << std::endl;
                                window.checked = !window.checked;
                                window.visible = true;
                                //std::wcout << L"Set window.checked to " << checkboxState[processName] << L" for window in processWindowsMap with key " << key << std::endl;
                            }
                            counter++;
                        }
                    }
                    else
                    {
                        std::wstring key = std::to_wstring(highlightedRow);
                        //std::wcout << L"Highlighted window row is -1. Key: " << key << std::endl;
                        const auto& processName = processNames[highlightedRow];
                        checkboxState[processName] = !checkboxState[processName];
                        //std::wcout << L"Checkbox state for key " << key << L" is now " << checkboxState[processName] << std::endl;

                        for (auto& window : processWindowsMap[processName])
                        {
                            window.checked = checkboxState[processName];
                            window.visible = true;
                            //std::wcout << L"Set window.checked to " << checkboxState[processName] << L" for window in processWindowsMap with key " << key << std::endl;
                        }
                    }
                    /*int yPos = 0 - scrollPos;
                    HFONT hFont = CreateFont(24, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, TEXT("Segoe UI")); // Create a font
                    HDC hdc = GetDC(hwnd);                                                                                                                                                                // Retrieve the device context
                    HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);                                                                                                                                     // Select the new font and save the old font
                    for (size_t i = 0; i < processNames.size(); ++i)
                    {
                        const auto &processName = processNames[i];
                        RECT rect = {30, yPos, textWidth, yPos + 30};
                        if (i == highlightedRow)
                        {
                            checkboxState[processName] = !checkboxState[processName];
                            for (auto &window : processWindowsMap[processName])
                            {
                                window.checked = checkboxState[processName];
                            }
                            InvalidateRect(hwnd, NULL, TRUE);
                        }
                        rect = {0, yPos, 30, yPos + 30};
                        if (i == highlightedRow)
                        {
                            expandedState[processName] = !expandedState[processName];
                            for (size_t j = 0; j < processWindowsMap[processName].size(); ++j)
                            {                                                     // Iterate through all windows of the process
                                auto &window = processWindowsMap[processName][j]; // Retrieve the current window
                                window.visible = true;
                                yPos += 30;
                            }
                            InvalidateWindow(hwnd);
                        }
                        yPos += 30; // Increase the y-position

                        if (expandedState[processName])
                        { // Check if the process is expanded
                            for (size_t j = 0; j < processWindowsMap[processName].size(); ++j)
                            {                                                       // Iterate through all windows of the process
                                auto &window = processWindowsMap[processName][j];   // Retrieve the current window
                                RECT windowRect = {50, yPos, textWidth, yPos + 30}; // Define a rectangle for the window
                                if (i * 100000 + j == highlightedWindowRow && window.visible)
                                {                                     // Check if the cursor is in the rectangle
                                    window.checked = !window.checked; // Toggle the checkbox state for the window
                                    InvalidateRect(hwnd, NULL, TRUE); // Invalidate and redraw the window
                                }
                                if (window.visible || window.checked)
                                    yPos += 30; // Increase the y-position
                            }
                        }
                    }
                    SelectObject(hdc, hOldFont); // Restore the old font
                    DeleteObject(hFont);         // Delete the new font
                    ReleaseDC(hwnd, hdc);        // Release the device context*/
                    InvalidateWindow(hwnd);
                }
                break;
                case VK_RIGHT:
                {
                    std::wstring key = std::to_wstring(highlightedRow);
                    //std::wcout << L"Highlighted window row is -1. Key: " << key << std::endl;
                    const auto& processName = processNames[highlightedRow];
                    expandedState[processName] = 1;
                    //std::wcout << L"Checkbox state for key " << key << L" is now " << checkboxState[processName] << std::endl;
                    for (auto& window : processWindowsMap[processName])
                    {
                        window.visible = true;
                        //std::wcout << L"Set window.checked to " << checkboxState[processName] << L" for window in processWindowsMap with key " << key << std::endl;
                    }
                    InvalidateWindow(hwnd);
                }
                break;
                case VK_LEFT:
                {
                    std::wstring key = std::to_wstring(highlightedRow);
                    //std::wcout << L"Highlighted window row is -1. Key: " << key << std::endl;
                    const auto& processName = processNames[highlightedRow];
                    expandedState[processName] = 0;
                    //std::wcout << L"Checkbox state for key " << key << L" is now " << checkboxState[processName] << std::endl;
                    for (auto& window : processWindowsMap[processName])
                    {
                        window.visible = false;
                        //std::wcout << L"Set window.checked to " << checkboxState[processName] << L" for window in processWindowsMap with key " << key << std::endl;
                    }
                    InvalidateWindow(hwnd);
                }
                break;
                }

                if (((wParam == 'F') || (wParam == 'A')) && (GetKeyState(VK_CONTROL) & 0x8000))
                {
                    // Ctrl+F was pressed
                    SetFocus(hSearchBox);
                    SendMessage(hSearchBox, EM_SETSEL, 0, -1); // Select all text in the search box
                    return 0;
                    /*} else if (wParam == VK_ESCAPE) {
                        // ESC key was pressed
                        wParam = IDC_ERASEBUTTON;*/
                }
                /*else if ((wParam == 'X') && (GetKeyState(VK_CONTROL) & 0x8000)) {
                    // Handle the hotkey (CTRL+X)
                    SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(ID_CTRL_X, 0), 0);
                    /*SetWindowPos(hwnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
                    //int result = MessageBox(NULL, L"Are you sure you want to exit the 'Many Open Windows' program?", L"Exit Confirmation", MB_YESNO | MB_ICONQUESTION  | MB_TOPMOST);
                    int result = ShowMessageBoxAndHandleTimers(NULL, L"Are you sure you want to exit the 'Many Open Windows' program?", L"Exit Confirmation", MB_YESNO | MB_ICONQUESTION  | MB_TOPMOST, timers);

                    if (result == IDYES)
                    {
                        PostQuitMessage(0);
                    }
                    else
                    {
                        SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
                    }*/
                //}
                else if ((wParam == 'W') && (GetKeyState(VK_CONTROL) & 0x8000))
                {
                    // Handle the hotkey (CTRL+W) (close to tray)
                    SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(ID_CTRL_W, 0), 0);
                }
                else if ((wParam == 'I') && (GetKeyState(VK_CONTROL) & 0x8000))
                {
                    // Handle the hotkey (CTRL+I) (minimize windows)
                    SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(ID_CTRL_I, 0), 0);
                }
                else if ((wParam == 'X') && (GetKeyState(VK_CONTROL) & 0x8000))
                {
                    // Handle the hotkey (CTRL+X) (maximize windows)
                    SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(ID_CTRL_X, 0), 0);
                }
                else if (((wParam == 'A') && (GetKeyState(VK_CONTROL) & 0x8000)) ||
                    ((wParam == VK_HOME) && (GetKeyState(VK_SHIFT) & 0x8000)))
                {
                    // Handle the hotkey (CTRL+A or SHIFT+HOME) (erase search field)
                    SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(ID_CTRL_A, 0), 0);
                }
                else if ((wParam == 'Q') && (GetKeyState(VK_CONTROL) & 0x8000))
                {
                    // Handle the hotkey (CTRL+Q) (Quit = Exit)
                    SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(ID_CTRL_Q, 0), 0);
                }
                else if ((wParam == 'R') && (GetKeyState(VK_CONTROL) & 0x8000))
                {
                    // Handle the hotkey (CTRL+R) (restore windows)
                    SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(ID_CTRL_R, 0), 0);
                }
                else if ((wParam == 'N') && (GetKeyState(VK_CONTROL) & 0x8000))
                {
                    // Handle the hotkey (CTRL+N) (arrange windows)
                    SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(ID_CTRL_N, 0), 0);
                }
                else if ((wParam == 'M') && (GetKeyState(VK_CONTROL) & 0x8000))
                {
                    // Handle the hotkey (CTRL+M) (move windows)
                    SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(ID_CTRL_M, 0), 0);
                }
                else if ((wParam == 'S') && (GetKeyState(VK_CONTROL) & 0x8000))
                {
                    // Handle the hotkey (CTRL+S) (select all)
                    SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(ID_SELECT_ALL, 0), 0);
                }
                else if ((wParam == 'U') && (GetKeyState(VK_CONTROL) & 0x8000))
                {
                    // Handle the hotkey (CTRL+U) (un-select all)
                    SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(ID_SELECT_NONE, 0), 0);
                }
                else if ((wParam == 'M') && (GetKeyState(VK_CONTROL) & 0x8000) && (GetKeyState(VK_SHIFT) & 0x8000))
                {
                    // Handle the hotkey (CTRL+SHIFT+M) (refresh list)
                    SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(ID_REFRESH, 0), 0);
                }
                else if ((wParam != VK_UP && wParam != VK_DOWN && IsValidInput(&ch)) || wParam == VK_BACK || wParam == VK_TAB)
            {
                HWND hFocusedWindow = GetFocus();
                //std::cout << wParam << std::endl;
                /*if (hFocusedWindow == hEraseButton && wParam == VK_TAB) {
                    // If the focus is on the hEraseButton and the user presses Tab, set focus to hGoToButton
                    //std::cout << "hfocusedWindow "<< std::endl;
                    SetFocus(hGoToButton);
                } else*/
                /*if (wParam == VK_TAB) {
                    // If the focus is on the hSearchBox and the user presses Tab, set focus to hGoToButton
                    //std::cout << "hSearchBox "<< std::endl;
                    SetFocus(hGoToButton);
                }*/
                /*if (hFocusedWindow == hwnd && wParam == VK_TAB) {
                    // If the focus is on the hEraseButton and the user presses Tab, set focus to hGoToButton
                    SetFocus(hSearchBox);
                } else*/
                {
                    SetFocus(hSearchBox);
                    wchar_t text[256];
                    GetWindowText(hSearchBox, text, 256);
                    if (wcscmp(text, L"\u2315 Search Name (CTRL-F)") == 0)
                    {
                        SendMessage(hSearchBox, EM_SETSEL, 0, -1); // Select all text in the search box
                    }

                    if (wParam == VK_BACK)
                    {
                        // Handle backspace: delete the last character
                        int len = wcslen(text);
                        if (len > 0)
                        {
                            text[len - 1] = '\0';
                            SetWindowText(hSearchBox, text);
                            SendMessage(hSearchBox, EM_SETSEL, len - 1, len - 1); // Move the cursor to the end
                            return 0;
                        }
                    }
                    if (wParam == VK_TAB)
                    {
                        // Handle tab: set focus to the "X" button
                        SetFocus(hGoToButton);
                        return 0;
                    } /*else if (hFocusedWindow == hEraseButton && wParam == VK_RETURN) {
                        // Handle enter: click the erase button
                        SendMessage(hEraseButton, BM_CLICK, 0, 0);
                    } else */
                    {
                        BYTE keyboardState[256];
                        GetKeyboardState(keyboardState);
                        WCHAR charBuffer[2];
                        if (ToUnicode(wParam, MapVirtualKey(wParam, MAPVK_VK_TO_VSC), keyboardState, charBuffer, 2, 0) == 1)
                        {
                            // Check if the character is a hyphen (dash)
                            if (charBuffer[0] == L'-' || charBuffer[0] == L'\u2013' || charBuffer[0] == L'\u2014')
                            {
                                SendMessage(hSearchBox, WM_CHAR, L'-', 0); // Send the hyphen character
                            }
                            else
                            {
                                SendMessage(hSearchBox, WM_CHAR, charBuffer[0], 0);
                            }
                        }
                    }
                }
                SetFocus(hwnd);
                return 0;
            }
            }
        }
        break;

        case WM_TIMER: {
            //std::cout << "WM_KEYDOWN" << std::endl;
            //std::cout << "WM_TIMER" << std::endl;
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
            //std::cout << "WM_DRAWITEM" << std::endl;
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
            //std::cout << "WM_UPDATE_LIST" << std::endl;
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
            //std::cout << "WM_VSCROLL" << std::endl;
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
            //std::cout << "WM_ERASEBKGND" << std::endl;
            return 1; // Hintergrund nicht löschen

        case WM_COMMAND: { // Message when a command is executed (e.g., button click)
            //std::cout << "WM_COMMAND" << std::endl;
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
                MinimizeToTray(hwnd);
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
                    SetWindowText(hSearchBox, L"\u2315 Search Name (CTRL-F)"); 
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
                /*if (GetKeyState(VK_RETURN) & 0x8000) {
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
                }*/
            }
            // Check if the command ID is one of the specified IDs
            if (   id == ID_MINIMIZE || id == IDI_ICON_MINIMIZE
                || id == ID_MAXIMIZE || id == IDI_ICON_MAXIMIZE
                || id == ID_RESTORE  || id == IDI_ICON_RESTORE
                || id == ID_CLOSE    || id == IDI_ICON_CLOSE
                || (id >= ID_ARRANGE && id <= ID_ARRANGE + screenCount) 
                || (id >= ID_MOVE_TO_SCREEN_BASE && id <= ID_MOVE_TO_SCREEN_BASE + screenCount) 
                || id == ID_SAVEANDCLOSE || id == IDI_ICON_SAVE
                ) {

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

                if (actionOnGoing) {
                    int result = ShowMessageBoxAndHandleTimers(hwnd, L"Please wait for the former action to finish, then restart your new action.", L"Warning", MB_OK | MB_ICONWARNING, timers);
                    break;
                }

                actionOnGoing = true;
                
            }

            if (id == ID_MINIMIZE || LOWORD(wParam) == ID_CTRL_I || LOWORD(wParam) == IDI_ICON_MINIMIZE) { // Check if the ID is 2000 (Minimize)
                MinimizeToTray(hwnd); // Minimize the window to the tray
                int screenWidth = GetSystemMetrics(SM_CXSCREEN);
                int screenHeight = GetSystemMetrics(SM_CYSCREEN);
                for (auto& entry : processWindowsMap) { // Iterate through all processes
                    for (auto& window : entry.second) { // Iterate through all windows of a process
                        if (window.checked) { // Check if the window is selected
                            //SetWindowPos(window.hwnd, NULL, 0, 0, screenWidth, screenHeight, SWP_NOZORDER);
                            //SetWindowPos(window.hwnd, NULL, 0, 0, screenWidth, screenHeight, SWP_NOZORDER);
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
                actionOnGoing = false;
            } else if (id == ID_MAXIMIZE || LOWORD(wParam) == ID_CTRL_X || LOWORD(wParam) == IDI_ICON_MAXIMIZE) { // Check if the ID is 2000 (Minimize)
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
                actionOnGoing = false;
            } else if (id == ID_RESTORE || LOWORD(wParam) == ID_CTRL_R || LOWORD(wParam) == IDI_ICON_RESTORE) { // Check if the ID is 2001 (Restore)
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
                            Sleep(400);
                            SetForegroundWindow(window.hwnd); // Bring the window to the foreground
                            SetFocus(window.hwnd); // Set the focus to the window
                            ProcessMessages(); // Process messages
                            Sleep(400);                            
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
                actionOnGoing = false;
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
                actionOnGoing = false;
            } else if (id == ID_EXIT_BUTTON || LOWORD(wParam) == ID_CTRL_Q || LOWORD(wParam) == IDI_ICON_EXIT) {
                SetWindowPos(hwnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
                //int result = MessageBox(NULL, L"Are you sure you want to exit the 'Many Open Windows' program?", L"Exit Confirmation", MB_YESNO | MB_ICONQUESTION  | MB_TOPMOST);
                int result = ShowMessageBoxAndHandleTimers(NULL, L"Are you sure you want to exit the 'Many Open Windows' program?", L"Exit Confirmation", MB_YESNO | MB_ICONQUESTION  | MB_TOPMOST, timers);

                if (result == IDYES)
                {
                    PostQuitMessage(0);
                }
                else
                {
                    SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
                }
                actionOnGoing = false;
            } else if (id == ID_CLOSE_PROGRAMM || LOWORD(wParam) == ID_CTRL_W || LOWORD(wParam) == IDI_ICON_CLOSE) {
                    MinimizeToTray(hwnd); // Minimize the window to the tray
                    actionOnGoing = false;
            } else if (id == ID_SAVEANDCLOSE || LOWORD(wParam) == IDI_ICON_SAVE) { // Check if the ID is SAVEANDCLOSE
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
                actionOnGoing = false;
            } else if (id == ID_CLOSE) { // Check if the ID is CLOSE
                if (ConfirmClose(hwnd)) { // Display confirmation dialog
                    ProcessMessages(); // Process messages
                    MinimizeToTray(hwnd); // Minimize the window to the tray
                    for (const auto& entry : processWindowsMap) { // Iterate through all processes
                        for (const auto& window : entry.second) { // Iterate through all windows of a process
                            if (window.checked) { // Check if the window is selected
                                // Fenster in den Vordergrund bringen
                                SetForegroundWindow(window.hwnd);
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
                actionOnGoing = false;
            } else if ((id >= ID_ARRANGE && id <= ID_ARRANGE + screenCount) || LOWORD(wParam) == ID_CTRL_N || LOWORD(wParam) == IDI_ICON_ARRANGE) {//else if (id == 2003) { Check if the ID is 2003 (Arrange)
                //std::cout << "arrange" << std::endl;
                MinimizeToTray(hwnd); // Minimize the window to the tray
                //MessageBoxW(hwnd, L"aha2", L"Debug Info", MB_OK);
                ClearTemporaryTiles();
                //std::thread([hwnd, id]() {
                    int screenIndex = 0;
                    if (id != ID_CTRL_N && id != IDI_ICON_ARRANGE)
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
                    //std::cout << "screenWidth : " << screenWidth << std::endl;

                    int numWindows = 0; // Counter for the number of windows
                    for (auto& entry : processWindowsMap) { // Iterate through all processes
                        for (auto& window : entry.second) { // Iterate through all windows of a process
                            if (window.checked) { // Check if the window is selected
                                window.arranged = true; 
                                numWindows++; // Increment the counter
                            }
                        }
                    }
                    if (numWindows == 0) return 0;
                    int cols = static_cast<int>(ceil(sqrt(numWindows))); // Calculate the number of columns
                    int rows = cols > 0 ? (numWindows + cols - 1) / cols : 1; // Calculate the number of rows

                    int windowWidth = cols > 0 ? (screenWidth) / cols : 1; // Calculate the window width
                    int windowHeight = rows > 0 ? (screenHeight) / rows : 1; // Calculate the window height

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
                                //std::cout << windowWidth << " " << adjustedWidth << std::endl;
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

                    //std::cout << "minAdjustedWidth: " << minAdjustedWidth << std::endl;
                    //std::cout << "cols: " << cols << std::endl;
                    // Anzahl der Fenster nebeneinander und die Anzahl der Fensterreihen berechnen
                    //std::cout << maxAdjustedWidth << " " << screenWidth << " " << cols << std::endl;
                    cols = screenWidth / std::max(std::max(maxAdjustedWidth, screenWidth/cols), 1);
                                    //std::cout << cols << std::endl;
                    //std::cout << "cols: " << cols << std::endl;                
                    rows = cols > 0 ? (numWindows + cols - 1) / cols : 1;
                                    //std::cout << rows << std::endl;
                    windowWidth = cols > 0 ? screenWidth / cols : 1; // Calculate the window width
                    //std::cout << windowWidth << std::endl;
                    windowHeight = rows > 0 ? (screenHeight) / rows : 1; // Calculate the window height
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
                //}).detach();
                InvalidateRect(hwnd, NULL, TRUE); // Invalidate and redraw the window
                UpdateWindowList(hwnd); // Update the window list
                AdjustWindowSize(hwnd); // Adjust the window size
                ProcessMessages(); // Process messages
                Sleep(100); // Short pause
                //InvalidateWindow(hwnd);
                ProcessMessages(); // Process messages
                actionOnGoing = false;
            } else if (id == ID_TRAY_EXIT) { // Check if the ID is for tray exit
                actionOnGoing = false;
                PostQuitMessage(0); // Quit the application
            } else if ((id >= ID_SELECT_SCREEN_BASE && id <= ID_SELECT_SCREEN_BASE + screenCount) || (id == IDI_ICON_SELECTONSCREEN)) {
                std::vector<MonitorInfo> monitors;
                //std::cout << "Initializing monitor enumeration..." << std::endl;

                BOOL enumResult = EnumDisplayMonitors(NULL, NULL, MonitorEnumProcSpecial, reinterpret_cast<LPARAM>(&monitors));
                if (!enumResult) {
                    std::cerr << "EnumDisplayMonitors failed with error: " << GetLastError() << std::endl;
                } else {
                    //std::cout << "EnumDisplayMonitors succeeded." << std::endl;
                }

                if (id == IDI_ICON_SELECTONSCREEN)
                {
                    int screenIndex = 0;
                    globalScreenIndexChosen++;
                    if (globalScreenIndexChosen > monitors.size()) globalScreenIndexChosen = 1;
                    if (monitors.size() > 1) screenIndex = globalScreenIndexChosen - 1;
                    id = screenIndex + ID_SELECT_SCREEN_BASE;
                }

                //std::cout << "Number of monitors found: " << monitors.size() << std::endl;

                int screenSelected = (id - ID_SELECT_SCREEN_BASE) + 1;
                //std::cout << "Screen selected: " << screenSelected << std::endl;

                for (size_t i = 0; i < processNames.size(); ++i)
                {
                    const auto &processName = processNames[i];
                    for (auto &window : processWindowsMap[processName]) {
                        HMONITOR hMonitor = MonitorFromWindow(window.hwnd, MONITOR_DEFAULTTONEAREST);
                        MONITORINFO monitorInfo = { sizeof(MONITORINFO) };
                        if (GetMonitorInfo(hMonitor, &monitorInfo)) {
                            //std::cout << "Window handle: " << window.hwnd << ", Monitor handle: " << hMonitor << std::endl;

                            bool monitorFound = false;
                            int screenIndex = 0;

                            for (size_t i = 0; i < monitors.size(); ++i) {
                                if (compareMonitorInfo(monitors[i], hMonitor)) {
                                    screenIndex = i + 1; // Convert to 1-based index
                                    monitorFound = true;
                                    break;
                                }
                            }

                            if (monitorFound) {
                                //std::cout << "Screen index: " << screenIndex << std::endl;
                                if (screenIndex == screenSelected) {
                                    window.checked = 1;
                                    //std::cout << "Window is on screen " << screenIndex << std::endl;
                                } else {
                                    window.checked = 0;
                                    //std::cout << "Window is not on the selected screen" << std::endl;
                                }
                            } else {
                                //std::cout << "Monitor not found for window handle: " << window.hwnd << std::endl;
                            }
                        } else {
                            std::cerr << "GetMonitorInfo failed for monitor: " << hMonitor << " with error: " << GetLastError() << std::endl;
                        }
                    }
                }
                InvalidateRect(hwnd, NULL, TRUE);
                    //std::cout << "Invalidated rect for process: " << processName << std::endl;
                
                actionOnGoing = false;
            } else if ((id >= ID_MOVE_TO_SCREEN_BASE && id <= ID_MOVE_TO_SCREEN_BASE + screenCount) || LOWORD(wParam) == ID_CTRL_M || LOWORD(wParam) == IDI_ICON_MOVE) { // Check if the ID is 2004ff (Move to Screen x)
                MinimizeToTray(hwnd); // Minimize the window to the tray
                ClearTemporaryTiles();
                int screenIndex = 0;
                if (id != ID_CTRL_M && id != IDI_ICON_MOVE)
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
                            SetForegroundWindow(window.hwnd); // Bring the window to the foreground
                            ProcessMessages();
                            Sleep(100);
                        }
                    }
                }
                InvalidateRect(hwnd, NULL, TRUE); // Invalidate and redraw the window
                UpdateWindowList(hwnd); // Update the window list
                AdjustWindowSize(hwnd); // Adjust the window size
                ProcessMessages(); // Process messages
                //InvalidateWindow(hwnd);
                Sleep(100); // Short pause
                actionOnGoing = false;
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
                actionOnGoing = false;
            } else if (HIWORD(wParam) == EN_SETFOCUS && LOWORD(wParam) == IDC_SEARCHBOX) {
                wchar_t text[256];
                GetWindowText(hSearchBox, text, 256);
                if (wcscmp(text, L"\u2315 Search Name (CTRL-F)") == 0) {
                    //SetWindowText(hSearchBox, L"");
                }
                actionOnGoing = false;
            } else if (id == ID_REFRESH || LOWORD(wParam) == ID_CTRL_F || LOWORD(wParam) == IDI_ICON_REFRESH) {
                RefreshWindowList(hwnd);
                SetForegroundWindow(hwnd);
                SetFocus(hwnd);
                actionOnGoing = false;
            } else if (HIWORD(wParam) == EN_KILLFOCUS && LOWORD(wParam) == IDC_SEARCHBOX) {
                wchar_t text[256];
                GetWindowText(hSearchBox, text, 256);
                if (wcslen(text) == 0) {
                    //SetEditPlaceholder(hSearchBox, L"\u2315 Search Name (CTRL-F)");
                }
                actionOnGoing = false;
            } else if (HIWORD(wParam) == EN_CHANGE && LOWORD(wParam) == IDC_SEARCHBOX) {
                wchar_t searchString[256];
                GetWindowTextW(hSearchBox, searchString, 256);
                SearchAndCheck(searchString, hwnd);
                SetFocus(hwnd);
                actionOnGoing = false;
            } else if (LOWORD(wParam) == ID_CTRL_A) {
                // Ctrl+A was pressed
                SetFocus(hSearchBox);
                SendMessage(hSearchBox, EM_SETSEL, 0, -1); // Select all text in the search box
                actionOnGoing = false;
            } else if (LOWORD(wParam) == ID_SELECT_ALL || LOWORD(wParam) == ID_CTRL_S || LOWORD(wParam) == IDI_ICON_SELECTALL) {
                // Ctrl+S was pressed
                for (size_t i = 0; i < processNames.size(); ++i)
                {
                    const auto &processName = processNames[i];
                    checkboxState[processName] = 1;
                    for (auto &window : processWindowsMap[processName])
                    {
                        window.checked = 1;
                    }
                    InvalidateRect(hwnd, NULL, TRUE);
                }
                actionOnGoing = false;
            } else if (LOWORD(wParam) == ID_SELECT_NONE || LOWORD(wParam) == ID_CTRL_U || LOWORD(wParam) == IDI_ICON_UNSELECTALL) {
                // Ctrl+U was pressed
                for (size_t i = 0; i < processNames.size(); ++i)
                {
                    const auto &processName = processNames[i];
                    checkboxState[processName] = 0;
                    for (auto &window : processWindowsMap[processName])
                    {
                        window.checked = 0;
                    }
                    InvalidateRect(hwnd, NULL, TRUE);
                }
                actionOnGoing = false;
            }
            if (LOWORD(wParam) == ID_ABOUT || LOWORD(wParam) == IDI_ICON_ABOUT) {
                // Datum der letzten Kompilierung abrufen
                std::string compileDate = __DATE__;
                std::string compileTime = __TIME__;

                // Datum der letzten Kompilierung formatieren
                std::istringstream dateStream(compileDate);
                std::string monthStr, dayStr, yearStr;
                dateStream >> monthStr >> dayStr >> yearStr;

                // Monat in numerisches Format umwandeln
                int month = 0;
                if (monthStr == "Jan") month = 1;
                else if (monthStr == "Feb") month = 2;
                else if (monthStr == "Mar") month = 3;
                else if (monthStr == "Apr") month = 4;
                else if (monthStr == "May") month = 5;
                else if (monthStr == "Jun") month = 6;
                else if (monthStr == "Jul") month = 7;
                else if (monthStr == "Aug") month = 8;
                else if (monthStr == "Sep") month = 9;
                else if (monthStr == "Oct") month = 10;
                else if (monthStr == "Nov") month = 11;
                else if (monthStr == "Dec") month = 12;

                // Versionsnummer formatieren
                std::ostringstream version;
                version << "v1."
                    << std::setw(2) << std::setfill('0') << month << "."
                    << std::setw(2) << std::setfill('0') << std::stoi(dayStr)
                    << std::setw(2) << std::setfill('0') << std::stoi(yearStr.substr(2, 2));

                std::wstring versionW = stringToWString(version.str());
                //MessageBoxW(hwnd, versionW.c_str(), L"Info", MB_OK | MB_ICONINFORMATION);
                
                // Nachricht anzeigen
                std::wstring message = L"MoW - Many Open Windows\n\n"
                    L"Thanks for using this tool,\nand we hope it helps you a lot\n"
                    L"to be more productive.\n\n"
                    L"Shortcuts:\n\n"
                    L"CTRL+SHIFT+M:\tShow program window, reload, refresh list\n"
                    L"CTRL+I:\t\tMinimize Window(s)\n"
                    L"CTRL+X:\t\tMaximize Window(s)\n"
                    L"CTRL+R:\t\tRestore Window(s)\n"
                    L"CTRL+N:\t\tArrange Window(s)\n"
                    L"CTRL+M:\t\tMove Window(s)\n"
                    L"CTRL+S:\t\tSelect All Windows\n"
                    L"CTRL+U:\t\tUn-select All Windows\n"
                    L"CTRL+F:\t\tFind\n"
                    L"CTRL+W:\t\tClose Program to Tray\n"
                    L"CTRL+Q:\t\tExit Program\n\n\n"
                    + std::wstring(version.str().begin(), version.str().end());

                MessageBox(hwnd, message.c_str(), L"About", MB_OK);
            }
            
        }
        break;

        case WM_LBUTTONDOWN: { // Message when the left mouse button is pressed
            //std::cout << "WM_LBUTTONDOWN" << std::endl;
            isScrolling = true; // Start scrolling
            SetCapture(hwnd); // Capture the mouse to receive all mouse events
            lastMousePos.x = GET_X_LPARAM(lParam); // Store the X position of the mouse
            lastMousePos.y = GET_Y_LPARAM(lParam); // Store the Y position of the mouse

            POINT pt; // Declaration of a POINT structure to store the cursor position
            GetCursorPos(&pt); // Retrieve the current cursor position
            ScreenToClient(hwnd, &pt); // Convert screen coordinates to client coordinates
            int yPos = 0 - scrollPos + heightToolbar; // Initialize the y-position based on the scroll position
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
            //std::cout << "WM_MENUSELECT" << std::endl;
            //std::cout << "Beginne WM_MENUSELECT" << std::endl;
            // Löschen Sie die temporären Kacheln bei jeder Menüauswahl
            try {
                //std::cout << "Vor ClearTemporaryTiles" << std::endl;
                ClearTemporaryTiles();
                //std::cout << "Nach ClearTemporaryTiles" << std::endl;

                if (HIWORD(wParam) & MF_MOUSESELECT) {
                    std::vector<MonitorInfo> monitors;
                    //std::cout << "Vor EnumDisplayMonitors" << std::endl;
                    if (!EnumDisplayMonitors(NULL, NULL, MonitorEnumProc, reinterpret_cast<LPARAM>(&monitors))) {
                        throw std::runtime_error("EnumDisplayMonitors failed");
                    }
                    //std::cout << "Nach EnumDisplayMonitors" << std::endl;

                    int menuIndex = LOWORD(wParam) - ID_ARRANGE;
                    if (menuIndex >= 0 && menuIndex < monitors.size()) {
                        //std::cout << "Vor ShowTemporaryTiles" << std::endl;
                        SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
                        StartShowTemporaryTilesInThread(hwnd, menuIndex);
                        //std::cout << "Nach ShowTemporaryTiles" << std::endl;
                    }
                    menuIndex = LOWORD(wParam) - ID_MOVE_TO_SCREEN_BASE;
                    if (menuIndex >= 0 && menuIndex < monitors.size()) {
                        //std::cout << "Vor ShowTemporaryTiles for Move To Screen" << std::endl;
                        SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
                        globalTotalCheckedIgnore = true;
                        StartShowTemporaryTilesInThread(hwnd, menuIndex);
                        //std::cout << "Nach ShowTemporaryTiles for Move To Screen" << std::endl;
                    }
                }
            } catch (const std::exception& e) {
                //std::cout << "Error in WM_MENUSELECT: " << e.what() << std::endl;
            }
            //std::cout << "Alles gut bei WM_MENUSELECT bis zum Ende" << std::endl;
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
            //std::cout << "WM_MOUSEMOVE" << std::endl;

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
                }
                else {
                    POINT pt;
                    GetCursorPos(&pt);

                    // Überprüfen, ob die Maus mindestens 3 Pixel verschoben wurde
                    if (prevPt.x == -1 || prevPt.y == -1 || abs(pt.x - prevPt.x) >= 3 || abs(pt.y - prevPt.y) >= 3) {
                        // Mausposition aktualisieren
                        prevPt = pt;

                        ScreenToClient(hwnd, &pt);

                        int yPos = 0 - scrollPos + heightToolbar;
                        int newHighlightedRow = -1;
                        int newHighlightedWindowRow = -1;

                        for (size_t i = 0; i < processNames.size(); ++i) {
                            const auto& processName = processNames[i];
                            RECT rect = { -30, yPos, textWidth, yPos + 30 };
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
                                        RECT windowRect = { -30, yPos, textWidth, yPos + 30 };
                                        if (PtInRect(&windowRect, pt)) {
                                            newHighlightedWindowRow = i * 100000 + j;
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

        }
        break;
    
        case WM_LBUTTONUP: { // Message when the left mouse button is released
            //std::cout << "WM_LBUTTONUP" << std::endl;
            isScrolling = false; // Stop scrolling
            ReleaseCapture(); // Release the mouse
        }
        break;
        
        case WM_MOUSEWHEEL: { // Nachricht, wenn das Mausrad gedreht wird
            //std::cout << "WM_MOUSEWHEEL" << std::endl;
            int delta = GET_WHEEL_DELTA_WPARAM(wParam); // Abrufen der Scroll-Richtung und -Menge
            SCROLLINFO si = {}; // Initialisieren einer SCROLLINFO-Struktur
            si.cbSize = sizeof(si); // Setzen der Größe der Struktur
            si.fMask = SIF_ALL; // Festlegen, dass alle Informationen gesetzt werden
            GetScrollInfo(hwnd, SB_VERT, &si); // Abrufen der aktuellen Scroll-Informationen
            int yPos = si.nPos - delta / std::max(WHEEL_DELTA,1) * 30; // Neue Y-Position berechnen

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
                ScrollWindowEx(hwnd, 0, delta / std::max(WHEEL_DELTA, 1) * 30, &scrollRect, &scrollRect, NULL, NULL, SW_INVALIDATE | SW_ERASE);
                InvalidateRect(hwnd, &scrollRect, TRUE); // Rechteck ungültig machen und neu zeichnen
            }
        }
        break;
                
        case WM_PAINT: {
            //std::cout << "WM_PAINT" << std::endl;
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
            int yPos = searchBoxHeight - scrollPos + heightToolbar; // Platz für das Suchfeld und etwas Abstand
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

                //std::wstring textAfterProcessName = L" (" + std::to_wstring(AlreadyOneChecked) + L" / " + std::to_wstring(windows.size()) + L")\tselected";
                std::wstringstream ss;
                if (AlreadyOneChecked == 0) {
                    ss << L"        (" << std::setw(2) << std::setfill(L'0') << windows.size() << L" windows)";
                }
                else {
                    ss << L" (" << std::setw(2) << std::setfill(L'0') << AlreadyOneChecked
                        << L" / " << std::setw(2) << std::setfill(L'0') << windows.size() << L" selected)";
                }
                std::wstring textAfterProcessName = ss.str();
                
                std::wstring textAfterProcessNameAfterNumbers = textAfterProcessName;

                bool listOfCheckedStarted = false;
                for (auto& window : processWindowsMap[processName]) { // Iterate through all windows of the process
                    if (window.checked)
                    {
                        if (!listOfCheckedStarted)
                        {
                            textAfterProcessNameAfterNumbers += L"    ";
                            listOfCheckedStarted = true;
                        }
                        else
                        {
                            textAfterProcessNameAfterNumbers += L" ]";
                        }
                        textAfterProcessNameAfterNumbers += L"  [ " + window.title;
                    }
                }
                if (listOfCheckedStarted) textAfterProcessNameAfterNumbers += L" ]";
                
                int clientWidth = clientRect.right - clientRect.left;
                textWidth = std::max(clientWidth, minWidth);
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

                // Zuerst den text schwarz schreiben
                hOldFont = (HFONT)SelectObject(hdcMem, hFontNormal);
                SetTextColor(hdcMem, RGB(0, 0, 0));
                /*if (textAfterProcessNameAfterNumbers.length() > 20) {
                    textAfterProcessNameAfterNumbers = textAfterProcessNameAfterNumbers.substr(0, 20) + L"...";
                }*/
                DrawTextW(hdcMem, textAfterProcessNameAfterNumbers.c_str(), -1, &rect, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
                //SelectObject(hdcMem, hOldFont);
                //DeleteObject(hFontNormal);


                // einen Teil des textes blau/rot darüber schreiben
                //hOldFont = (HFONT)SelectObject(hdcMem, hFontNormal);
                //if (textAfterProcessName.find(L"selected") == std::wstring::npos) {
                    SetTextColor(hdcMem, RGB(0, 0, 255)); // Blue
                /*}
                else {
                    SetTextColor(hdcMem, blinkState ? RGB(0, 0, 255) : RGB(255, 0, 0)); // Blinking between blue and red
                }*/
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

            std::wstring windowTitle = L"Many Open Windows - Total Windows Selected: " + std::to_wstring(TotalChecked);
            globalTotalChecked = TotalChecked;
            SetWindowText(hwnd, windowTitle.c_str());


            // Scroll-Informationen aktualisieren
            /*SCROLLINFO si = {}; // Initialize a SCROLLINFO structure
            si.cbSize = sizeof(si); // Set the size of the SCROLLINFO structure
            si.fMask = SIF_RANGE | SIF_PAGE; // Specify the masks to use
            si.nMin = 0; // Set the minimum scroll range
            si.nMax = std::max(nmbOfItemsOnWindow * 30, 100);
            si.nPage = si.nMax / std::max(nmbOfItemsOnWindow,1); // Set the page length for scrolling
            SetScrollInfo(hwnd, SB_VERT, &si, TRUE); // Set the scroll information for the vertical scrollbar
            */
            UpdateScrollBar(hwnd, nmbOfItemsOnWindow);
            UpdateControlPositions(hwnd);
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
                    //std::cout << "WM_SIZE" << std::endl;
                    RECT rect; // Declaration of a RECT structure to store the client rectangles
                    GetClientRect(hwnd, &rect); // Retrieve the client rectangles of the window
                    SCROLLINFO si = {}; // Initialize a SCROLLINFO structure
                    si.cbSize = sizeof(si); // Set the size of the SCROLLINFO structure
                    si.fMask = SIF_PAGE; // Specify the masks to use
                    si.nPage = rect.bottom - rect.top - 115; // Set the page length for scrolling
                    SetScrollInfo(hwnd, SB_VERT, &si, TRUE); // Set the scroll information for the vertical scrollbar
                    int scrollbarWidth = GetSystemMetrics(SM_CXVSCROLL); // Retrieve the width of the vertical scrollbar
                    int width = rect.right - rect.left - scrollbarWidth; // Calculate the width of the window without the scrollbar
        
                    // Get the screen height
                    /*int screenHeight = GetSystemMetrics(SM_CYSCREEN);
                    int maxHeight = screenHeight - 100; // Set a maximum height with some padding
        
                    // Adjust the window height if it exceeds the maximum height
                    if (rect.bottom - rect.top > maxHeight) {
                        SetWindowPos(hwnd, NULL, 0, 0, rect.right - rect.left, maxHeight, SWP_NOMOVE | SWP_NOZORDER);
                        GetClientRect(hwnd, &rect); // Retrieve the updated client rectangles of the window
                    }
        
                    int yPos = rect.bottom - 55; // Calculate the y-position for the white bar
                    SetWindowPos(whiteBar, NULL, 0, yPos, width + scrollbarWidth, 55, SWP_NOZORDER);*/ // Position the white bar
                    //InvalidateWindow(hwnd);
                    if (wParam == SIZE_MINIMIZED) {
                        KillTimer(hwnd, BLINKING_TIMER_ID); // Timer stoppen
                    } else {
                        SetTimer(hwnd, BLINKING_TIMER_ID, BLINKING_TIMER_ID_time, NULL); // Timer neu starten
                    }
                    // int buttonCount = 4; // Number of buttons
                    // int buttonWidth = (width - 20) / buttonCount; // Calculate the width of a button
        
                    // Position the buttons for minimize, restore, close, and arrange
                    // SetWindowPos(minimizeButton, NULL, 10, yPos + 5 + 5, buttonWidth, 40, SWP_NOZORDER);
                    // SetWindowPos(restoreButton, NULL, 10 + buttonWidth + 5, yPos + 5 + 5, buttonWidth, 40, SWP_NOZORDER);
                    // SetWindowPos(closeButton, NULL, 10 + 2 * (buttonWidth + 5), yPos + 5 + 5, buttonWidth, 40, SWP_NOZORDER);
                    // SetWindowPos(arrangeButton, NULL, 10 + 3 * (buttonWidth + 5), yPos + 5 + 5, buttonWidth, 40, SWP_NOZORDER);
                    // UpdateControlPositions(hwnd);
                    SendMessage(hwndToolbar, TB_AUTOSIZE, 0, 0); // Toolbar neu zeichnen
                    SendMessage(hSearchBox, TB_AUTOSIZE, 0, 0);
                    SendMessage(hEraseButton, TB_AUTOSIZE, 0, 0);
                    SendMessage(hGoToButton, TB_AUTOSIZE, 0, 0);
                    InvalidateRect(hwnd, NULL, TRUE);
                }
                break;
        case WM_DESTROY: { // Message when the window is destroyed
            //std::cout << "WM_DESTROY" << std::endl;
            UnregisterHotKey(NULL, HOTKEY_ID);
            RemoveTrayIcon(hwnd); // Remove the tray icon
            PostQuitMessage(0); // Quit the application
            return 0; // Return 0
        }
        break;

        case WM_CLOSE: { // Message when the window is closed
            //std::cout << "WM_CLOSE" << std::endl;
            //RemoveTrayIcon(hwnd); // Remove the tray icon (commented out)
            //DestroyWindow(hwnd); // Destroy the window (commented out)
            MinimizeToTray(hwnd); // Minimize the window to the tray
            return 0; // Return 0
        }
        break;

        case WM_TRAYICON:
        case WM_HOTKEY: { 
            //std::cout << "WM_HOTKEY or WM_TRAYICON" << std::endl;
            if (uMsg == WM_HOTKEY && wParam != HOTKEY_ID) {
                //std::cout << "Incorrect hotkey ID" << std::endl;
                break; // Ensure it's the correct hotkey
            }
            if (lParam == WM_LBUTTONUP || uMsg == WM_HOTKEY) { 
                //std::cout << "Restoring window" << std::endl;

                // Show the window and bring it to the foreground as early as possible
                windowReady = false;
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

                /*if (!CreateSimpleToolbar(hwnd, ((LPCREATESTRUCT)lParam)->hInstance)) {
                    MessageBox(hwnd, L"Failed to create toolbar!", L"Error", MB_OK | MB_ICONERROR);
                    return -1; // Fenstererstellung abbrechen
                }*/

                // Perform essential operations           
                RefreshWindowList(hwnd);
                //AdjustWindowToFitMenu(hwnd);
                //UpdateWindowList(hwnd);

                // Perform non-essential operations asynchronously
                std::thread([hwnd]() {
                    AdjustWindowSize(hwnd); 
                    UpdateDynamicMenus(hwnd); 
                    std::vector<BOOL> timerStates(timers.size());
                    RestartTimers(hwnd, timers, timerStates);
                    CreateTrayIcon(hwnd);
                    //UpdateWindowList(hwnd);

                    SCROLLINFO si = {};
                    si.cbSize = sizeof(si);
                    si.fMask = SIF_RANGE | SIF_PAGE;
                    si.nMin = 0;
                    si.nMax =(processNames.size() + 2)  * 30;
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
                    windowReady = true;
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
                SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(IDC_ERASEBUTTON, 0), 0);

                /*if (!CreateSimpleToolbar(hwnd, ((LPCREATESTRUCT)lParam)->hInstance)) {
                    MessageBox(hwnd, L"Failed to create toolbar!", L"Error", MB_OK | MB_ICONERROR);
                    return -1; // Fenstererstellung abbrechen
                }*/

                // Toolbar wieder sichtbar machen
                if (hwndToolbar) {
                    ShowWindow(hwndToolbar, SW_SHOW);
                }
                else
                    MessageBox(hwnd, L"kann toolbar nicht wieder sichtbar machen", L"Error", MB_OK | MB_ICONERROR);

                SetForegroundWindow(hwnd);
                SetFocus(hwnd);
            } else if (lParam == WM_RBUTTONUP) { 
                ShowTrayMenu(hwnd); 
            }
        }
        break;

        default: { // Default message
            //std::cout << "default" << std::endl;
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
        //std::cout << "Die Prioritätsstufe wurde erfolgreich erhöht." << std::endl;
    } else {
        //std::cerr << "Fehler beim Setzen der Prioritätsstufe." << std::endl;
    }
    //std::cout << "WinMain" << std::endl;
    // Main function for Windows applications
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

    const wchar_t CLASS_NAME[] = L"SampleWindowClass"; // Define the class name for the window

    WNDCLASSW wc = { }; // Initialize a WNDCLASSW structure
    wc.lpfnWndProc = WindowProc; // Set the window procedure
    wc.hInstance = hInstance; // Set the instance handle
    wc.lpszClassName = CLASS_NAME; // Set the class name
    wc.hCursor = LoadCursor(NULL, IDC_ARROW); // Load the default cursor (arrow)
    wc.hbrBackground = CreateSolidBrush(RGB(255, 255, 255)); // Create a white background brush
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MYICON)); // Load the window icon

    RegisterClassW(&wc); // Register the window class

    std::wstring windowTitle = L"Many Open Windows - Total Windows Selected: " + std::to_wstring(0);
    HWND hwnd = CreateWindowExW(
        0, // Extended window style
        CLASS_NAME, // Name of the window class
        windowTitle.c_str(), // Window title
        WS_OVERLAPPED | WS_SYSMENU | WS_VSCROLL | WS_EX_COMPOSITED | WS_SIZEBOX, // Window style
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, // Position and size of the window
        NULL, // No parent window
        NULL, // No menu
        hInstance, // Instance handle
        NULL // No additional parameters
    );

    if (hwnd == NULL) {
        return 0; // Return 0 if the window could not be created
    }

    AdjustWindowToFitMenu(hwnd);

    // Register the hotkey (CTRL+SHIFT+M)
    if (!RegisterHotKey(hwnd, HOTKEY_ID, MOD_CONTROL | MOD_SHIFT, 'M')) {
        //std::cout << "Failed to register hotkey" << std::endl;
        return 0;
    }

    // Register the hotkey (CTRL+X)
    /*if (!RegisterHotKey(hwnd, HOTKEY_EXIT, MOD_CONTROL, 'X')) {
        //std::cout << "Failed to register hotkey" << std::endl;
        return 1;
    }*/

    HICON hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MYICON)); // Load the window icon
    SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon); // Set the small window icon
    SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon); // Set the large window icon

    ShowWindow(hwnd, SW_HIDE); // Hide the window at startup
    UpdateWindow(hwnd); // Update the window

    MSG msg = {}; // Initialize a MSG structure
    while (GetMessage(&msg, NULL, 0, 0)) {
        //std::cout << "Message received: " << msg.message << std::endl;
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    UnregisterHotKey(NULL, HOTKEY_ID);

    return 0; // Return 0 when the application exits
}