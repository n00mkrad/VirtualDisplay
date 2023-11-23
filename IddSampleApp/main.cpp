#include <windows.h>
#include <shellapi.h>
#include <iostream>
#include <vector>

#include <swdevice.h>
#include <conio.h>
#include <wrl.h>

#define WM_TRAYICON (WM_USER + 1)
#define ID_EXIT 1001

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
VOID WINAPI CreationCallback(_In_ HSWDEVICE hSwDevice, _In_ HRESULT hrCreateResult, _In_opt_ PVOID pContext, _In_opt_ PCWSTR pszDeviceInstanceId);

HSWDEVICE hSwDevice;
HANDLE hEvent;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int)
{
    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"TrayAppClass";

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(0, L"TrayAppClass", L"MiniVirtDisplay Tray", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        NULL, NULL, hInstance, NULL);

    NOTIFYICONDATA nid = {};
    nid.cbSize = sizeof(nid);
    nid.hWnd = hwnd;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = WM_TRAYICON;
    // Set icon and tooltip
    nid.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wcscpy_s(nid.szTip, L"MiniVirtDisplay (Virtual Display Active)");
    Shell_NotifyIcon(NIM_ADD, &nid);

    // Virtual display setup
    hEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    SW_DEVICE_CREATE_INFO createInfo = { 0 };
    PCWSTR description = L"MiniVirtDisplay";
    PCWSTR instanceId = L"MiniVirtDisplay";
    PCWSTR hardwareIds = L"MiniVirtDisplay\0\0";
    PCWSTR compatibleIds = L"MiniVirtDisplay\0\0";

    createInfo.cbSize = sizeof(createInfo);
    createInfo.pszzCompatibleIds = compatibleIds;
    createInfo.pszInstanceId = instanceId;
    createInfo.pszzHardwareIds = hardwareIds;
    createInfo.pszDeviceDescription = description;

    createInfo.CapabilityFlags = SWDeviceCapabilitiesRemovable | SWDeviceCapabilitiesSilentInstall | SWDeviceCapabilitiesDriverRequired;

    HRESULT hr = SwDeviceCreate(L"MiniVirtDisplay", L"HTREE\\ROOT\\0", &createInfo, 0, nullptr, CreationCallback, &hEvent, &hSwDevice);

    if (FAILED(hr))
    {
        MessageBox(NULL, L"SwDeviceCreate failed", L"MiniVirtDisplay Error", MB_OK | MB_ICONERROR);
        return 1;
    }

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    Shell_NotifyIcon(NIM_DELETE, &nid);
    return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_TRAYICON:
        if (lParam == WM_RBUTTONUP)
        {
            HMENU hMenu = CreatePopupMenu();
            InsertMenu(hMenu, 0, MF_BYPOSITION | MF_STRING, ID_EXIT, L"Destroy Virtual Display and Exit");

            POINT pt;
            GetCursorPos(&pt);
            SetForegroundWindow(hwnd);
            TrackPopupMenu(hMenu, TPM_BOTTOMALIGN | TPM_LEFTALIGN, pt.x, pt.y, 0, hwnd, NULL);
            PostMessage(hwnd, WM_NULL, 0, 0);

            DestroyMenu(hMenu);
        }
        break;
    case WM_COMMAND:
        if (LOWORD(wParam) == ID_EXIT)
        {
            SwDeviceClose(hSwDevice);
            PostQuitMessage(0);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

VOID WINAPI CreationCallback(_In_ HSWDEVICE hSwDevice, _In_ HRESULT hrCreateResult, _In_opt_ PVOID pContext, _In_opt_ PCWSTR pszDeviceInstanceId)
{
    SetEvent(hEvent);
    UNREFERENCED_PARAMETER(hSwDevice);
    UNREFERENCED_PARAMETER(hrCreateResult);
    UNREFERENCED_PARAMETER(pszDeviceInstanceId);
}