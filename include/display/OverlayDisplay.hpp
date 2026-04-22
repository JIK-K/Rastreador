#pragma once
#include "display/IDisplay.hpp"
#include <Windows.h>
#include <gdiplus.h>
#include <map>
#include <string>
#include <thread>
#include <atomic>
#include <mutex>

#pragma comment(lib, "gdiplus.lib")

class OverlayDisplay : public IDisplay {
public:
	OverlayDisplay();
	~OverlayDisplay();

	OverlayDisplay(const OverlayDisplay&) = delete;
	OverlayDisplay& operator=(const OverlayDisplay&) = delete;

	void render(const CollectorData& sysData,
		const std::map<std::string, float>& procNet,
		const std::map<std::string, ProcessInfo>& procInfo,
		const AnalysisResult& result) override;

private:
	void createWindow();
	void messageLoop();
	void drawText(Gdiplus::Graphics& g, const std::wstring& text,
		float x, float y, Gdiplus::Color color, float fontSize = 14.0f);

	static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
private:
	HWND m_hwnd = nullptr;
	ULONG_PTR m_gdiplusToken = 0;

	std::thread m_msgThread;
	std::atomic<bool> m_running{ false };

	std::map<std::string, float> m_procNet;
	std::mutex m_dataMutex;

	static constexpr int OVERLAY_WIDTH = 240;
	static constexpr int OVERLAY_HEIGHT = 200;
};