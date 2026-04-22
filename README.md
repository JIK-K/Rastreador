

# <img width="48" height="48" alt="icon" src="https://github.com/user-attachments/assets/6ae7023d-da2c-4854-a95b-f4fa0ba01668" /> Rastreador
> **실시간 프로세스 자원 모니터링 및 오버레이 도구**

[![Platform: Windows](https://img.shields.io/badge/Platform-Windows-blue.svg)](https://www.microsoft.com/windows)
[![C++](https://img.shields.io/badge/Language-C%2B%2B-00599C.svg?logo=c%2B%2B)](https://isocpp.org/)

Rastreador는 Windows ETW(Event Tracing for Windows)를 활용하여 개별 프로세스의 네트워크 사용량을 실시간으로 감지하고, 시스템 자원(CPU, Memory) 상태를 화면에 오버레이로 표시해주는 프로그램입니다.


기능
- **ETW 기반 실시간 네트워크 감지**: 개별 프로세스별 실시간 네트워크 수신 속도(MB/s) 측정
- **프로세스 자원 분석**: CPU 사용율 및 메모리 점유율이 높은 상위 프로세스 자동 추적
- **투명 오버레이 디스플레이**: 게임이나 작업 중에도 방해받지 않는 투명 UI (GDI+ 기반)
- **상태 분석 및 알림**: 수집된 데이터를 바탕으로 시스템 상태 분석 결과 출력
- **경량 네이티브 앱**: C++로 작성되어 낮은 시스템 부하로 백그라운드 실행 가능

빠른 시작
1. [Releases](https://github.com/JIK-K/Rastreador/releases) 페이지에서 최신 빌드를 다운로드하거나 직접 빌드하세요.
2. **관리자 권한**으로 `Rastreador.exe`를 실행하세요. (ETW 세션 시작을 위해 관리자 권한이 필수입니다.)
3. 화면 우측 상단에 실시간 모니터링 오버레이가 나타납니다.
```

⚠️ 주의사항
- ETW 커널 세션을 사용하므로 반드시 **관리자 권한**으로 실행해야 네트워크 모니터링 기능이 동작합니다.
- Windows 10/11 환경에 최적화되어 있습니다.
