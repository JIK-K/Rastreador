#include "collector/ETWMonitor.hpp"
#include <iostream>
#include <evntprov.h>

std::map<DWORD, ULONG64> ETWMonitor::s_pidBytes[2];
std::mutex ETWMonitor::s_mutex;
int ETWMonitor::s_activeIdx = 0;

// ETW가 추적 할 커널 네트워크 이벤트
// MS - Windows - Kernel - Network
static const GUID SYSTEM_TRACE_GUID = {
    0x9e814aad, 0x3204, 0x11d2,
    { 0x9a, 0x82, 0x00, 0x60, 0x08, 0xa8, 0x69, 0x39 }
};

ETWMonitor::ETWMonitor() {}
ETWMonitor::~ETWMonitor() {
    stop();
}

bool ETWMonitor::start() {
    // 이미 실행 중이라면 중복 시작 방지
    if (m_hTrace != INVALID_PROCESSTRACE_HANDLE || m_propBuf != nullptr) {
        return true;
    }

    const ULONG bufSize = sizeof(EVENT_TRACE_PROPERTIES) +
        sizeof(KERNEL_LOGGER_NAME) + 64;

    m_propBuf = new BYTE[bufSize]();
    EVENT_TRACE_PROPERTIES* prop =
        reinterpret_cast<EVENT_TRACE_PROPERTIES*>(m_propBuf);

    prop->Wnode.BufferSize = bufSize;
    prop->Wnode.Flags = WNODE_FLAG_TRACED_GUID;
    prop->Wnode.Guid = SYSTEM_TRACE_GUID;
    prop->Wnode.ClientContext = 1;
    prop->LogFileMode = EVENT_TRACE_REAL_TIME_MODE;
    prop->EnableFlags = EVENT_TRACE_FLAG_NETWORK_TCPIP;
    prop->LoggerNameOffset = sizeof(EVENT_TRACE_PROPERTIES);
    prop->EnableFlags = EVENT_TRACE_FLAG_NETWORK_TCPIP;

    ControlTraceW(0, KERNEL_LOGGER_NAME, prop, EVENT_TRACE_CONTROL_STOP);

    ULONG ret = StartTraceW(&m_hSession, KERNEL_LOGGER_NAME, prop);

    if (ret != ERROR_SUCCESS && ret != ERROR_ALREADY_EXISTS) {
        std::cerr << "[ETW] StartTrace FAIL: " << ret << std::endl;
        return false;
    }

    // StartTraceW가 EnableFlags를 덮어쓰므로 다시 설정 후 업데이트
    prop->EnableFlags = EVENT_TRACE_FLAG_NETWORK_TCPIP;
    ULONG updateRet = ControlTraceW(m_hSession, nullptr, prop, EVENT_TRACE_CONTROL_UPDATE);

    // delete 하지 말고 멤버로 유지
    if (ret != ERROR_SUCCESS && ret != ERROR_ALREADY_EXISTS) {
        std::cerr << "[ETW] StartTrace FAIL: " << ret << std::endl;
        return false;
    }

    m_pLogFile = new EVENT_TRACE_LOGFILE();
    memset(m_pLogFile, 0, sizeof(EVENT_TRACE_LOGFILE));
    m_pLogFile->LoggerName = const_cast<LPWSTR>(KERNEL_LOGGER_NAME);
    m_pLogFile->ProcessTraceMode = PROCESS_TRACE_MODE_REAL_TIME |
        PROCESS_TRACE_MODE_EVENT_RECORD;
    m_pLogFile->EventRecordCallback = eventCallback;

    m_hTrace = OpenTrace(m_pLogFile);
    if (m_hTrace == INVALID_PROCESSTRACE_HANDLE) {
        std::cerr << "[ETW] OpenTrace FAIL" << std::endl;
        return false;
    }


    m_hThread = CreateThread(nullptr, 0,
        [](LPVOID param) -> DWORD {
            ETWMonitor* self = reinterpret_cast<ETWMonitor*>(param);
            ULONG result = ProcessTrace(&self->m_hTrace, 1, nullptr, nullptr);
            std::cout << "[ETW] ProcessTrace returned: " << result << std::endl;
            return 0;
        },
        this, 0, nullptr
    );

    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    std::cout << "[ETW] Session Start Complete" << std::endl;
    return true;
}

void ETWMonitor::stop() {
    // 이미 정지되었거나 자원이 없는 경우 중복 해제 방지
    if (m_hTrace == INVALID_PROCESSTRACE_HANDLE && m_propBuf == nullptr) {
        return;
    }

    if (m_hTrace != INVALID_PROCESSTRACE_HANDLE) {
        CloseTrace(m_hTrace);
        m_hTrace = INVALID_PROCESSTRACE_HANDLE;
    }
    if (m_hThread) {
        WaitForSingleObject(m_hThread, 2000);
        CloseHandle(m_hThread);
        m_hThread = nullptr;
    }

    const ULONG bufSize = sizeof(EVENT_TRACE_PROPERTIES) +
        sizeof(KERNEL_LOGGER_NAME) + 64;
    BYTE* buf = new BYTE[bufSize]();
    EVENT_TRACE_PROPERTIES* prop =
        reinterpret_cast<EVENT_TRACE_PROPERTIES*>(buf);
    prop->Wnode.BufferSize = bufSize;
    prop->LoggerNameOffset = sizeof(EVENT_TRACE_PROPERTIES);

    ControlTraceW(0, KERNEL_LOGGER_NAME, prop, EVENT_TRACE_CONTROL_STOP);
    
    if (buf) delete[] buf;

    if (m_propBuf) {
        delete[] m_propBuf;
        m_propBuf = nullptr;
    }
    if (m_pLogFile) {
        delete m_pLogFile;
        m_pLogFile = nullptr;
    }

    std::cout << "[ETW] Session Stop Complete" << std::endl;
}

VOID WINAPI ETWMonitor::eventCallback(PEVENT_RECORD pEvent) {
    if (!pEvent) return;

    UCHAR opcode = pEvent->EventHeader.EventDescriptor.Opcode;
    if (opcode != 10 && opcode != 11 && opcode != 26 && opcode != 27) return;

    if (pEvent->UserDataLength < sizeof(ULONG) * 2) return;

    ULONG* data = reinterpret_cast<ULONG*>(pEvent->UserData);
    ULONG pid = data[0];  // 페이로드에서 PID
    ULONG recvSize = data[1];  // 페이로드에서 크기

    if (pid == 0 || recvSize == 0) return;

    std::lock_guard<std::mutex> lock(s_mutex);
    s_pidBytes[s_activeIdx][pid] += recvSize;
}

std::map<DWORD, ULONG64> ETWMonitor::getAndResetBytes() {
    std::lock_guard<std::mutex> lock(s_mutex);
    int readIdx = s_activeIdx;
    s_activeIdx = 1 - s_activeIdx;

    auto copy = s_pidBytes[readIdx];
    s_pidBytes[readIdx].clear();
    return copy;
}