#include "collector/ProcessMonitor.hpp"
#include <Psapi.h>
#include <iostream>

ProcessMonitor::ProcessMonitor(){}
ProcessMonitor::~ProcessMonitor() {}

std::string ProcessMonitor::getProcessName(DWORD pid) {
	// 윈도우 대기 프로세스
	if (pid == 0) return "System Idle"; 
	// 윈도우 커널
	if (pid == 4) return "System";

	char name[MAX_PATH] = "<unknown>";

	// PROCESS_QUERY_INFORMATION : 프로세스의 이름 또는 종료코드 같은 기본 정보 읽기 권한
	// PROCESS_VM_READ : 프로세스 메모리 영역 읽기 권한
	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);

	if (hProcess) {
		if (GetModuleBaseNameA(hProcess, nullptr, name, sizeof(name))) {
			//success
			std::cout << "[파일이름가져오기] : " << name << '\n';
		}
		CloseHandle(hProcess);
	}
	return std::string(name);
}

void ProcessMonitor::collect() {
	m_netPerProc.clear();

	ULONG size = 0;
	// AF_INET : IPv4
	GetExtendedTcpTable(nullptr, &size, FALSE, AF_INET, TCP_TABLE_OWNER_PID_ALL, 0);

	std::vector<BYTE> buffer(size);
	PMIB_TCPTABLE_OWNER_PID pTcpTable = reinterpret_cast<PMIB_TCPTABLE_OWNER_PID>(buffer.data());

	if (GetExtendedTcpTable(pTcpTable, &size, FALSE, AF_INET, TCP_TABLE_OWNER_PID_ALL, 0) == NO_ERROR) {
		for (DWORD i = 0; i < pTcpTable->dwNumEntries; i++) {
			DWORD pid = pTcpTable->table[i].dwOwningPid;

			std::string procName = getProcessName(pid);

			// @todo 해당 프로세스가 통신 중이다 라는 표시로 1.0 이후 실제 전송량 계산 로직으로 변경
			m_netPerProc[procName] = 1.0f;
		}
	}
}

CollectorData ProcessMonitor::getData() const {
	CollectorData data = { 0 };
	// ProcessMonitor의 주 목적은 m_netPerProc 맵이므로 
	// 여기서는 기본값만 넘기거나, 필요 시 합계 netSpeed를 계산해서 넘깁니다.
	return data;
}