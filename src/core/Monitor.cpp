#include "core/Monitor.hpp"
#include <chrono>

Monitor::Monitor(IDisplay* display, float maxNetSpeed) : 
	m_display(display), m_maxNetSpeed(maxNetSpeed) {}

Monitor::~Monitor() {
	stop();
}

void Monitor::start() {
	m_running = true;

	std::this_thread::sleep_for(std::chrono::milliseconds(2000));

	m_collectThread = std::thread(&Monitor::collectLoop, this);
	m_displayThread = std::thread(&Monitor::displayLoop, this);

	m_collectThread.join();
	m_displayThread.join();
}

void Monitor::stop() {
	m_running = false;
}

void Monitor::collectLoop() {
	while (m_running) {
		m_sysMon.collect();
		m_procMon.collect();

		CollectorData sysData = m_sysMon.getData();
		auto procNet = m_procMon.getProcessNetUsage();
		auto procInfo = m_procMon.getProcessInfo();
		AnalysisResult result = m_analyzer.analyze(sysData, procNet, m_maxNetSpeed);

		{
			std::lock_guard<std::mutex> lock(m_mutex);
			m_sysData = sysData;
			m_procNet = procNet;
			m_procInfo = procInfo;
			m_result = result;
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}
}

void Monitor::displayLoop() {
	while (m_running) {
		CollectorData sysData;
		std::map<std::string, float> procNet;
		std::map<std::string, ProcessInfo> procInfo;
		AnalysisResult result;

		{
			std::lock_guard<std::mutex> lock(m_mutex);
			sysData = m_sysData;
			procNet = m_procNet;
			procInfo = m_procInfo;
			result = m_result;
		}

		m_display->render(sysData, procNet, procInfo, result);

		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}
}