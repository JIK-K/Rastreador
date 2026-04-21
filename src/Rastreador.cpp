#include <iostream>
#include <thread>
#include <chrono>
#include <iomanip> 
#include "collector/SystemMonitor.hpp"
#include "collector/ProcessMonitor.hpp"
#include "analyzer/BottleneckAnalyzer.hpp"
#include "display/ConsoleDisplay.hpp"
#include "core/Monitor.hpp"

int main() {
    ConsoleDisplay display;
    Monitor monitor(&display, 100.0f);

    monitor.start();

    return 0;
}