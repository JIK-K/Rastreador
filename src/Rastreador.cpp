#include "core/Monitor.hpp"
#include "display/ConsoleDisplay.hpp"
#include "display/OverlayDisplay.hpp"
#include <iostream>

int main() {
    OverlayDisplay overlayDisplay;
    Monitor monitor(&overlayDisplay, 100.0f);

    ShowWindow(GetConsoleWindow(), SW_HIDE);

    // Monitor 별도 스레드로 실행
    std::thread monitorThread([&monitor]() {
        monitor.start();
        });

    // 트레이 아이콘 (메인 스레드에서 메시지 루프)
    TrayIcon tray([&monitor, &monitorThread]() {
        monitor.stop();
        if (monitorThread.joinable()) monitorThread.join();
        PostQuitMessage(0);
        });
    tray.onCreate();

    return 0;
}