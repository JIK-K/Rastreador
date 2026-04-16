#include "collector/SystemMonitor.hpp"
#include <iostream>

// 1. 생성자: 객체가 태어날 때 시스템에 빨대를 꽂습니다.
SystemMonitor::SystemMonitor() {
    queryPDH();
}

// 2. 소멸자: 객체가 죽을 때 빌려온 장바구니를 반납합니다.
SystemMonitor::~SystemMonitor() {
    if (m_hQuery) {
        PdhCloseQuery(m_hQuery);
    }
}

// 3. 초기화 로직: 어떤 데이터를 가져올지 주소를 등록합니다.
void SystemMonitor::queryPDH() {
    // 장바구니(Query)를 먼저 엽니다.
    if (PdhOpenQuery(NULL, NULL, &m_hQuery) != ERROR_SUCCESS) {
        return;
    }

    // CPU 전체 사용량 빨대를 꽂습니다.
    PdhAddCounter(m_hQuery, "\\Processor(_Total)\\% Processor Time", NULL, &m_CpuCounter);

    // 네트워크 수신 속도 빨대를 꽂습니다. (와일드카드 *는 모든 랜카드를 뜻함)
    PdhAddCounter(m_hQuery, "\\Network Interface(*)\\Bytes Received/sec", NULL, &m_NetCounter);

    // 기준점(T1)을 잡기 위해 첫 번째 수집을 실행합니다.
    PdhCollectQueryData(m_hQuery);
}

// 4. 수집 로직: 1초마다 호출되어 실제 숫자를 갱신합니다.
void SystemMonitor::collect() {
    // 최신 데이터로 갱신 (T2, T3... 시점 데이터 수집)
    if (PdhCollectQueryData(m_hQuery) != ERROR_SUCCESS) return;

    // --- CPU 데이터 읽기 ---
    PDH_FMT_COUNTERVALUE cpuVal;
    PdhGetFormattedCounterValue(m_CpuCounter, PDH_FMT_DOUBLE, NULL, &cpuVal);
    m_data.cpuUsage = static_cast<float>(cpuVal.doubleValue);

    // --- 메모리 데이터 읽기 (이건 PDH보다 GlobalMemoryStatusEx가 더 정확하고 빠름) ---
    MEMORYSTATUSEX memStatus;
    memStatus.dwLength = sizeof(MEMORYSTATUSEX);
    if (GlobalMemoryStatusEx(&memStatus)) {
        // (전체 물리 메모리 - 가용 물리 메모리) / 1024^3 = GB 단위 사용량
        DWORDLONG usedBytes = memStatus.ullTotalPhys - memStatus.ullAvailPhys;
        m_data.memUsage = static_cast<float>(usedBytes) / (1024.0f * 1024.0f * 1024.0f);
    }

    // --- 네트워크 데이터 읽기 ---
    PDH_FMT_COUNTERVALUE netVal;
    PdhGetFormattedCounterValue(m_NetCounter, PDH_FMT_DOUBLE, NULL, &netVal);
    // Bytes/sec -> MB/s 변환 (1024^2로 나눔)
    m_data.netSpeed = static_cast<float>(netVal.doubleValue) / (1024.0f * 1024.0f);
}

// 5. 반환 로직: 수집된 바구니를 그대로 넘겨줍니다.
CollectorData SystemMonitor::getData() const {
    return m_data;
}