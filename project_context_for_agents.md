# Rastreador Project Context for Agents

## Project Overview (프로젝트 개요)
Rastreador는 Windows 환경에서 시스템 자원(CPU, Memory) 상태와 개별 프로세스의 네트워크 사용량을 실시간으로 추적하고, 이를 GDI+ 기반의 투명 오버레이로 화면에 표시해주는 경량 네이티브 모니터링 도구입니다.

## Technical Stack (기술 스택)
- **Language**: C++20
- **Build System**: CMake
- **OS Platform**: Windows (10/11)
- **APIs & Libraries**: 
  - **Windows API**: 시스템 제어 및 정보 수집 (Advapi32, PSAPI, Iphlpapi)
  - **ETW (Event Tracing for Windows)**: 커널 수준의 이벤트 추적 (`tdh.lib`, `evntrace.h`)
  - **PDH (Performance Data Helper)**: 시스템 전반적인 성능 데이터 수집
  - **GDI+ (Graphic Device Interface Plus)**: 오버레이 UI 렌더링

## System Architecture (시스템 아키텍처)
시스템은 모듈화된 파이프라인 구조를 가집니다:
- **`Core (Monitor)`**: 프로그램의 중앙 컨트롤러로 수집 및 디스플레이 스레드의 라이프사이클을 관리합니다.
- **`Collector`**: 
  - `SystemMonitor`: 전체 시스템의 CPU, Memory, 네트워크 사용량을 수집합니다.
  - `ProcessMonitor`: 실행 중인 개별 프로세스들의 리소스 정보를 수집합니다.
  - `ETWMonitor`: ETW 세션을 통해 커널 단위의 네트워크 트래픽 이벤트를 캡처하고 PID별 수신량을 집계합니다.
- **`Analyzer (BottleneckAnalyzer)`**: 수집된 데이터를 바탕으로 임계치를 검사하여 시스템의 병목 현상(CPU/Memory/Network 과부하)을 판별합니다.
- **`Display (OverlayDisplay)`**: 분석 결과와 데이터를 바탕화면 위에 비침입형(투명) 오버레이로 실시간 렌더링합니다.
- **`Tray (TrayIcon)`**: 시스템 트레이 아이콘을 통해 백그라운드 동작을 관리합니다.

## Key Features (핵심 기능)
- **ETW 기반 실시간 네트워크 감지**: 개별 프로세스별 실시간 네트워크 수신 속도(MB/s)를 매우 정확하게 측정합니다.
- **프로세스 자원 분석**: CPU 사용율 및 메모리 점유율이 높은 상위 프로세스를 실시간으로 자동 추적합니다.
- **투명 오버레이 디스플레이**: 렌더링된 UI가 마우스 클릭을 통과(Pass-through)하게 만들어, 게임이나 작업에 방해를 주지 않습니다.
- **상태 분석 및 알림**: CPU, 메모리, 네트워크 임계치 기반으로 병목 프로세스를 찾아내어 실시간 경고를 제공합니다.
- **경량 네이티브 앱 동작**: C++로 작성되어 낮은 시스템 부하로 백그라운드 실행에 최적화되어 있습니다.

## Technical Challenges & Troubleshooting (기술적 도전 및 해결)
- **도전**: 프로세스별 정확한 네트워크 사용량을 실시간으로 측정하는 것은 기본 Windows API만으로는 한계가 있음.
- **해결**: Windows 커널의 **ETW(Event Tracing for Windows)** 세션을 직접 열어 네트워크 트래픽 이벤트를 수신하는 `ETWMonitor`를 구현. C++ 정적 콜백의 한계를 극복하기 위해 `static` 맵과 Mutex를 활용하여 Thread-Safe하게 데이터를 수집하도록 처리.
- **도전**: 오버레이 UI가 지속적으로 업데이트될 때 발생하는 화면 깜빡임과 리소스 낭비.
- **해결**: GDI+ 더블 버퍼링 기법과 윈도우 속성(`WS_EX_LAYERED`, `WS_EX_TRANSPARENT`)을 조합하여 최적의 렌더링 루프를 구축. 

## Core Logic & Optimization (핵심 로직 및 최적화)
- **멀티스레딩 파이프라인**: `collectLoop` (데이터 수집 스레드)와 `displayLoop` (화면 렌더링 스레드)를 분리. 수집 로직의 병목이 화면 주사율에 영향을 미치지 않도록 비동기적으로 동작하도록 설계되었습니다 (`std::mutex` 기반 동기화).
- **ETW 콜백 최적화 (더블 버퍼링)**: ETW 이벤트는 커널에서 초당 수천 번 이상 발생할 수 있습니다. 잦은 Lock을 피하기 위해 `s_pidBytes[2]` 와 같은 배열 기반의 더블 버퍼링 구조를 사용하여 콜백 스레드와 데이터 읽기 스레드 간의 Lock 경합을 최소화했습니다.
- **UAC 권한 제어**: ETW 사용을 위해서는 관리자 권한이 필수적이므로 `CMakeLists.txt` 에 `/MANIFESTUAC` 플래그를 설정하여 사용자가 관리자 권한 없이 실행하는 것을 원천 차단했습니다.

## Retrospective (회고 및 성과)
- 단순히 상위 레벨 API를 사용하는 것을 넘어 Windows 커널 수준의 ETW(Event Tracing for Windows)를 직접 핸들링함으로써 운영체제 내부의 패킷 처리 및 이벤트 트레이싱 원리를 깊이 이해하게 되었습니다.
- 무거운 서드파티 라이브러리(Qt, CEF 등) 없이 순수 C++와 네이티브 Windows API만을 조합하여 실용적이고 성능이 우수한 시스템 유틸리티를 완성하며 시스템 프로그래밍 및 최적화 역량을 크게 향상시켰습니다.

## Links (관련 링크)
- **GitHub Repository**: [JIK-K/Rastreador](https://github.com/JIK-K/Rastreador)
- **Releases**: [Download Rastreador](https://github.com/JIK-K/Rastreador/releases)
