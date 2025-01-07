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

#pragma comment(lib, "psapi.lib") // Link psapi.lib library

#define WM_TRAYICON (WM_USER + 1) // Define a custom message for the tray icon
#define WM_UPDATE_LIST (WM_USER + 2) // Define a custom message to update the window list

// Structure to store window information
struct WindowInfo {
    HWND hwnd; // Handle of the window
    std::wstring title; // Title of the window
    std::wstring processName; // Name of the associated process
    bool checked; // Status indicating if the window is selected
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

// Global variables
static std::unordered_map<std::wstring, std::vector<WindowInfo>> processWindowsMap; // Hashmap to store windows by processes
static std::unordered_map<std::wstring, bool> expandedState; // Hashmap to store the expanded state of processes
static std::vector<std::wstring> processNames; // Vector to store process names
static std::unordered_map<std::wstring, bool> checkboxState; // Hashmap to store the checkbox state
static HWND closeButton; // Handle of the close button
static HWND arrangeButton; // Handle of the arrange button
std::vector<WindowInfo> currentWindows; // Vector to store current windows
static bool initialized = false; // Status indicating if the application is initialized
static std::unordered_map<std::wstring, HWND> expandButtons; // Hashmap to store expand buttons
static HWND whiteBar; // Handle of the white bar
static HWND minimizeButton; // Handle of the minimize button
static HWND restoreButton; // Handle of the restore button
static bool isScrolling = false; // Variable to track if scrolling is in progress
static POINT lastMousePos = {0, 0}; // Variable to store the last mouse position
HWND hwndTT;

// Callback function to list open windows
BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam) {
    wchar_t title[256]; // Buffer for the window title
    DWORD processId; // Process ID
    GetWindowThreadProcessId(hwnd, &processId); // Retrieve the process ID of the window
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processId); // Open the process
    if (hProcess) {
        wchar_t processName[MAX_PATH]; // Buffer for the process name
        if (GetModuleBaseNameW(hProcess, NULL, processName, sizeof(processName) / sizeof(wchar_t))) { // Retrieve the process name
            // List of processes to exclude as they are created by Windows and not useful to the user
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
            if (IsWindowVisible(hwnd) && std::find(excludedProcesses.begin(), excludedProcesses.end(), processName) == excludedProcesses.end()) { // Check if the window is visible and not excluded
                int length = GetWindowTextW(hwnd, title, sizeof(title) / sizeof(wchar_t)); // Retrieve the window title
                if (length > 0 && wcscmp(title, L"Program Manager") != 0) { // Exclude "Program Manager"
                    std::vector<WindowInfo>* windows = reinterpret_cast<std::vector<WindowInfo>*>(lParam); // Cast lParam to a vector of WindowInfo
                    windows->emplace_back(WindowInfo{ hwnd, title, processName, false }); // Add the window to the list
                }
            }
        }
        CloseHandle(hProcess); // Close the process handle
    }
    return TRUE; // Continue enumeration
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
    wcscpy_s(nid.szTip, sizeof(nid.szTip) / sizeof(wchar_t), L"Minimize and Restore"); // Tooltip for the tray icon
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
    ShowWindow(hwnd, SW_RESTORE); // Restore the window

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
    // Function to update the window list

    // Clear window content
    RECT rect; // Declaration of a RECT structure to store the window size
    GetClientRect(hwnd, &rect); // Retrieve the client rectangles of the window
    HDC hdc = GetDC(hwnd); // Retrieve the device context
    HBRUSH hBrush = (HBRUSH)GetStockObject(WHITE_BRUSH); // Create a white brush
    FillRect(hdc, &rect, hBrush); // Fill the rectangle with white color
    ReleaseDC(hwnd, hdc); // Release the device context

    // Destroy old controls
    for (auto& button : expandButtons) { // Iterate through all expandButtons
        DestroyWindow(button.second); // Destroy each button
    }
    expandButtons.clear(); // Clear the button list

    // Destroy the white bar and control buttons if they exist
    if (whiteBar) { // Check if the white bar exists
        DestroyWindow(whiteBar); // Destroy the white bar
        whiteBar = NULL; // Reset the pointer
    }
    if (minimizeButton) { // Check if the minimize button exists
        DestroyWindow(minimizeButton); // Destroy the minimize button
        minimizeButton = NULL; // Reset the pointer
    }
    if (restoreButton) { // Check if the restore button exists
        DestroyWindow(restoreButton); // Destroy the restore button
        restoreButton = NULL; // Reset the pointer
    }
    if (closeButton) { // Check if the close button exists
        DestroyWindow(closeButton); // Destroy the close button
        closeButton = NULL; // Reset the pointer
    }
    if (arrangeButton) { // Check if the arrange button exists
        DestroyWindow(arrangeButton); // Destroy the arrange button
        arrangeButton = NULL; // Reset the pointer
    }

    // Clear the window and state maps
    processWindowsMap.clear(); // Clear the map that associates windows with processes
    processNames.clear(); // Clear the list of process names
    checkboxState.clear(); // Clear the map that stores the checkbox state
    expandedState.clear(); // Clear the map that stores the expanded state

    // Retrieve windows
    auto windows = getOpenWindows(); // Retrieve the current open windows
    if (windows.empty()) { // Check if no windows were found
        // MessageBoxW(hwnd, L"No windows found", L"Debug Info", MB_OK); // Debug output (commented out)
    }

    // Associate windows with their respective processes
    for (auto& window : windows) { // Iterate through all found windows
        processWindowsMap[window.processName].push_back(window); // Add the window to the respective process
        expandedState[window.processName] = false; // Default to not expanded
        checkboxState[window.processName] = false; // Default to not selected
    }

    // Insert process names into a list
    for (const auto& entry : processWindowsMap) { // Iterate through all entries in the map
        processNames.push_back(entry.first); // Add the process name to the list
    }

    // Sort process names alphabetically
    std::sort(processNames.begin(), std::end(processNames), caseInsensitiveCompare); // Sort the process names

    // Set scroll information
    SCROLLINFO si = {}; // Initialize a SCROLLINFO structure
    si.cbSize = sizeof(si); // Set the size of the SCROLLINFO structure
    si.fMask = SIF_RANGE | SIF_PAGE; // Specify the masks to use
    si.nMin = 0; // Set the minimum scroll range
    si.nMax = processNames.size() * 30; // Set the maximum scroll range based on the number of processes
    si.nPage = 10; // Set the page length for scrolling
    SetScrollInfo(hwnd, SB_VERT, &si, TRUE); // Set the scroll information for the vertical scrollbar

    // Create buttons for the processes
    int yPos = 60; // Initialize the y-position for the buttons
    for (size_t i = 0; i < processNames.size(); ++i) { // Iterate through all process names
        HWND expandButton = CreateWindowExW(
            0, // Extended window style
            L"BUTTON", // Name of the window class
            L">", // Text of the button
            WS_VISIBLE | WS_CHILD | BS_OWNERDRAW, // Window style
            10, yPos, 30, 30, // Position and size of the button
            hwnd, // Handle of the parent window
            (HMENU)(3000 + i), // Menu handle (ID of the button)
            GetModuleHandle(NULL), // Instance handle
            NULL // No additional parameters
        );
        expandButtons[processNames[i]] = expandButton; // Add the button to the map
        yPos += 30; // Increase the y-position for the next button
    }

    // Create a white bar at the bottom of the window
    GetClientRect(hwnd, &rect); // Retrieve the client rectangles of the window
    int scrollbarWidth = GetSystemMetrics(SM_CXVSCROLL); // Retrieve the width of the vertical scrollbar
    int width = rect.right - rect.left - scrollbarWidth; // Calculate the width of the window without the scrollbar
    whiteBar = CreateWindowExW(
        0, // Extended window style
        L"STATIC", // Name of the window class
        NULL, // No text
        WS_VISIBLE | WS_CHILD | SS_WHITERECT, // Window style
        0, rect.bottom - 55, width + scrollbarWidth, 55, // Position and size of the white bar
        hwnd, // Handle of the parent window
        NULL, // No menu
        GetModuleHandle(NULL), // Instance handle
        NULL // No additional parameters
    );

    // Create buttons for minimize, restore, close, and arrange
    int buttonCount = 4; // Number of buttons
    int buttonWidth = (width - 20) / buttonCount; // Calculate the width of a button

    minimizeButton = CreateWindowW(
        L"BUTTON", // Name of the window class
        L"Minimize\nWindow(s)", // Text of the button
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON | BS_MULTILINE, // Window style
        10, rect.bottom - 50, buttonWidth, 40, // Position and size of the button
        hwnd, // Handle of the parent window
        (HMENU)2000, // Menu handle (ID of the button)
        GetModuleHandle(NULL), // Instance handle
        NULL // No additional parameters
    );

    restoreButton = CreateWindowW(
        L"BUTTON", // Name of the window class
        L"Restore\nWindow(s)", // Text of the button
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON | BS_MULTILINE, // Window style
        10 + buttonWidth + 5, rect.bottom - 50, buttonWidth, 40, // Position and size of the button
        hwnd, // Handle of the parent window
        (HMENU)2001, // Menu handle (ID of the button)
        GetModuleHandle(NULL), // Instance handle
        NULL // No additional parameters
    );

    closeButton = CreateWindowW(
        L"BUTTON", // Name of the window class
        L"Close\nWindow(s)", // Text of the button
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON | BS_MULTILINE, // Window style
        10 + 2 * (buttonWidth + 5), rect.bottom - 50, buttonWidth, 40, // Position and size of the button
        hwnd, // Handle of the parent window
        (HMENU)2002, // Menu handle (ID of the button)
        GetModuleHandle(NULL), // Instance handle
        NULL // No additional parameters
    );

    arrangeButton = CreateWindowW(
        L"BUTTON", // Name of the window class
        L"Arrange\nWindow(s)", // Text of the button
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON | BS_MULTILINE, // Window style
        10 + 3 * (buttonWidth + 5), rect.bottom - 50, buttonWidth, 40, // Position and size of the button
        hwnd, // Handle of the parent window
        (HMENU)2003, // Menu handle (ID of the button)
        GetModuleHandle(NULL), // Instance handle
        NULL // No additional parameters
    );

    // Save current windows and adjust window size
    SaveCurrentWindows(); // Save current windows
    AdjustWindowSize(hwnd); // Adjust window size

    // Debug output
    // std::wstring debugMessage = L"Number of windows found: " + std::to_wstring(windows.size());
    // MessageBoxW(hwnd, debugMessage.c_str(), L"Debug Info", MB_OK);
}

// Function to adjust the window size
void AdjustWindowSize(HWND hwnd) {
    RECT rect; // Declaration of a RECT structure to store the window size
    GetClientRect(hwnd, &rect); // Retrieve the client rectangles of the window
    int width = rect.right - rect.left; // Calculate the window width
    int height = rect.bottom - rect.top; // Calculate the window height
    SCROLLINFO si = {}; // Initialize a SCROLLINFO structure
    si.cbSize = sizeof(si); // Set the size of the SCROLLINFO structure
    si.fMask = SIF_RANGE | SIF_PAGE; // Specify the masks to use
    GetScrollInfo(hwnd, SB_VERT, &si); // Retrieve the scroll information of the window
    int screenHeight = GetSystemMetrics(SM_CYSCREEN); // Retrieve the screen height
    int titleBarHeight = GetSystemMetrics(SM_CYCAPTION); // Retrieve the title bar height
    int usableScreenHeight = screenHeight - titleBarHeight - 25; // Calculate the usable screen height
    int contentHeight = si.nMax + 30 + 70; // Calculate the content height
    int contentWidth = 500; // Set the content width
    int newHeight = std::min(contentHeight, usableScreenHeight); // Calculate the new window height
    int xPos = (GetSystemMetrics(SM_CXSCREEN) - contentWidth) / 2; // Calculate the X position of the window
    int yPos = (usableScreenHeight - newHeight) / 2; // Calculate the Y position of the window
    SetWindowPos(hwnd, NULL, xPos, yPos, contentWidth, newHeight, SWP_NOZORDER); // Set the new window position and size
}

void RefreshWindowList(HWND hwnd) {
    // Invalidate and redraw the window immediately
    RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE); // Invalidate and redraw

    // Process messages
    ProcessMessages(); // Process messages

    // Update the window list
    UpdateWindowList(hwnd); // Update the window list
}

// Function to confirm closing windows
bool ConfirmClose(HWND hwnd) {
    int result = MessageBoxW(hwnd, L"Do you really want to close the selected windows?", L"Yes", MB_YESNO | MB_ICONQUESTION | MB_TOPMOST); // Display confirmation dialog
    return (result == IDYES); // Return whether the user chose "Yes"
}

// Window procedure
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static int scrollPos = 0; // Static variable to store the scroll position
    int id; // Variable to store the ID
    switch (uMsg) { // Check the messages
case WM_CREATE: {
    CreateTrayIcon(hwnd);
    if (!initialized) {
        UpdateWindowList(hwnd);
        initialized = true;
        MinimizeToTray(hwnd);
    }
    hwndTT = CreateTooltip(hwnd); // Tooltip-Fenster erstellen

    SCROLLINFO si = {};
    si.cbSize = sizeof(si);
    si.fMask = SIF_RANGE | SIF_PAGE;
    si.nMin = 0;
    si.nMax = processNames.size() * 30;
    si.nPage = 10;
    SetScrollInfo(hwnd, SB_VERT, &si, TRUE);
    scrollPos = 0;
    //MessageBoxW(hwnd, (L"Initial scrollPos: " + std::to_wstring(scrollPos)).c_str(), L"Info", MB_OK | MB_ICONINFORMATION);
}
break;

case WM_TIMER: {
    if (wParam == 1) {
        DestroyWindow(hwnd);
    }
}
break;

        case WM_DRAWITEM: { // Message when drawing an item
            LPDRAWITEMSTRUCT lpDrawItem = (LPDRAWITEMSTRUCT)lParam; // Draw item structure
            if (lpDrawItem->CtlID >= 3000 && lpDrawItem->CtlID < 3000 + processNames.size()) { // Check the ID
                std::wstring processName = processNames[lpDrawItem->CtlID - 3000]; // Retrieve the process name
                wchar_t buttonText = expandedState[processName] ? 'v' : '>'; // Set the button text
                HBRUSH hBrush = CreateSolidBrush(GetSysColor(COLOR_WINDOW)); // Create a brush
                FillRect(lpDrawItem->hDC, &lpDrawItem->rcItem, hBrush); // Fill the rectangle
                DeleteObject(hBrush); // Delete the brush
                SetBkMode(lpDrawItem->hDC, TRANSPARENT); // Set the background mode
                SetTextColor(lpDrawItem->hDC, RGB(0, 0, 0)); // Set the text color
                DrawTextW(lpDrawItem->hDC, &buttonText, 1, &lpDrawItem->rcItem, DT_CENTER | DT_VCENTER | DT_SINGLELINE); // Draw the text
                return TRUE; // Return TRUE
            }
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

            for (const auto& entry : processWindowsMap) { // Iterate through the process window map
                processNames.push_back(entry.first); // Add the process name
            }
            std::sort(processNames.begin(), std::end(processNames), caseInsensitiveCompare); // Sort the process names
            SCROLLINFO si = {}; // Initialize scroll info
            si.cbSize = sizeof(si); // Set the size of the scroll info
            si.fMask = SIF_RANGE | SIF_PAGE; // Set the mask
            si.nMin = 0; // Set the minimum
            si.nMax = processNames.size() * 30; // Set the maximum
            si.nPage = 10; // Set the page
            SetScrollInfo(hwnd, SB_VERT, &si, TRUE); // Set the scroll info
            InvalidateRect(hwnd, NULL, TRUE); // Invalidate the rectangle
            AdjustWindowSize(hwnd); // Adjust the window size
        }
case WM_VSCROLL: {
    SCROLLINFO si = {};
    si.cbSize = sizeof(si);
    si.fMask = SIF_ALL;
    GetScrollInfo(hwnd, SB_VERT, &si);
    int yPos = si.nPos;
    switch (LOWORD(wParam)) {
        case SB_LINEUP: yPos -= 1; break;
        case SB_LINEDOWN: yPos += 1; break;
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
        InvalidateRect(hwnd, NULL, TRUE);
    }
    //MessageBoxW(hwnd, (L"Updated scrollPos: " + std::to_wstring(scrollPos)).c_str(), L"Info", MB_OK | MB_ICONINFORMATION);
}
break;
        case WM_COMMAND: { // Message when a command is executed (e.g., button click)
            id = LOWORD(wParam); // Extract the command ID from wParam
            if (id == 2000) { // Check if the ID is 2000 (Minimize)
                for (const auto& entry : processWindowsMap) { // Iterate through all processes
                    for (const auto& window : entry.second) { // Iterate through all windows of a process
                        if (window.checked) { // Check if the window is selected
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
                Sleep(100); // Short pause
                MinimizeToTray(hwnd); // Minimize the window to the tray
            } else if (id == 2001) { // Check if the ID is 2001 (Restore)
                for (const auto& entry : processWindowsMap) { // Iterate through all processes
                    for (const auto& window : entry.second) { // Iterate through all windows of a process
                        if (window.checked) { // Check if the window is selected
			    MoveWindowToMainMonitor(window.hwnd); // Move the window to the main monitor
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
                Sleep(100); // Short pause
                MinimizeToTray(hwnd); // Minimize the window to the tray
            } else if (id == 2002) { // Check if the ID is 2002 (Close)
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
                    Sleep(100); // Short pause
                    MinimizeToTray(hwnd); // Minimize the window to the tray
                }
            } else if (id == 2003) { // Check if the ID is 2003 (Arrange)
                RECT rect; // Declaration of a RECT structure
                GetClientRect(hwnd, &rect); // Retrieve the client rectangles of the window
                int screenWidth = GetSystemMetrics(SM_CXSCREEN); // Retrieve the screen width
                int screenHeight = GetSystemMetrics(SM_CYSCREEN); // Retrieve the screen height

                int numWindows = 0; // Counter for the number of windows
                for (const auto& entry : processWindowsMap) { // Iterate through all processes
                    for (const auto& window : entry.second) { // Iterate through all windows of a process
                        if (window.checked) { // Check if the window is selected
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
			    MoveWindowToMainMonitor(window.hwnd); // Move the window to the main monitor
                            ShowWindow(window.hwnd, SW_MINIMIZE); // Minimize the window
                            ShowWindow(window.hwnd, SW_RESTORE); // Restore the window
			    SetForegroundWindow(window.hwnd); // Bring the window to the foreground
			    //SetWindowPos(window.hwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE); // Ensure the window is brought to the top of the Z-order
                            ProcessMessages(); // Process messages
                            Sleep(100); // Short pause
                            MoveWindow(window.hwnd, x, y, windowWidth, windowHeight, TRUE); // Move and resize the window
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
                MinimizeToTray(hwnd); // Minimize the window to the tray
                ProcessMessages(); // Process messages
            } else if (id == ID_TRAY_EXIT) { // Check if the ID is for tray exit
                PostQuitMessage(0); // Quit the application
            } else if (id >= 3000 && id < 3000 + processNames.size()) { // Check if the ID is within the range of process names
                std::wstring processName = processNames[id - 3000]; // Retrieve the process name based on the ID
                checkboxState[processName] = !checkboxState[processName]; // Toggle the checkbox state for the process
                for (auto& window : processWindowsMap[processName]) { // Iterate through all windows of the process
                    window.checked = checkboxState[processName]; // Set the checkbox state for each window of the process
                }
                expandedState[processName] = !expandedState[processName]; // Toggle the expanded state for the process
                SCROLLINFO si = {}; // Initialize a SCROLLINFO structure
                si.cbSize = sizeof(si); // Set the size of the SCROLLINFO structure
                si.fMask = SIF_RANGE | SIF_PAGE; // Specify the masks to use
                si.nMin = 0; // Set the minimum scroll range
                si.nMax = 0; // Initialize the maximum scroll range
                for (const auto& processName : processNames) { // Iterate through all process names
                    si.nMax += 30; // Increment the maximum scroll range for each process
                    if (expandedState[processName]) { // Check if the process is expanded
                        si.nMax += processWindowsMap[processName].size() * 30; // Increment the maximum scroll range based on the number of windows
                    }
                }
                si.nPage = 10; // Set the page length for scrolling
                SetScrollInfo(hwnd, SB_VERT, &si, TRUE); // Set the scroll information for the vertical scrollbar
                InvalidateRect(hwnd, NULL, TRUE); // Invalidate and redraw the window
                AdjustWindowSize(hwnd); // Adjust the window size
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
            bool found = false; // Flag to check if an item was found
            HFONT hFont = CreateFont(24, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, TEXT("Segoe UI")); // Create a font
            HDC hdc = GetDC(hwnd); // Retrieve the device context
            HFONT hOldFont = (HFONT)SelectObject(hdc, hFont); // Select the new font and save the old font
            for (size_t i = 0; i < processNames.size() && !found; ++i) { // Iterate through all process names until an item is found
                const auto& processName = processNames[i]; // Retrieve the current process name
                RECT rect = { 30, yPos, 400, yPos + 30 }; // Define a rectangle for the process name
                if (PtInRect(&rect, pt)) { // Check if the cursor is in the rectangle
                    checkboxState[processName] = !checkboxState[processName]; // Toggle the checkbox state
                    for (auto& window : processWindowsMap[processName]) { // Iterate through all windows of the process
                        window.checked = checkboxState[processName]; // Set the checkbox state for each window
                    }
                    InvalidateRect(hwnd, NULL, TRUE); // Invalidate and redraw the window
                    found = true; // Set the flag that an item was found
                    break; // Exit the loop
                }
                yPos += 30; // Increase the y-position
                if (expandedState[processName]) { // Check if the process is expanded
                    for (size_t j = 0; j < processWindowsMap[processName].size(); ++j) { // Iterate through all windows of the process
                        auto& window = processWindowsMap[processName][j]; // Retrieve the current window
                        RECT windowRect = { 50, yPos, 400, yPos + 30 }; // Define a rectangle for the window
                        if (PtInRect(&windowRect, pt)) { // Check if the cursor is in the rectangle
                            window.checked = !window.checked; // Toggle the checkbox state for the window
                            InvalidateRect(hwnd, NULL, TRUE); // Invalidate and redraw the window
                            found = true; // Set the flag that an item was found
                            break; // Exit the loop
                        }
                        yPos += 30; // Increase the y-position
                    }
                }
            }
            SelectObject(hdc, hOldFont); // Restore the old font
            DeleteObject(hFont); // Delete the new font
            ReleaseDC(hwnd, hdc); // Release the device context
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
                    scrollRect.bottom -= 55; // Area without white bar
                    ScrollWindowEx(hwnd, 0, deltaY, &scrollRect, &scrollRect, NULL, NULL, SW_INVALIDATE | SW_ERASE); 
                    InvalidateRect(hwnd, &scrollRect, TRUE); // Invalidate and redraw the rectangle
                }
                InvalidateRect(hwnd, NULL, TRUE); // Invalidate and redraw the rectangle
                lastMousePos = currentMousePos; // Update the last mouse position
  	  } else {
        	POINT pt; 
	        GetCursorPos(&pt); 
	        ScreenToClient(hwnd, &pt); 
	
	        int yPos = 0 - scrollPos;
	
                for (size_t i = 0; i < processNames.size(); ++i) {
                    const auto& processName = processNames[i];
                    RECT rect = { 30, yPos, 400, yPos + 30 };
                    if (PtInRect(&rect, pt)) {
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
                        yPos += processWindowsMap[processName].size() * 30;
                    }
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
                scrollRect.bottom -= 55; // Bereich ohne grauen Balken
                ScrollWindowEx(hwnd, 0, delta / WHEEL_DELTA * 30, &scrollRect, &scrollRect, NULL, NULL, SW_INVALIDATE | SW_ERASE); 
                InvalidateRect(hwnd, &scrollRect, TRUE); // Rechteck ungültig machen und neu zeichnen
            }
        }
        break;
        
        case WM_PAINT: { // Message when the window needs to be repainted
            PAINTSTRUCT ps; // Declaration of a PAINTSTRUCT structure
            HDC hdc = BeginPaint(hwnd, &ps); // Begin the painting process and retrieve the device context
            int yPos = 0 - scrollPos; // Initialize the y-position based on the scroll position
            HFONT hFont = CreateFont(24, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, TEXT("Segoe UI Symbol")); // Create a font
            HFONT hOldFont = (HFONT)SelectObject(hdc, hFont); // Select the new font and save the old font

            RECT clientRect; // Declaration of a RECT structure to store the client rectangles
            GetClientRect(hwnd, &clientRect); // Retrieve the client rectangles of the window
            int scrollbarWidth = GetSystemMetrics(SM_CXVSCROLL); // Retrieve the width of the vertical scrollbar
            int textWidth = clientRect.right - scrollbarWidth - 30; // Calculate the width of the text area

            for (size_t i = 0; i < processNames.size(); ++i) { // Iterate through all process names
                const auto& processName = processNames[i]; // Retrieve the current process name
                std::wstring text = L"  " + std::wstring(checkboxState[processName] ? L"\u2611 " : L"\u2610 ") + std::wstring(processName.begin(), processName.end()); // Create the text with checkbox
                RECT rect = { 30, yPos, textWidth, yPos + 30 }; // Define a rectangle for the text
                SetTextColor(hdc, RGB(0, 0, 0)); // Set the text color to black
                DrawTextW(hdc, text.c_str(), -1, &rect, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS); // Draw the text
                SetWindowPos(expandButtons[processName], NULL, 10, yPos, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW); // Position the expand button
                yPos += 30; // Increase the y-position
                if (expandedState[processName]) { // Check if the process is expanded
                    for (size_t j = 0; j < processWindowsMap[processName].size(); ++j) { // Iterate through all windows of the process
                        const auto& window = processWindowsMap[processName][j]; // Retrieve the current window
                        std::wstring windowText = L"  " + std::wstring(window.checked ? L"\u2611 " : L"\u2610 ") + std::wstring(window.title.begin(), window.title.end()); // Create the text with checkbox for the window
                        RECT windowRect = { 50, yPos, textWidth, yPos + 30 }; // Define a rectangle for the window text
                        SetTextColor(hdc, RGB(0, 0, 255)); // Set the text color to blue
                        DrawTextW(hdc, windowText.c_str(), -1, &windowRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS); // Draw the text for the window
                        yPos += 30; // Increase the y-position
                    }
                }
            }
            yPos += 30; // Increase the y-position
            yPos += 30; // Increase the y-position
            SelectObject(hdc, hOldFont); // Restore the old font
            DeleteObject(hFont); // Delete the new font
            EndPaint(hwnd, &ps); // End the painting process
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

            int buttonCount = 4; // Number of buttons
            int buttonWidth = (width - 20) / buttonCount; // Calculate the width of a button

            // Position the buttons for minimize, restore, close, and arrange
            SetWindowPos(minimizeButton, NULL, 10, yPos + 5 + 5, buttonWidth, 40, SWP_NOZORDER);
            SetWindowPos(restoreButton, NULL, 10 + buttonWidth + 5, yPos + 5 + 5, buttonWidth, 40, SWP_NOZORDER);
            SetWindowPos(closeButton, NULL, 10 + 2 * (buttonWidth + 5), yPos + 5 + 5, buttonWidth, 40, SWP_NOZORDER);
            SetWindowPos(arrangeButton, NULL, 10 + 3 * (buttonWidth + 5), yPos + 5 + 5, buttonWidth, 40, SWP_NOZORDER);
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

        case WM_TRAYICON: { // Message when interacting with the tray icon
            if (lParam == WM_LBUTTONUP) { // Check if the left mouse button was released
                RefreshWindowList(hwnd); // Refresh the window list
                ShowWindow(hwnd, SW_RESTORE); // Restore the window
                SaveCurrentWindows(); // Save the current windows
                AdjustWindowSize(hwnd); // Adjust the window size
                SetForegroundWindow(hwnd); // Bring the window to the foreground
            } else if (lParam == WM_RBUTTONUP) { // Check if the right mouse button was released
                ShowTrayMenu(hwnd); // Show the tray menu
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

    const char CLASS_NAME[] = "SampleWindowClass"; // Define the class name for the window

    WNDCLASSA wc = {}; // Initialize a WNDCLASSA structure
    wc.lpfnWndProc = WindowProc; // Set the window procedure
    wc.hInstance = hInstance; // Set the instance handle
    wc.lpszClassName = CLASS_NAME; // Set the class name
    wc.hCursor = LoadCursor(NULL, IDC_ARROW); // Load the default cursor (arrow)
    wc.hbrBackground = CreateSolidBrush(RGB(255, 255, 255)); // Create a white background brush
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MYICON)); // Load the window icon

    RegisterClassA(&wc); // Register the window class

    HWND hwnd = CreateWindowExA(
        0, // Extended window style
        CLASS_NAME, // Name of the window class
        "Manage Multiple Open Windows", // Window title
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