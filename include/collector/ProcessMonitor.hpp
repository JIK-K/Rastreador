#pragma once

#include "IDataCollector.hpp"
#include <map>
#include <string>
#include <vector>
#include <windows.h>
#include <iphlpapi.h>

class ProcessMonitor : public IDataCollector {
public:
	ProcessMonitor();
	virtual ~ProcessMonitor();

	ProcessMonitor(const ProcessMonitor&) = delete;
	ProcessMonitor& operator=(const ProcessMonitor&) = delete;

	ProcessMonitor(ProcessMonitor&&) = delete;
	ProcessMonitor& operator=(ProcessMonitor&&) = delete;

	void collect() override;
	CollectorData getData() const override;

	std::map<std::string, float> getProcessNetUsage() const {
		return m_netPerProc;
	}
private:
	std::map<std::string, float> m_netPerProc;
	void queryTcpTable();
	std::string getProcessName(DWORD pid);
};