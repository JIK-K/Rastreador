#include "display/OverlayDisplay.hpp"
#include <sstream>
#include <iomanip>

#pragma comment(lib, "gdiplus.lib")

OverlayDisplay::OverlayDisplay() {
	Gdiplus::GdiplusStartupInput input;
	Gdiplus::GdiplusStartup(&m_gdiplusToken, &input, nullptr);

	m_running = true;
	m_msgThread = std::thread(&OverlayDisplay::messageLoop, this);

	std::this_thread::sleep_for(std::chrono::milliseconds(500));
}

OverlayDisplay::~OverlayDisplay() {
	m_running = false;
	if (m_hwnd) PostMessage(m_hwnd, WM_QUIT, 0, 0);
	if (m_msgThread.joinable()) m_msgThread.join();
	Gdiplus::GdiplusShutdown(m_gdiplusToken);
}

void OverlayDisplay::createWindow() {
	HINSTANCE hInst = GetModuleHandle(nullptr);

	WNDCLASSEX wc = {};
	wc.cbSize = sizeof(wc);
	wc.lpfnWndProc = WndProc;
	wc.hInstance = hInst;
	wc.lpszClassName = L"RastreadorOverlay";
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	RegisterClassEx(&wc);

	// 화면 우상단 위치 계산
	int screenW = GetSystemMetrics(SM_CXSCREEN);
	int x = screenW - OVERLAY_WIDTH - 10;
	int y = 10;

	m_hwnd = CreateWindowEx(
		WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
		L"RastreadorOverlay",
		L"Rastreador",
		WS_POPUP,
		x, y, OVERLAY_WIDTH, OVERLAY_HEIGHT,
		nullptr, nullptr, hInst, nullptr
	);

	// 검정 배경을 투명으로
	SetLayeredWindowAttributes(m_hwnd, RGB(0, 0, 0), 0, LWA_COLORKEY);

	ShowWindow(m_hwnd, SW_SHOW);
	UpdateWindow(m_hwnd);
}

void OverlayDisplay::messageLoop() {
	createWindow();

	MSG msg;
	while (m_running && GetMessage(&msg, nullptr, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

LRESULT CALLBACK OverlayDisplay::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	if (msg == WM_DESTROY) {
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

void OverlayDisplay::drawText(Gdiplus::Graphics& g,
	const std::wstring& text,
	float x, float y,
	Gdiplus::Color color,
	float fontSize)
{
	//NanumGothic
	Gdiplus::Font font(L"Malgun Gothic", fontSize, Gdiplus::FontStyleRegular);
	Gdiplus::SolidBrush brush(color);
	Gdiplus::PointF pos(x, y);
	g.DrawString(text.c_str(), -1, &font, pos, &brush);
}

void OverlayDisplay::render(const CollectorData& sysData,
	const std::map<std::string, float>& procNet,
	const std::map<std::string, ProcessInfo>& procInfo,
	const AnalysisResult& result)
{
	if (!m_hwnd) return;

	{
		std::lock_guard<std::mutex> lock(m_dataMutex);
		m_procNet = procNet;
	}

	// 창 다시 그리기
	HDC hdc = GetDC(m_hwnd);
	HDC memDC = CreateCompatibleDC(hdc);
	HBITMAP memBmp = CreateCompatibleBitmap(hdc, OVERLAY_WIDTH, OVERLAY_HEIGHT);
	SelectObject(memDC, memBmp);

	// 검정 배경 투명색
	RECT rc = { 0, 0, OVERLAY_WIDTH, OVERLAY_HEIGHT };
	HBRUSH blackBrush = CreateSolidBrush(RGB(0, 0, 0));
	FillRect(memDC, &rc, blackBrush);
	DeleteObject(blackBrush);

	// GDI Text
	Gdiplus::Graphics g(memDC);;
	g.SetTextRenderingHint(Gdiplus::TextRenderingHintAntiAlias);

	float y = 8.0f;
	Gdiplus::Color white(255, 255, 255);
	Gdiplus::Color green(100, 255, 100);
	Gdiplus::Color yellow(255, 220, 50);
	Gdiplus::Color red(255, 80, 80);

	// Network
	std::wostringstream oss;
	oss << std::fixed << std::setprecision(1);
	oss << L"NET  " << sysData.netSpeed << L" MB/s";
	drawText(g, oss.str(), 8, y, white);
	y += 18;

	// Process
	{
		std::lock_guard<std::mutex> lock(m_dataMutex);
		for (auto const& [name, mbps] : m_procNet) {
			if (mbps < 0.01f) continue;
			std::wstring wname(name.begin(), name.end());
			std::wostringstream ps;
			ps << std::fixed << std::setprecision(2);
			ps << L"  " << wname << L" " << mbps << L"MB/s";
			drawText(g, ps.str(), 8, y, yellow, 11.0f);
			y += 15;
		}
	}

	// CPU / MEM
	oss.str(L"");
	oss << L"CPU  " << sysData.cpuUsage << L" %";
	drawText(g, oss.str(), 8, y, white); 
	y += 18;

	if (!procInfo.empty()) {
		auto topCpu = std::max_element(procInfo.begin(), procInfo.end(),
			[](const auto& a, const auto& b) {
				return a.second.cpuUsage < b.second.cpuUsage;
			});
		if (topCpu->second.cpuUsage > 0.1f) {
			std::wstring wname(topCpu->first.begin(), topCpu->first.end());
			std::wostringstream cs;
			cs << std::fixed << std::setprecision(1);
			cs << L"  " << wname << L" " << topCpu->second.cpuUsage << L"%";
			drawText(g, cs.str(), 8, y, yellow, 11.0f);
			y += 15;
		}
	}

	oss.str(L"");
	oss << L"MEM  " << sysData.memUsage << L" GB";
	drawText(g, oss.str(), 8, y, white);
	y += 18;

	if (!procInfo.empty()) {
		auto topMem = std::max_element(procInfo.begin(), procInfo.end(),
			[](const auto& a, const auto& b) {
				return a.second.memUsage < b.second.memUsage;
			});
		std::wstring wname(topMem->first.begin(), topMem->first.end());
		std::wostringstream ms;
		ms << std::fixed << std::setprecision(0);
		ms << L"  " << wname << L" " << topMem->second.memUsage << L"MB";
		drawText(g, ms.str(), 8, y, yellow, 11.0f);
		y += 15;
	}

	std::wstring wmsg(result.message.begin(), result.message.end());
	Gdiplus::Color msgColor = (result.state == AnalysisResult::State::NORMAL)
		? green : red;
	drawText(g, wmsg, 8, y, msgColor, 11.0f);

	// memDC -> hdc 복사
	BitBlt(hdc, 0, 0, OVERLAY_WIDTH, OVERLAY_HEIGHT, memDC, 0, 0, SRCCOPY);

	DeleteObject(memBmp);
	DeleteDC(memDC);
	ReleaseDC(m_hwnd, hdc);
}

