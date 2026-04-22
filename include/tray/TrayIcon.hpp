#pragma once
#include <Windows.h>
#include <string>
#include <functional>

class TrayIcon {
public:
	TrayIcon(std::function<void()> onClicked);
	~TrayIcon();

	TrayIcon(const TrayIcon&) = delete;
	TrayIcon& operator=(const TrayIcon&) = delete;

	void onCreate();
	void onDestroy();
	HWND getHwnd() const { return m_hwnd; }
private:
	static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

	HWND m_hwnd = nullptr;
	NOTIFYICONDATAW m_nid = {};
	std::function<void()> m_onClicked;

	static TrayIcon* s_instance;
	static constexpr UINT WM_TRAYICON = WM_USER + 1;
};