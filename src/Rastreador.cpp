#include <iostream>
#include <thread>
#include <chrono>
#include <iomanip> // 출력 포맷팅을 위해 필요
#include "collector/SystemMonitor.hpp"

int main() {
    // 1. 수집기 생성
    SystemMonitor monitor;

    std::cout << "==========================================" << std::endl;
    std::cout << "   System Monitor Test (1초 주기로 수집)   " << std::endl;
    std::cout << "==========================================" << std::endl;
    std::cout << "  CPU(%)  |  MEM(GB)  |  NET(MB/s)  " << std::endl;
    std::cout << "------------------------------------------" << std::endl;

    // 무한 루프로 1초마다 데이터 찍어보기
    while (true) {
        // 데이터 수집 수행
        monitor.collect();

        // 수집된 데이터 가져오기
        CollectorData data = monitor.getData();

        // 화면 출력 (소수점 2자리까지 깔끔하게)
        std::cout << std::fixed << std::setprecision(2)
            << "  " << std::setw(6) << data.cpuUsage << "%  |  "
            << std::setw(7) << data.memUsage << " GB |  "
            << std::setw(8) << data.netSpeed << " MB/s" << std::endl;

        // 1초 대기 (1000ms)
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    return 0;
}