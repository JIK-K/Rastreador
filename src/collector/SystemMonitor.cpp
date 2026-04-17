#include "collector/SystemMonitor.hpp"
#include <iostream>

SystemMonitor::SystemMonitor() {
    queryPDH();
}

SystemMonitor::~SystemMonitor() {
    if (m_hQuery) {
        PdhCloseQuery(m_hQuery);
    }
}

void SystemMonitor::queryPDH() {
    if (PdhOpenQuery(NULL, NULL, &m_hQuery) != ERROR_SUCCESS) {
        return;
    }

    // CPU 전체 사용량.
    PdhAddCounter(m_hQuery, "\\Processor(_Total)\\% Processor Time", NULL, &m_CpuCounter);

    // 네트워크 수신 속도 (와일드카드 *는 모든 랜카드를 뜻함)
    PdhAddCounter(m_hQuery, "\\Network Interface(*)\\Bytes Received/sec", NULL, &m_NetCounter);

    // 기준점(T1)을 잡기 위해 첫 번째 수집을 실행합
    PdhCollectQueryData(m_hQuery);
}

void SystemMonitor::collect() {
    if (PdhCollectQueryData(m_hQuery) != ERROR_SUCCESS) return;

    PDH_FMT_COUNTERVALUE cpuVal;
    PdhGetFormattedCounterValue(m_CpuCounter, PDH_FMT_DOUBLE, NULL, &cpuVal);
    m_data.cpuUsage = static_cast<float>(cpuVal.doubleValue);

    MEMORYSTATUSEX memStatus;
    memStatus.dwLength = sizeof(MEMORYSTATUSEX);
    if (GlobalMemoryStatusEx(&memStatus)) {
        DWORDLONG usedBytes = memStatus.ullTotalPhys - memStatus.ullAvailPhys;
        m_data.memUsage = static_cast<float>(usedBytes) / (1024.0f * 1024.0f * 1024.0f);
    }

    DWORD bufSize = 0;
    DWORD itemCount = 0;
    double totalBytes = 0.0;

    // 필요한 버퍼 크기(bufSize)와 아이템 개수(itemCount)를 먼저 확인
    PdhGetFormattedCounterArray(m_NetCounter, PDH_FMT_DOUBLE, &bufSize, &itemCount, nullptr);

    if (bufSize > 0) {
        BYTE* buffer = new BYTE[bufSize];
        PDH_FMT_COUNTERVALUE_ITEM* items = reinterpret_cast<PDH_FMT_COUNTERVALUE_ITEM*>(buffer);

        if (PdhGetFormattedCounterArray(m_NetCounter, PDH_FMT_DOUBLE, &bufSize, &itemCount, items) == ERROR_SUCCESS) {
            // 모든 네트워크 인터페이스(이더넷, Wi-Fi 등)의 값을 합산
            for (DWORD i = 0; i < itemCount; i++) {
                totalBytes += items[i].FmtValue.doubleValue;
            }
        }

        // MB/s 단위로 변환 (Bytes/sec -> MB/s)
        m_data.netSpeed = static_cast<float>(totalBytes) / (1024.0f * 1024.0f);

        delete[] buffer;
    }
}

// 5. 반환 로직: 수집된 바구니를 그대로 넘겨줍니다.
CollectorData SystemMonitor::getData() const {
    return m_data;
}