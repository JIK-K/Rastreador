#pragma once
#include "collector/IDataCollector.hpp"
#include "collector/ETWMonitor.hpp"
#include <map>
#include <string>
#include <vector>
#include <windows.h>
#include <iphlpapi.h>
#include <mutex>

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

    std::map<std::string, float> getProcessNetUsage() const;
    std::map<std::string, ProcessInfo> getProcessInfo() const;

private:
    std::string getProcessName(DWORD pid);

private:
    ETWMonitor m_etw;
    std::map<std::string, float> m_netPerProc;
    std::map<std::string, ProcessInfo> m_procInfo;
    mutable std::mutex m_mutex;

    std::map<DWORD, ULONGLONG> m_prevCpuTime;
    ULONGLONG m_prevSysTime = 0;
};