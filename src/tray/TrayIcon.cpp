#include "tray/TrayIcon.hpp"
#include "../resources/resource.h"
#include <iostream>

TrayIcon* TrayIcon::s_instance = nullptr;

TrayIcon::TrayIcon(std::function<void()> onClicked)
    : m_onClicked(onClicked)
{
    s_instance = this;

    HINSTANCE hInst = GetModuleHandle(nullptr);

    WNDCLASSEX wc = {};
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInst;
    wc.lpszClassName = L"RastreadorTray";
    RegisterClassEx(&wc);

    m_hwnd = CreateWindowEx(0, L"RastreadorTray", L"",
        WS_OVERLAPPEDWINDOW, 0, 0, 0, 0,
        nullptr, nullptr, hInst, nullptr);
}

TrayIcon::~TrayIcon() {
    onDestroy();
    if (m_hwnd) DestroyWindow(m_hwnd);
    s_instance = nullptr;
}

void TrayIcon::onCreate() {
    m_nid = {};
    m_nid.cbSize = sizeof(m_nid);
    m_nid.hWnd = m_hwnd;
    m_nid.uID = 1;
    m_nid.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE;
    m_nid.uCallbackMessage = WM_TRAYICON;
    //m_nid.hIcon = LoadIcon(nullptr, IDI_APPLICATION);

    m_nid.hIcon = (HICON)LoadImage(
        GetModuleHandle(nullptr),
        MAKEINTRESOURCE(IDI_ICON1),
        IMAGE_ICON,
        0, 0,
        LR_DEFAULTSIZE
    );
    wcscpy_s(m_nid.szTip, L"Rastreador");

    Shell_NotifyIconW(NIM_ADD, &m_nid);

    m_nid.uVersion = NOTIFYICON_VERSION_4;
    Shell_NotifyIconW(NIM_SETVERSION, &m_nid);

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

void TrayIcon::onDestroy() {
    Shell_NotifyIconW(NIM_DELETE, &m_nid);
}

LRESULT CALLBACK TrayIcon::WndProc(HWND hwnd, UINT msg,
    WPARAM wParam, LPARAM lParam) {
    if (msg == WM_TRAYICON) {
        if (LOWORD(lParam) == WM_RBUTTONUP) {
            // 우클릭 → 컨텍스트 메뉴
            HMENU hMenu = CreatePopupMenu();
            AppendMenuW(hMenu, MF_STRING, 1, L"종료");

            POINT pt;
            GetCursorPos(&pt);
            SetForegroundWindow(hwnd);
            int cmd = TrackPopupMenu(hMenu, TPM_RETURNCMD, pt.x, pt.y, 0, hwnd, nullptr);
            DestroyMenu(hMenu);

            if (cmd == 1) {
                if (s_instance && s_instance->m_onClicked) {
                    s_instance->m_onClicked();
                }
            }
        }
        return 0;
    }
    if (msg == WM_DESTROY) {
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}