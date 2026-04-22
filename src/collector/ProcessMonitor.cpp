#include "collector/ProcessMonitor.hpp"
#include <Psapi.h>
#include <TlHelp32.h>
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
    // 시스템 전체 CPU 시간
    FILETIME idleTime, kernelTime, userTime;
    GetSystemTimes(&idleTime, &kernelTime, &userTime);

    ULONGLONG sysKernel = ((ULONGLONG)kernelTime.dwHighDateTime << 32) | kernelTime.dwLowDateTime;
    ULONGLONG sysUser = ((ULONGLONG)userTime.dwHighDateTime << 32) | userTime.dwLowDateTime;
    ULONGLONG sysTotalNow = sysKernel + sysUser;
    ULONGLONG sysDelta = sysTotalNow - m_prevSysTime;

    // ETW Network
    auto pidBytes = m_etw.getAndResetBytes();

    std::map<std::string, float>       newNetMap;
    std::map<std::string, ProcessInfo> newProcInfo;

    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnap != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32W pe = {};
        pe.dwSize = sizeof(pe);

        if (Process32FirstW(hSnap, &pe)) {
            do {
                DWORD pid = pe.th32ProcessID;
                if (pid == 0 || pid == 4) continue;

                HANDLE hProc = OpenProcess(
                    PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
                if (!hProc) continue;

                std::wstring wname(pe.szExeFile);
                std::string  name(wname.begin(), wname.end());

                // 메모리 수집
                PROCESS_MEMORY_COUNTERS_EX pmc = {};
                if (GetProcessMemoryInfo(hProc, (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc))) {
                    float memMB = static_cast<float>(pmc.WorkingSetSize) / (1024.0f * 1024.0f);
                    newProcInfo[name].memUsage += memMB;
                }

                // CPU 수집
                FILETIME ct, et, kt, ut;
                if (GetProcessTimes(hProc, &ct, &et, &kt, &ut)) {
                    ULONGLONG procKernel = ((ULONGLONG)kt.dwHighDateTime << 32) | kt.dwLowDateTime;
                    ULONGLONG procUser = ((ULONGLONG)ut.dwHighDateTime << 32) | ut.dwLowDateTime;
                    ULONGLONG procTotal = procKernel + procUser;

                    if (m_prevSysTime > 0 && m_prevCpuTime.count(pid) && sysDelta > 0) {
                        ULONGLONG procDelta = procTotal - m_prevCpuTime[pid];
                        int numCores = std::thread::hardware_concurrency();
                        float cpuPct = 100.0f * (float)procDelta / (float)sysDelta * numCores;
                        newProcInfo[name].cpuUsage += cpuPct;
                    }
                    m_prevCpuTime[pid] = procTotal;
                }

                CloseHandle(hProc);
            } while (Process32NextW(hSnap, &pe));
        }
        CloseHandle(hSnap);
    }

    // 네트워크 수집
    for (auto const& [pid, bytes] : pidBytes) {
        if (bytes == 0) continue;
        std::string name = getProcessName(pid);
        float mbps = static_cast<float>(bytes) / (1024.0f * 1024.0f);
        newNetMap[name] += mbps;
    }

    m_prevSysTime = sysTotalNow;

    std::lock_guard<std::mutex> lock(m_mutex);
    m_netPerProc = std::move(newNetMap);
    m_procInfo = std::move(newProcInfo);
}

std::map<std::string, float> ProcessMonitor::getProcessNetUsage() const {
    std::lock_guard<std::mutex> lock(m_mutex); 
    return m_netPerProc;
}

std::map<std::string, ProcessInfo> ProcessMonitor::getProcessInfo() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_procInfo;
}

CollectorData ProcessMonitor::getData() const {
    CollectorData data = {};
    return data;
}