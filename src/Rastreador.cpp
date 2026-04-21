#include "core/Monitor.hpp"
#include "display/ConsoleDisplay.hpp"
#include "display/OverlayDisplay.hpp"
#include <iostream>

int main() {
    std::cout << "[Rastreador]\n";
    std::cout << "1. 콘솔 모드\n";
    std::cout << "2. 오버레이 모드\n";
    std::cout << "선택 (1/2): ";

    int choice = 1;
    std::cin >> choice;

    if (choice == 2) {
        OverlayDisplay display;
        Monitor monitor(&display, 100.0f);
        monitor.start();
    }
    else {
        ConsoleDisplay display;
        Monitor monitor(&display, 100.0f);
        monitor.start();
    }

    return 0;
}