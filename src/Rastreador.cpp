#include <iostream>
#include <thread>
#include <chrono>
#include <iomanip> 
#include "collector/SystemMonitor.hpp"
#include "collector/ProcessMonitor.hpp"
#include "analyzer/BottleneckAnalyzer.hpp"

int main() {
    SystemMonitor sysMon;
    ProcessMonitor procMon;
    BottleneckAnalyzer analyzer;
    
    float maxNetSpeed = 100.0f;

    std::this_thread::sleep_for(std::chrono::milliseconds(2000));

    std::cout << "======================================================" << std::endl;
    std::cout << "         Rastreador - 다운로드 모니터                  " << std::endl;
    std::cout << "======================================================" << std::endl;

    while (true) {
        sysMon.collect();
        procMon.collect();

        CollectorData sysData = sysMon.getData();
        auto procNetMap = procMon.getProcessNetUsage();

        AnalysisResult result = analyzer.analyze(sysData, procNetMap, maxNetSpeed);

        std::cout << "\n[시스템 상태] "
            << "CPU: " << std::fixed << std::setprecision(1) << sysData.cpuUsage << "% | "
            << "MEM: " << sysData.memUsage << "GB | "
            << "NET: " << sysData.netSpeed << "MB/s" << std::endl;

        std::cout << "[프로세스별 수신량]" << std::endl;
        if (procNetMap.empty()) {
            std::cout << "  - 없음" << std::endl;
        }
        else {
            for (auto const& [name, mbps] : procNetMap) {
                std::cout << "  " << std::left << std::setw(25) << name
                    << std::fixed << std::setprecision(2)
                    << mbps << " MB/s" << std::endl;
            }
        }
        std::cout << "[판정] " << result.message << std::endl;
        std::cout << "------------------------------------------------------" << std::endl;

        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    return 0;
}