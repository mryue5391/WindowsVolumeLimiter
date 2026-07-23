
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <windows.h>
#include <commctrl.h>
#include <thread>
#include <atomic>
#include "AudioController.h"

class MainWindow {
public:
    MainWindow();
    ~MainWindow();

    bool Create(HINSTANCE hInstance);
    void Show(int nCmdShow);
    void RunMessageLoop();

private:
    static LRESULT CALLBACK StaticWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    void InitializeControls();
    void UpdateCurrentVolumeLabel();
    void UpdateTargetVolumeLabel();
    void OnLockButtonClick();
    void LockVolume();
    void UnlockVolume();
    void MonitorThreadFunc();

    HWND m_hWnd;
    HWND m_hTrackbar;
    HWND m_hTargetLabel;
    HWND m_hCurrentLabel;
    HWND m_hIntervalEdit;
    HWND m_hLockButton;
    HWND m_hIntervalLabel;

    AudioController m_audioController;

    std::atomic<bool> m_isLocked;
    std::atomic<int> m_targetVolume;
    std::atomic<int> m_monitorInterval;
    std::thread m_monitorThread;
    HANDLE m_hStopEvent;
};

#endif
