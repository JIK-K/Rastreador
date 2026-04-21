#pragma once
#include <Windows.h>
#include <evntrace.h>
#include <evntcons.h>
#include <tdh.h>
#include <map>
#include <string>
#include <mutex>

class ETWMonitor {
public:
	ETWMonitor();
	~ETWMonitor();

	ETWMonitor(const ETWMonitor&) = delete;
	ETWMonitor& operator=(const ETWMonitor&) = delete;

	ETWMonitor(ETWMonitor&&) = delete;
	ETWMonitor& operator=(ETWMonitor&&) = delete;

	bool start();
	void stop();
	// PID별 누적 수신 바이트 반환 후 초기화
	std::map<DWORD, ULONG64> getAndResetBytes();
    
private:
    // 커널 이벤트 콜백 (static — Windows API 요구사항)
    static VOID WINAPI eventCallback(PEVENT_RECORD pEvent);

    // 세션 핸들
    TRACEHANDLE m_hSession = INVALID_PROCESSTRACE_HANDLE;
    TRACEHANDLE m_hTrace = INVALID_PROCESSTRACE_HANDLE;

    // ETW 처리 스레드
    HANDLE m_hThread = nullptr;
    bool   m_running = false;

    // PID별 누적 수신 바이트 (콜백에서 누적)
    static std::map<DWORD, ULONG64> s_pidBytes[2];
    static std::mutex s_mutex;
    static int s_activeIdx;

    // 세션 이름
    static const wchar_t* SESSION_NAME;

    BYTE* m_propBuf = nullptr;
    EVENT_TRACE_LOGFILE* m_pLogFile = nullptr;
};