
#include <windows.h>
#include <objbase.h>
#include "MainWindow.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    if (FAILED(hr)) {
        return 1;
    }

    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_BAR_CLASSES;
    InitCommonControlsEx(&icex);

    MainWindow window;
    if (!window.Create(hInstance)) {
        CoUninitialize();
        return 1;
    }

    window.Show(nCmdShow);
    window.RunMessageLoop();

    CoUninitialize();
    return 0;
}
