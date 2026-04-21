#include "display/ConsoleDisplay.hpp"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <Windows.h>

ConsoleDisplay::ConsoleDisplay() {
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	DWORD mode = 0;
	GetConsoleMode(hOut, &mode);
	SetConsoleMode(hOut, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
}

void ConsoleDisplay::moveCursorToTop() {
	std::cout << "\033[H";
}

void ConsoleDisplay::clear() {
	std::cout << "\033[2J\033[H";
}

void ConsoleDisplay::render(
    const CollectorData& sysData,
    const std::map<std::string, float>& procNet,
    const AnalysisResult& result)
{
    if (m_firstRender) {
        clear();
        m_firstRender = false;
    }
    else {
        moveCursorToTop();
    }

    SYSTEMTIME st;
    GetLocalTime(&st);
    char timeBuf[16];
    sprintf_s(timeBuf, "%02d:%02d:%02d", st.wHour, st.wMinute, st.wSecond);

    // 각 줄 끝에 \033[K 추가 — 커서 이후 줄 나머지 지우기
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode = 0;
    GetConsoleMode(hOut, &mode);
    SetConsoleMode(hOut, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);

    std::cout << "[ Rastreador ]  " << timeBuf << "\033[K\n";
    std::cout << "─────────────────────────────────────────\033[K\n";

    std::cout << std::fixed << std::setprecision(1);
    std::cout << "NET  " << std::setw(7) << sysData.netSpeed << " MB/s\033[K\n";

    for (auto const& [name, mbps] : procNet) {
        if (mbps < 0.01f) continue;
        std::cout << "  ▶ " << std::left << std::setw(20) << name
            << std::right << std::setw(6) << std::setprecision(2)
            << mbps << " MB/s\033[K\n";
    }

    std::cout << "─────────────────────────────────────────\033[K\n";
    std::cout << "CPU  " << std::setw(6) << std::setprecision(1)
        << sysData.cpuUsage << " %\033[K\n";
    std::cout << "MEM  " << std::setw(6) << std::setprecision(1)
        << sysData.memUsage << " GB\033[K\n";
    std::cout << "─────────────────────────────────────────\033[K\n";
    std::cout << "  ▶ " << result.message << "\033[K\n";

    std::cout << "\033[J";
    std::cout.flush();
}