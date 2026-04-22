#pragma once
#include "collector/SystemMonitor.hpp"
#include "collector/ProcessMonitor.hpp"
#include "analyzer/BottleneckAnalyzer.hpp"
#include "display/IDisplay.hpp"
#include "tray/TrayIcon.hpp"
#include <thread>
#include <mutex>
#include <atomic>

class Monitor {
public:
	Monitor(IDisplay* display, float maxNetSpeed = 100.0f);
	~Monitor();

	Monitor(const Monitor&) = delete;
	Monitor& operator=(const Monitor&) = delete;

	//Monitor(Monitor&&) = delete;
	//Monitor& operator=(Monitor&&) = delete;

	void start();
	void stop();

private:
	void collectLoop();
	void displayLoop();
private:
	SystemMonitor m_sysMon;
	ProcessMonitor m_procMon;
	BottleneckAnalyzer m_analyzer;
	IDisplay* m_display;

	float m_maxNetSpeed;

	CollectorData m_sysData;
	std::map<std::string, float> m_procNet;
	std::map<std::string, ProcessInfo> m_procInfo;
	AnalysisResult m_result;

	std::mutex m_mutex;
	std::atomic<bool> m_running{ false };
	std::thread m_collectThread;
	std::thread m_displayThread;

	IDisplay* m_overlayDisplay = nullptr;
	TrayIcon* m_tray = nullptr;
	std::thread   m_trayThread;
};
