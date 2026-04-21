#pragma once
#include "collector/IDataCollector.hpp"
#include <windows.h>
#include <Pdh.h>

class SystemMonitor : public IDataCollector{
public:
	SystemMonitor(); 
	virtual ~SystemMonitor();

	SystemMonitor(const SystemMonitor&) = delete;
	SystemMonitor& operator=(const SystemMonitor&) = delete;

	SystemMonitor(SystemMonitor&&) = delete;
	SystemMonitor& operator=(SystemMonitor&&) = delete;

	void collect() override;
	CollectorData getData() const override;

private:
	// 클래스 내부에서만 쓸 보조 멤버 함수 (PDH 카운터 설정 등)
	void queryPDH();

private:
	// Windows PDH API를 사용하기 위한 포인터, 핸들
	PDH_HQUERY m_hQuery = nullptr; // 전체 핸들
	PDH_HCOUNTER m_CpuCounter = nullptr; // CPU용 측정기
	PDH_HCOUNTER m_NetCounter = nullptr; // Network용 측정기

	CollectorData m_data;
};