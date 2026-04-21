#include "collector/ProcessMonitor.hpp"
#include <Psapi.h>
#include <iostream>

ProcessMonitor::ProcessMonitor() {
    m_etw.start();
}

ProcessMonitor::~ProcessMonitor() {
    m_etw.stop();
}

std::string ProcessMonitor::getProcessName(DWORD pid) {
    if (pid == 0) return "System Idle";
    if (pid == 4) return "System";

    char name[MAX_PATH] = "<unknown>";
    HANDLE hProcess = OpenProcess(
        PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
    if (hProcess) {
        GetModuleBaseNameA(hProcess, nullptr, name, sizeof(name));
        CloseHandle(hProcess);
    }
    return std::string(name);
}

void ProcessMonitor::collect() {
    auto pidBytes = m_etw.getAndResetBytes();

    std::map<std::string, float> newMap;
    for (auto const& [pid, bytes] : pidBytes) {
        if (bytes == 0) continue;
        std::string name = getProcessName(pid);
        float mbps = static_cast<float>(bytes) / (1024.0f * 1024.0f);
        newMap[name] += mbps;
    }

    std::lock_guard<std::mutex> lock(m_mutex);
    m_netPerProc = std::move(newMap);
}

std::map<std::string, float> ProcessMonitor::getProcessNetUsage() const {
    std::lock_guard<std::mutex> lock(m_mutex); 
    return m_netPerProc;
}

CollectorData ProcessMonitor::getData() const {
    CollectorData data = {};
    return data;
}