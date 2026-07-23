
#include "MainWindow.h"
#include <stdio.h>
#include <string>
#include <algorithm>

#define IDC_TRACKBAR 101
#define IDC_TARGET_LABEL 102
#define IDC_CURRENT_LABEL 103
#define IDC_INTERVAL_EDIT 104
#define IDC_LOCK_BUTTON 105
#define IDC_INTERVAL_LABEL 106
#define ID_TIMER_UPDATE 1001

MainWindow::MainWindow()
    : m_hWnd(nullptr)
    , m_hTrackbar(nullptr)
    , m_hTargetLabel(nullptr)
    , m_hCurrentLabel(nullptr)
    , m_hIntervalEdit(nullptr)
    , m_hLockButton(nullptr)
    , m_hIntervalLabel(nullptr)
    , m_isLocked(false)
    , m_targetVolume(50)
    , m_monitorInterval(500)
    , m_hStopEvent(nullptr)
{
}

MainWindow::~MainWindow()
{
    if (m_isLocked) {
        UnlockVolume();
    }
}

bool MainWindow::Create(HINSTANCE hInstance)
{
    const wchar_t* CLASS_NAME = L"WindowsVolumeLimiterClass";
    // Use a static wchar_t array to ensure string lifetime is valid
    static const wchar_t WINDOW_TITLE[] = L"WindowsVolumeLimiter - created by MrYue_5391";

    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = StaticWndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = CLASS_NAME;

    if (!RegisterClassExW(&wc)) {
        MessageBoxW(nullptr, L"Failed to register window class!", L"Error", MB_ICONERROR);
        return false;
    }

    int windowWidth = 480; // Even wider to fit full text
    int windowHeight = 280;

    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);
    int x = (screenWidth - windowWidth) / 2;
    int y = (screenHeight - windowHeight) / 2;

    m_hWnd = CreateWindowExW(
        0,
        CLASS_NAME,
        WINDOW_TITLE,
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        x, y, windowWidth, windowHeight,
        nullptr,
        nullptr,
        hInstance,
        this
    );

    if (!m_hWnd) {
        MessageBoxW(nullptr, L"Failed to create window!", L"Error", MB_ICONERROR);
        return false;
    }

    // Set the title explicitly just to be 100% sure
    SetWindowTextW(m_hWnd, WINDOW_TITLE);

    if (!m_audioController.Initialize()) {
        DestroyWindow(m_hWnd);
        return false;
    }

    InitializeControls();
    UpdateTargetVolumeLabel();
    UpdateCurrentVolumeLabel();
    SetTimer(m_hWnd, ID_TIMER_UPDATE, 200, nullptr);

    return true;
}

void MainWindow::Show(int nCmdShow)
{
    ShowWindow(m_hWnd, nCmdShow);
    UpdateWindow(m_hWnd);
}

void MainWindow::RunMessageLoop()
{
    MSG msg;
    while (GetMessageW(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
}

LRESULT CALLBACK MainWindow::StaticWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    MainWindow* pThis = nullptr;

    if (uMsg == WM_NCCREATE) {
        CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
        pThis = reinterpret_cast<MainWindow*>(pCreate->lpCreateParams);
        SetWindowLongPtrW(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
        pThis->m_hWnd = hWnd;
    } else {
        pThis = reinterpret_cast<MainWindow*>(GetWindowLongPtrW(hWnd, GWLP_USERDATA));
    }

    if (pThis) {
        return pThis->WndProc(hWnd, uMsg, wParam, lParam);
    }

    return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

LRESULT MainWindow::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {
    case WM_COMMAND: {
        if (LOWORD(wParam) == IDC_LOCK_BUTTON && HIWORD(wParam) == BN_CLICKED) {
            OnLockButtonClick();
        } else if (LOWORD(wParam) == IDC_INTERVAL_EDIT && HIWORD(wParam) == EN_CHANGE) {
            wchar_t buffer[32];
            GetWindowTextW(m_hIntervalEdit, buffer, 32);
            int value = _wtoi(buffer);
            m_monitorInterval = std::clamp(value, 0, 10000);
        }
        break;
    }
    case WM_HSCROLL: {
        if ((HWND)lParam == m_hTrackbar) {
            m_targetVolume = SendMessageW(m_hTrackbar, TBM_GETPOS, 0, 0);
            UpdateTargetVolumeLabel();
        }
        break;
    }
    case WM_CTLCOLORSTATIC: {
        HDC hdcStatic = (HDC)wParam;
        SetBkColor(hdcStatic, GetSysColor(COLOR_WINDOW));
        return (INT_PTR)GetSysColorBrush(COLOR_WINDOW);
    }
    case WM_TIMER: {
        if (wParam == ID_TIMER_UPDATE) {
            UpdateCurrentVolumeLabel();
        }
        break;
    }
    case WM_DESTROY: {
        KillTimer(hWnd, ID_TIMER_UPDATE);
        if (m_isLocked) {
            UnlockVolume();
        }
        m_audioController.Shutdown();
        PostQuitMessage(0);
        break;
    }
    default:
        return DefWindowProcW(hWnd, uMsg, wParam, lParam);
    }
    return 0;
}

void MainWindow::InitializeControls()
{
    HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
    int clientWidth = 480 - 40; // Window width minus left/right padding

    m_hIntervalLabel = CreateWindowExW(
        0,
        L"STATIC",
        L"监控间隔 (ms):",
        WS_CHILD | WS_VISIBLE,
        20, 20, 180, 20,
        m_hWnd,
        (HMENU)IDC_INTERVAL_LABEL,
        (HINSTANCE)GetWindowLongPtrW(m_hWnd, GWLP_HINSTANCE),
        nullptr
    );
    SendMessageW(m_hIntervalLabel, WM_SETFONT, (WPARAM)hFont, TRUE);

    m_hIntervalEdit = CreateWindowExW(
        WS_EX_CLIENTEDGE,
        L"EDIT",
        L"500",
        WS_CHILD | WS_VISIBLE | ES_NUMBER | WS_TABSTOP,
        210, 20, 100, 24,
        m_hWnd,
        (HMENU)IDC_INTERVAL_EDIT,
        (HINSTANCE)GetWindowLongPtrW(m_hWnd, GWLP_HINSTANCE),
        nullptr
    );
    SendMessageW(m_hIntervalEdit, WM_SETFONT, (WPARAM)hFont, TRUE);

    m_hTargetLabel = CreateWindowExW(
        0,
        L"STATIC",
        L"",
        WS_CHILD | WS_VISIBLE,
        20, 60, clientWidth, 20,
        m_hWnd,
        (HMENU)IDC_TARGET_LABEL,
        (HINSTANCE)GetWindowLongPtrW(m_hWnd, GWLP_HINSTANCE),
        nullptr
    );
    SendMessageW(m_hTargetLabel, WM_SETFONT, (WPARAM)hFont, TRUE);

    m_hTrackbar = CreateWindowExW(
        0,
        TRACKBAR_CLASSW,
        L"",
        WS_CHILD | WS_VISIBLE | TBS_HORZ | TBS_AUTOTICKS,
        20, 90, clientWidth, 30,
        m_hWnd,
        (HMENU)IDC_TRACKBAR,
        (HINSTANCE)GetWindowLongPtrW(m_hWnd, GWLP_HINSTANCE),
        nullptr
    );
    SendMessageW(m_hTrackbar, TBM_SETRANGE, TRUE, MAKELONG(0, 100));
    SendMessageW(m_hTrackbar, TBM_SETPOS, TRUE, 50);

    m_hCurrentLabel = CreateWindowExW(
        0,
        L"STATIC",
        L"",
        WS_CHILD | WS_VISIBLE,
        20, 140, clientWidth, 20,
        m_hWnd,
        (HMENU)IDC_CURRENT_LABEL,
        (HINSTANCE)GetWindowLongPtrW(m_hWnd, GWLP_HINSTANCE),
        nullptr
    );
    SendMessageW(m_hCurrentLabel, WM_SETFONT, (WPARAM)hFont, TRUE);

    m_hLockButton = CreateWindowExW(
        0,
        L"BUTTON",
        L"锁定",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,
        20, 180, clientWidth, 40,
        m_hWnd,
        (HMENU)IDC_LOCK_BUTTON,
        (HINSTANCE)GetWindowLongPtrW(m_hWnd, GWLP_HINSTANCE),
        nullptr
    );
    SendMessageW(m_hLockButton, WM_SETFONT, (WPARAM)hFont, TRUE);
}

void MainWindow::UpdateCurrentVolumeLabel()
{
    float volume = m_audioController.GetMasterVolume();
    int volumePercent = static_cast<int>(volume * 100.0f + 0.5f);
    bool isMuted = m_audioController.IsMuted();

    wchar_t buffer[128];
    if (isMuted) {
        swprintf_s(buffer, L"目前音量: %d%% (靜音)", volumePercent);
    } else {
        swprintf_s(buffer, L"目前音量: %d%%", volumePercent);
    }
    SetWindowTextW(m_hCurrentLabel, buffer);
}

void MainWindow::UpdateTargetVolumeLabel()
{
    wchar_t buffer[64];
    swprintf_s(buffer, L"目標音量: %d%%", m_targetVolume.load());
    SetWindowTextW(m_hTargetLabel, buffer);
}

void MainWindow::OnLockButtonClick()
{
    if (!m_isLocked) {
        LockVolume();
    } else {
        UnlockVolume();
    }
}

void MainWindow::LockVolume()
{
    wchar_t intervalBuffer[32];
    GetWindowTextW(m_hIntervalEdit, intervalBuffer, 32);
    int interval = _wtoi(intervalBuffer);
    interval = std::clamp(interval, 0, 10000);
    m_monitorInterval = interval;

    swprintf_s(intervalBuffer, L"%d", interval);
    SetWindowTextW(m_hIntervalEdit, intervalBuffer);

    // Create a manual-reset event, initially non-signaled
    m_hStopEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);

    m_isLocked = true;

    EnableWindow(m_hTrackbar, FALSE);
    EnableWindow(m_hIntervalEdit, FALSE);
    SetWindowTextW(m_hLockButton, L"解鎖");

    m_monitorThread = std::thread(&MainWindow::MonitorThreadFunc, this);
}

void MainWindow::UnlockVolume()
{
    m_isLocked = false;

    // Signal the stop event to wake up the monitor thread immediately
    if (m_hStopEvent != nullptr) {
        SetEvent(m_hStopEvent);
    }

    if (m_monitorThread.joinable()) {
        m_monitorThread.join();
    }

    // Close the event handle
    if (m_hStopEvent != nullptr) {
        CloseHandle(m_hStopEvent);
        m_hStopEvent = nullptr;
    }

    EnableWindow(m_hTrackbar, TRUE);
    EnableWindow(m_hIntervalEdit, TRUE);
    SetWindowTextW(m_hLockButton, L"鎖定");
}

void MainWindow::MonitorThreadFunc()
{
    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);

    AudioController threadAudioController;
    if (!threadAudioController.Initialize()) {
        CoUninitialize();
        return;
    }

    while (m_isLocked) {
        int targetVol = m_targetVolume.load();
        int intervalMs = m_monitorInterval.load();

        float currentVolume = threadAudioController.GetMasterVolume();
        int currentPercent = static_cast<int>(currentVolume * 100.0f + 0.5f);
        bool isMuted = threadAudioController.IsMuted();

        if (currentPercent != targetVol || isMuted) {
            threadAudioController.SetMuted(false);
            threadAudioController.SetMasterVolume(targetVol / 100.0f);
        }

        if (intervalMs > 0) {
            // Wait for either the interval to pass OR the stop event to be signaled
            WaitForSingleObject(m_hStopEvent, intervalMs);
        } else {
            // If interval is 0, just check the event quickly to allow immediate stop
            WaitForSingleObject(m_hStopEvent, 1);
        }
    }

    threadAudioController.Shutdown();
    CoUninitialize();
}
