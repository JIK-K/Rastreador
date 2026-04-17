#include <iostream>
#include <thread>
#include <chrono>
#include <iomanip> 
#include "collector/SystemMonitor.hpp"
#include "collector/ProcessMonitor.hpp"

int main() {
    SystemMonitor sysMon;
    ProcessMonitor procMon;

    std::cout << "======================================================" << std::endl;
    std::cout << "       Download Monitor 통합 수집 테스트 시작          " << std::endl;
    std::cout << "======================================================" << std::endl;

    while (true) {
        sysMon.collect();
        procMon.collect();

        CollectorData sysData = sysMon.getData();
        auto procNetMap = procMon.getProcessNetUsage();

        // 2. 시스템 전체 정보 출력
        std::cout << "\n[시스템 상태] "
            << "CPU: " << std::fixed << std::setprecision(1) << sysData.cpuUsage << "% | "
            << "MEM: " << sysData.memUsage << "GB | "
            << "NET 합계: " << sysData.netSpeed << "MB/s" << std::endl;

        // 3. 네트워크 사용 중인 프로세스 목록 출력
        std::cout << "[통신 중인 앱 목록]" << std::endl;
        if (procNetMap.empty()) {
            std::cout << "  - 현재 통신 중인 프로세스 없음" << std::endl;
        }
        else {
            int count = 0;
            for (auto const& [name, usage] : procNetMap) {
                // 너무 많으면 지저분하니까 상위 몇 개만 보거나, 
                // 지금은 표시용 1.0f가 들어간 놈들을 다 출력해봅니다.
                std::cout << "  ▶ " << std::left << std::setw(20) << name;
                if (++count % 3 == 0) std::cout << "\n"; // 3개마다 줄바꿈
            }
        }
        std::cout << "\n------------------------------------------------------" << std::endl;

        // 1초 대기
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    return 0;
}